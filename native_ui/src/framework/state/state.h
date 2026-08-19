#pragma once

#include <functional>
#include <mutex>
#include <vector>

#include "src/framework/state/property.h"

namespace native::ui {

class State {
public:
  State() = default;
  virtual ~State();

  void AddWatcher(void* key, std::function<void()> callback, PropertyBase* prop);
  void RemoveWatcher(void* key);

  void EnqueueDirty(PropertyBase* prop);

  bool Flush();

protected:
  void NotifyWatchers(PropertyBase* key);

private:
  struct WatcherEntry {
    void* key;
    PropertyBase* prop;
    std::function<void()> callback;
  };

  std::mutex mutex_;
  std::vector<WatcherEntry> watchers_;
  std::vector<PropertyBase*> dirty_queue_;
};

}  // namespace native::ui

#include "src/framework/state/property_inl.h"
