/*
 * Copyright 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_NDEBUG 1
#undef LOG_TAG
#define LOG_TAG "ClientCache"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <cinttypes>

#include <android-base/stringprintf.h>
#include <gui/TraceUtils.h>
#include <renderengine/impl/ExternalTexture.h>

#include "ClientCache.h"

namespace android {

ANDROID_SINGLETON_STATIC_INSTANCE(ClientCache);

ClientCache::ClientCache() : mDeathRecipient(sp<CacheDeathRecipient>::make()) {}

bool ClientCache::getBuffer(const client_cache_t& cacheId,
                            ClientCacheBuffer** outClientCacheBuffer) {
    auto& [processToken, id] = cacheId;
    if (processToken == nullptr) {
        ALOGE_AND_TRACE("ClientCache::getBuffer - invalid (nullptr) process token");
        return false;
    }
    auto it = mBuffers.find(processToken);
    if (it == mBuffers.end()) {
        ALOGE_AND_TRACE("ClientCache::getBuffer - invalid process token");
        return false;
    }

    auto& processBuffers = it->second.second;

    auto bufItr = processBuffers.find(id);
    if (bufItr == processBuffers.end()) {
        ALOGE_AND_TRACE("ClientCache::getBuffer - invalid buffer id");
        return false;
    }

    ClientCacheBuffer& buf = bufItr->second;
    *outClientCacheBuffer = &buf;
    return true;
}

base::expected<std::shared_ptr<renderengine::ExternalTexture>, ClientCache::AddError>
ClientCache::add(const client_cache_t& cacheId, const sp<GraphicBuffer>& buffer) {
    auto& [processToken, id] = cacheId;
    if (processToken == nullptr) {
        ALOGE_AND_TRACE("ClientCache::add - invalid (nullptr) process token");
        return base::unexpected(AddError::Unspecified);
    }

    if (!buffer) {
        ALOGE_AND_TRACE("ClientCache::add - invalid (nullptr) buffer");
        return base::unexpected(AddError::Unspecified);
    }

    std::lock_guard lock(mMutex);
    sp<IBinder> token;

    // If this is a new process token, set a death recipient. If the client process dies, we will
    // get a callback through binderDied.
    auto it = mBuffers.find(processToken);
    if (it == mBuffers.end()) {
        token = processToken.promote();
        if (!token) {
            ALOGE_AND_TRACE("ClientCache::add - invalid token");
            return base::unexpected(AddError::Unspecified);
        }

        // Only call linkToDeath if not a local binder
        if (token->localBinder() == nullptr) {
            status_t err = token->linkToDeath(mDeathRecipient);
            if (err != NO_ERROR) {
                ALOGE_AND_TRACE("ClientCache::add - could not link to death");
                return base::unexpected(AddError::Unspecified);
            }
        }
        auto [itr, success] =
                mBuffers.emplace(processToken,
                                 std::make_pair(token,
                                                std::unordered_map<uint64_t, ClientCacheBuffer>()));
        LOG_ALWAYS_FATAL_IF(!success, "failed to insert new process into client cache");
        it = itr;
    }

    auto& processBuffers = it->second.second;

    if (processBuffers.size() > BUFFER_CACHE_MAX_SIZE) {
        ALOGE_AND_TRACE("ClientCache::add - cache is full");
        return base::unexpected(AddError::CacheFull);
    }

    LOG_ALWAYS_FATAL_IF(mRenderEngine == nullptr,
                        "Attempted to build the ClientCache before a RenderEngine instance was "
                        "ready!");

    return (processBuffers[id].buffer = std::make_shared<
                    renderengine::impl::ExternalTexture>(buffer, *mRenderEngine,
                                                         renderengine::impl::ExternalTexture::
                                                                 Usage::READABLE));
}

sp<GraphicBuffer> ClientCache::erase(const client_cache_t& cacheId) {
    sp<GraphicBuffer> buffer;
    auto& [processToken, id] = cacheId;
    std::vector<sp<ErasedRecipient>> pendingErase;
    {
        std::lock_guard lock(mMutex);
        ClientCacheBuffer* buf = nullptr;
        if (!getBuffer(cacheId, &buf)) {
            ALOGE("failed to erase buffer, could not retrieve buffer");
            return nullptr;
        }

        buffer = buf->buffer->getBuffer();

        for (auto& recipient : buf->recipients) {
            sp<ErasedRecipient> erasedRecipient = recipient.promote();
            if (erasedRecipient) {
                pendingErase.push_back(erasedRecipient);
            }
        }

        mBuffers[processToken].second.erase(id);
    }

    for (auto& recipient : pendingErase) {
        recipient->bufferErased(cacheId);
    }
    return buffer;
}

std::shared_ptr<renderengine::ExternalTexture> ClientCache::get(const client_cache_t& cacheId) {
    std::lock_guard lock(mMutex);

    ClientCacheBuffer* buf = nullptr;
    if (!getBuffer(cacheId, &buf)) {
        ALOGE("failed to get buffer, could not retrieve buffer");
        return nullptr;
    }

    return buf->buffer;
}

bool ClientCache::registerErasedRecipient(const client_cache_t& cacheId,
                                          const wp<ErasedRecipient>& recipient) {
    std::lock_guard lock(mMutex);

    ClientCacheBuffer* buf = nullptr;
    if (!getBuffer(cacheId, &buf)) {
        ALOGV("failed to register erased recipient, could not retrieve buffer");
        return false;
    }
    buf->recipients.insert(recipient);
    return true;
}

void ClientCache::unregisterErasedRecipient(const client_cache_t& cacheId,
                                            const wp<ErasedRecipient>& recipient) {
    std::lock_guard lock(mMutex);

    ClientCacheBuffer* buf = nullptr;
    if (!getBuffer(cacheId, &buf)) {
        ALOGE("failed to unregister erased recipient");
        return;
    }

    buf->recipients.erase(recipient);
}

void ClientCache::removeProcess(const wp<IBinder>& processToken) {
    std::vector<std::pair<sp<ErasedRecipient>, client_cache_t>> pendingErase;
    {
        if (processToken == nullptr) {
            ALOGE("failed to remove process, invalid (nullptr) process token");
            return;
        }
        std::lock_guard lock(mMutex);
        auto itr = mBuffers.find(processToken);
        if (itr == mBuffers.end()) {
            ALOGE("failed to remove process, could not find process");
            return;
        }

        for (auto& [id, clientCacheBuffer] : itr->second.second) {
            client_cache_t cacheId = {processToken, id};
            for (auto& recipient : clientCacheBuffer.recipients) {
                sp<ErasedRecipient> erasedRecipient = recipient.promote();
                if (erasedRecipient) {
                    pendingErase.emplace_back(erasedRecipient, cacheId);
                }
            }
        }
        mBuffers.erase(itr);
    }

    for (auto& [recipient, cacheId] : pendingErase) {
        recipient->bufferErased(cacheId);
    }
}

void ClientCache::CacheDeathRecipient::binderDied(const wp<IBinder>& who) {
    ClientCache::getInstance().removeProcess(who);
}

void ClientCache::dump(std::string& result) {
    std::lock_guard lock(mMutex);
    for (const auto& [_, cache] : mBuffers) {
        base::StringAppendF(&result, " Cache owner: %p\n", cache.first.get());

        for (const auto& [id, entry] : cache.second) {
            const auto& buffer = entry.buffer->getBuffer();
            base::StringAppendF(&result, "\tID: %" PRIu64 ", size: %ux%u\n", id, buffer->getWidth(),
                                buffer->getHeight());
        }
    }
}

} // namespace android
