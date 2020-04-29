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

#ifndef COBALT_DOM_LOTTIE_PLAYER_H_
#define COBALT_DOM_LOTTIE_PLAYER_H_

#include <string>

#include "cobalt/dom/html_element.h"
#include "cobalt/loader/image/image_cache.h"
#include "cobalt/script/environment_settings.h"
#include "cobalt/script/global_environment.h"

namespace cobalt {
namespace dom {

class Document;

// Custom element that represents a Lottie web player, which embeds and plays
// Lottie animations.
// Although LottiePlayer does not inherit from HTMLImageElement, much of its
// functionality is based off that of HTMLImageElement - in particular, loading
// the animation pointed to by the "src" attribute.
//   https://github.com/LottieFiles/lottie-player
class LottiePlayer : public HTMLElement {
 public:
  static const char kTagName[];

  explicit LottiePlayer(Document* document)
      : HTMLElement(document, base::Token(kTagName)) {}

  // Web API: LottiePlayer
  //
  std::string src() const { return GetAttribute("src").value_or(""); }
  void set_src(const std::string& src) { SetAttribute("src", src); }

  // Custom, not in any spec
  //
  // From HTMLElement
  scoped_refptr<LottiePlayer> AsLottiePlayer() override { return this; }

  const scoped_refptr<loader::image::CachedImage>& cached_image() {
    return cached_image_;
  }

  DEFINE_WRAPPABLE_TYPE(LottiePlayer);

 private:
  ~LottiePlayer() override {}

  // From Node.
  void PurgeCachedBackgroundImagesOfNodeAndDescendants() override;

  // From Element.
  void OnSetAttribute(const std::string& name,
                      const std::string& value) override;
  void OnRemoveAttribute(const std::string& name) override;

  void UpdateAnimationData();

  void OnLoadingSuccess();
  void OnLoadingError();

  void PreventGarbageCollectionUntilEventIsDispatched(base::Token event_name);
  void AllowGarbageCollectionAfterEventIsDispatched(
      base::Token event_name,
      std::unique_ptr<script::GlobalEnvironment::ScopedPreventGarbageCollection>
          scoped_prevent_gc);
  void DestroyScopedPreventGC(
      std::unique_ptr<script::GlobalEnvironment::ScopedPreventGarbageCollection>
          scoped_prevent_gc);

  scoped_refptr<loader::image::CachedImage> cached_image_;
  std::unique_ptr<loader::image::CachedImage::OnLoadedCallbackHandler>
      cached_image_loaded_callback_handler_;

  std::unique_ptr<script::GlobalEnvironment::ScopedPreventGarbageCollection>
      prevent_gc_until_load_complete_;
};

}  // namespace dom
}  // namespace cobalt

#endif  // COBALT_DOM_LOTTIE_PLAYER_H_