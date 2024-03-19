/*
 * Copyright 2013 The Android Open Source Project
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

// TODO(b/129481165): remove the #pragma below and fix conversion issues
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"

#include <stdio.h>
#include <stdlib.h>
#include <log/log.h>

#include "EventLog.h"

namespace android {

ANDROID_SINGLETON_STATIC_INSTANCE(EventLog)


EventLog::EventLog() {
}

void EventLog::doLogFrameDurations(const std::string_view& name, const int32_t* durations,
                                   size_t numDurations) {
}

void EventLog::logFrameDurations(const std::string_view& name, const int32_t* durations,
                                 size_t numDurations) {
}

// ---------------------------------------------------------------------------

EventLog::TagBuffer::TagBuffer(int32_t tag)
    : mPos(0), mTag(tag), mOverflow(false) {
}

void EventLog::TagBuffer::log() {
}

void EventLog::TagBuffer::startList(int8_t count) {
}

void EventLog::TagBuffer::endList() {
}

void EventLog::TagBuffer::writeInt32(int32_t value) {
}

void EventLog::TagBuffer::writeInt64(int64_t value) {
}

void EventLog::TagBuffer::writeString(const std::string_view& value) {
}

} // namespace android

// TODO(b/129481165): remove the #pragma below and fix conversion issues
#pragma clang diagnostic pop // ignored "-Wconversion"
