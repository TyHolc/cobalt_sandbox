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

#include "cobalt/dom/lottie_player.h"

#include <memory>
#include <string>

#include "base/message_loop/message_loop.h"
#include "base/trace_event/trace_event.h"
#include "cobalt/base/polymorphic_downcast.h"
#include "cobalt/dom/csp_delegate.h"
#include "cobalt/dom/document.h"
#include "cobalt/dom/dom_settings.h"
#include "cobalt/dom/html_element_context.h"
#include "cobalt/dom/window.h"
#include "cobalt/script/global_environment.h"
#include "url/gurl.h"

namespace cobalt {
namespace dom {

const char LottiePlayer::kTagName[] = "lottie-player";

void LottiePlayer::PurgeCachedBackgroundImagesOfNodeAndDescendants() {
  if (!cached_image_loaded_callback_handler_) {
    return;
  }

  // While we are still loading, treat this as an error.
  OnLoadingError();
}

void LottiePlayer::OnSetAttribute(const std::string& name,
                                  const std::string& value) {
  if (name == "src") {
    UpdateAnimationData();
  } else {
    HTMLElement::OnSetAttribute(name, value);
  }
}

void LottiePlayer::OnRemoveAttribute(const std::string& name) {
  if (name == "src") {
    UpdateAnimationData();
  } else {
    HTMLElement::OnRemoveAttribute(name);
  }
}

void LottiePlayer::UpdateAnimationData() {
  DCHECK(base::MessageLoop::current());
  DCHECK(node_document());
  TRACE_EVENT0("cobalt::dom", "LottiePlayer::UpdateAnimationData()");

  if (cached_image_loaded_callback_handler_) {
    cached_image_loaded_callback_handler_.reset();
    prevent_gc_until_load_complete_.reset();
    node_document()->DecreaseLoadingCounter();
  }

  const std::string src = GetAttribute("src").value_or("");

  if (!src.empty()) {
    const GURL& base_url = node_document()->url_as_gurl();
    const GURL selected_source = base_url.Resolve(src);
    if (!selected_source.is_valid()) {
      LOG(WARNING) << src << " cannot be resolved based on " << base_url << ".";
      return;
    }

    auto image_cache = node_document()->html_element_context()->image_cache();
    cached_image_ = image_cache->GetOrCreateCachedResource(selected_source,
                                                           loader::Origin());

    if (cached_image_->TryGetResource()) {
      PreventGarbageCollectionUntilEventIsDispatched(base::Tokens::load());
      return;
    }
  } else {
    PreventGarbageCollectionUntilEventIsDispatched(base::Tokens::error());
    return;
  }

  DCHECK(!prevent_gc_until_load_complete_);
  prevent_gc_until_load_complete_.reset(
      new script::GlobalEnvironment::ScopedPreventGarbageCollection(
          html_element_context()->script_runner()->GetGlobalEnvironment(),
          this));
  node_document()->IncreaseLoadingCounter();
  cached_image_loaded_callback_handler_.reset(
      new loader::image::CachedImage::OnLoadedCallbackHandler(
          cached_image_,
          base::Bind(&LottiePlayer::OnLoadingSuccess, base::Unretained(this)),
          base::Bind(&LottiePlayer::OnLoadingError, base::Unretained(this))));
}

void LottiePlayer::OnLoadingSuccess() {
  TRACE_EVENT0("cobalt::dom", "LottiePlayer::OnLoadingSuccess()");
  AllowGarbageCollectionAfterEventIsDispatched(
      base::Tokens::load(), std::move(prevent_gc_until_load_complete_));
  if (node_document()) {
    node_document()->DecreaseLoadingCounterAndMaybeDispatchLoadEvent();
  }
  cached_image_loaded_callback_handler_.reset();

  // Set up the Lottie objects in the box and render trees once the file has
  // successfully loaded.
  node_document()->RecordMutation();
  InvalidateLayoutBoxRenderTreeNodes();
}

void LottiePlayer::OnLoadingError() {
  TRACE_EVENT0("cobalt::dom", "LottiePlayer::OnLoadingError()");
  AllowGarbageCollectionAfterEventIsDispatched(
      base::Tokens::error(), std::move(prevent_gc_until_load_complete_));
  if (node_document()) {
    node_document()->DecreaseLoadingCounterAndMaybeDispatchLoadEvent();
  }
  cached_image_loaded_callback_handler_.reset();
}

void LottiePlayer::PreventGarbageCollectionUntilEventIsDispatched(
    base::Token event_name) {
  std::unique_ptr<script::GlobalEnvironment::ScopedPreventGarbageCollection>
      prevent_gc_until_event_dispatch(
          new script::GlobalEnvironment::ScopedPreventGarbageCollection(
              html_element_context()->script_runner()->GetGlobalEnvironment(),
              this));
  AllowGarbageCollectionAfterEventIsDispatched(
      event_name, std::move(prevent_gc_until_event_dispatch));
}

void LottiePlayer::AllowGarbageCollectionAfterEventIsDispatched(
    base::Token event_name,
    std::unique_ptr<script::GlobalEnvironment::ScopedPreventGarbageCollection>
        scoped_prevent_gc) {
  PostToDispatchEventNameAndRunCallback(
      FROM_HERE, event_name,
      base::Bind(&LottiePlayer::DestroyScopedPreventGC,
                 base::AsWeakPtr<LottiePlayer>(this),
                 base::Passed(&scoped_prevent_gc)));
}

void LottiePlayer::DestroyScopedPreventGC(
    std::unique_ptr<script::GlobalEnvironment::ScopedPreventGarbageCollection>
        scoped_prevent_gc) {
  scoped_prevent_gc.reset();
}

}  // namespace dom
}  // namespace cobalt