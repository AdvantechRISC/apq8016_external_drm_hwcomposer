/*
 * Copyright (C) 2015 The Android Open Source Project
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

#define LOG_TAG "hwc-drm-compositor-worker"

#include "drmdisplaycompositor.h"
#include "drmcompositorworker.h"
#include "worker.h"

#include <stdlib.h>

#include <cutils/log.h>
#include <hardware/hardware.h>

namespace android {

DrmCompositorWorker::DrmCompositorWorker(DrmDisplayCompositor *compositor)
    : Worker("drm-compositor", HAL_PRIORITY_URGENT_DISPLAY),
      compositor_(compositor) {
}

DrmCompositorWorker::~DrmCompositorWorker() {
}

int DrmCompositorWorker::Init() {
  return InitWorker();
}

void DrmCompositorWorker::Routine() {
  int ret;
  if (!compositor_->HaveQueuedComposites()) {
    ret = Lock();
    if (ret) {
      ALOGE("Failed to lock worker, %d", ret);
      return;
    }

    int wait_ret = WaitForSignalOrExitLocked();

    ret = Unlock();
    if (ret) {
      ALOGE("Failed to unlock worker, %d", ret);
      return;
    }

    if (wait_ret == -EINTR) {
      return;
    } else if (wait_ret) {
      ALOGE("Failed to wait for signal, %d", wait_ret);
      return;
    }
  }

  ret = compositor_->Composite();
  if (ret)
    ALOGE("Failed to composite! %d", ret);
}
}
