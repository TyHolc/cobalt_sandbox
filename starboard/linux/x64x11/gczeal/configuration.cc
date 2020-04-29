// Copyright 2020 The Cobalt Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "starboard/linux/x64x11/gczeal/configuration.h"

#include "cobalt/extension/configuration.h"
#include "starboard/common/configuration_defaults.h"

// Omit namespace linux due to symbol name conflict.
namespace starboard {
namespace x64x11 {
namespace gczeal {

namespace {

int CobaltEglSwapInterval() {
  return 0;
}

bool CobaltGcZeal() {
  return true;
}

bool CobaltEnableJit() {
  return true;
}

const CobaltExtensionConfigurationApi kConfigurationApi = {
    kCobaltExtensionConfigurationName,
    1,
    &common::CobaltUserOnExitStrategyDefault,
    &common::CobaltRenderDirtyRegionOnlyDefault,
    &CobaltEglSwapInterval,
    &common::CobaltFallbackSplashScreenUrlDefault,
    &common::CobaltEnableQuicDefault,
    &common::CobaltSkiaCacheSizeInBytesDefault,
    &common::CobaltOffscreenTargetCacheSizeInBytesDefault,
    &common::CobaltEncodedImageCacheSizeInBytesDefault,
    &common::CobaltImageCacheSizeInBytesDefault,
    &common::CobaltLocalTypefaceCacheSizeInBytesDefault,
    &common::CobaltRemoteTypefaceCacheSizeInBytesDefault,
    &common::CobaltMeshCacheSizeInBytesDefault,
    &common::CobaltSoftwareSurfaceCacheSizeInBytesDefault,
    &common::CobaltImageCacheCapacityMultiplierWhenPlayingVideoDefault,
    &common::CobaltSkiaGlyphAtlasWidthDefault,
    &common::CobaltSkiaGlyphAtlasHeightDefault,
    &common::CobaltJsGarbageCollectionThresholdInBytesDefault,
    &common::CobaltReduceCpuMemoryByDefault,
    &common::CobaltReduceGpuMemoryByDefault,
    &CobaltGcZeal,
    &common::CobaltRasterizerTypeDefault,
    &CobaltEnableJit,
};

}  // namespace

const void* GetConfigurationApi() {
  return &kConfigurationApi;
}

}  // namespace gczeal
}  // namespace x64x11
}  // namespace starboard