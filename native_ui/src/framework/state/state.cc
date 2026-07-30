#include "state.h"

namespace native::ui {

State::~State() {
  std::lock_guard<std::mutex> lock(mutex_);
  watchers_.clear();
  dirty_queue_.clear();
}

void State::AddWatcher(void* key, std::function<void()> callback,
                        PropertyBase* prop) {
  std::lock_guard<std::mutex> lock(mutex_);
  watchers_.push_back({key, prop, std::move(callback)});
}

void State::RemoveWatcher(void* key) {
  std::lock_guard<std::mutex> lock(mutex_);
  for (auto it = watchers_.begin(); it != watchers_.end();) {
    if (it->key == key) {
      it = watchers_.erase(it);
    } else {
      ++it;
    }
  }
}

void State::EnqueueDirty(PropertyBase* prop) {
  std::lock_guard<std::mutex> lock(mutex_);
  dirty_queue_.push_back(prop);
}

bool State::Flush() {
  std::vector<PropertyBase*> batch;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    batch.swap(dirty_queue_);
  }

  if (batch.empty()) {
    return false;
  }

  std::sort(batch.begin(), batch.end());
  auto last = std::unique(batch.begin(), batch.end());
  for (auto it = batch.begin(); it != last; ++it) {
    NotifyWatchers(*it);
  }

  return true;
}

void State::NotifyWatchers(PropertyBase* key) {
  for (auto& entry : watchers_) {
    if (entry.prop->key() == key->key()) {
      if (entry.callback) {
        entry.callback();
      }
    }
  }
}

}  // namespace native::ui
