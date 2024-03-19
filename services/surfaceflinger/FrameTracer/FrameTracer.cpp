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

// TODO(b/129481165): remove the #pragma below and fix conversion issues
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"

#undef LOG_TAG
#define LOG_TAG "FrameTracer"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "FrameTracer.h"

#include <android-base/stringprintf.h>
#include <perfetto/common/builtin_clock.pbzero.h>

#include <algorithm>
#include <mutex>

PERFETTO_DEFINE_DATA_SOURCE_STATIC_MEMBERS(android::FrameTracer::FrameTracerDataSource);

namespace android {

void FrameTracer::initialize() {
}

void FrameTracer::registerDataSource() {
}

void FrameTracer::traceNewLayer(int32_t layerId, const std::string& layerName) {
}

void FrameTracer::traceTimestamp(int32_t layerId, uint64_t bufferID, uint64_t frameNumber,
                                 nsecs_t timestamp, FrameEvent::BufferEventType type,
                                 nsecs_t duration) {
}

void FrameTracer::traceFence(int32_t layerId, uint64_t bufferID, uint64_t frameNumber,
                             const std::shared_ptr<FenceTime>& fence,
                             FrameEvent::BufferEventType type, nsecs_t startTime) {
}

void FrameTracer::tracePendingFencesLocked(FrameTracerDataSource::TraceContext& ctx,
                                           int32_t layerId, uint64_t bufferID) {
}

void FrameTracer::traceLocked(FrameTracerDataSource::TraceContext& ctx, int32_t layerId,
                              uint64_t bufferID, uint64_t frameNumber, nsecs_t timestamp,
                              FrameEvent::BufferEventType type, nsecs_t duration) {
}

void FrameTracer::traceSpanLocked(FrameTracerDataSource::TraceContext& ctx, int32_t layerId,
                                  uint64_t bufferID, uint64_t frameNumber,
                                  FrameEvent::BufferEventType type, nsecs_t startTime,
                                  nsecs_t endTime) {
}

void FrameTracer::onDestroy(int32_t layerId) {
}

std::string FrameTracer::miniDump() {
    return 0;
}

} // namespace android

// TODO(b/129481165): remove the #pragma below and fix conversion issues
#pragma clang diagnostic pop // ignored "-Wconversion"
