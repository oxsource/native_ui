# Glide Contract

**Purpose**: Define Glide singleton for async image loading, DefaultGlide implementation, and ImageWidget integration.

## Glide (Abstract Base)

```cpp
namespace native::ui {

enum class LoadState { kLoading, kLoaded, kError };

struct LoadOptions {
  int target_width = 0;
  int target_height = 0;
};

using LoadCallback = std::function<void(const std::string& path,
                                        std::shared_ptr<Image> image,
                                        LoadState state)>;

class Glide {
public:
  static Glide* Default();
  static void SetDefault(Glide* glide);  // main-thread-only, takes ownership

  virtual ~Glide() = default;

  virtual uint64_t Load(const std::string& file_path,
                        LoadCallback callback,
                        const LoadOptions& options = {}) = 0;
  virtual void Cancel(uint64_t request_id) = 0;
  virtual void ClearCache() = 0;
};

}  // namespace native::ui
```

## DefaultGlide

```cpp
class DefaultGlide : public Glide {
public:
  DefaultGlide(size_t max_cache_bytes = 50 * 1024 * 1024,
               int thread_pool_size = 2);

  uint64_t Load(const std::string& path,
                LoadCallback cb,
                const LoadOptions& opts) override;
  void Cancel(uint64_t id) override;
  void ClearCache() override;

private:
  struct Request {
    std::string path;
    std::future<void> future;
    std::atomic<bool> cancelled{false};
  };

  std::mutex mutex_;
  LRUCache<std::string, std::shared_ptr<Image>> cache_;
  std::unordered_map<uint64_t, Request> requests_;
  uint64_t next_id_ = 1;
};
```

## ImageWidget Integration

```cpp
class ImageWidget : public Widget {
public:
  // New tags
  struct ImageURI { std::string value; };
  struct Placeholder { std::string value; };
  struct ErrorImage { std::string value; };

  ~ImageWidget() override;

  // Lifecycle
  void OnUnmount() override;
  void Draw(Canvas& canvas) override;

private:
  void LoadImage();
  void CancelLoad();

  ScaleMode scale_type_ = ScaleMode::kCenterCrop;
  Gravity scale_gravity_ = Gravity::kCenter;
  std::string uri_;
  std::shared_ptr<Image> loaded_image_;
  std::shared_ptr<Image> placeholder_image_;
  std::shared_ptr<Image> error_image_;
  LoadState state_ = LoadState::kLoading;
  uint64_t request_id_ = 0;
  std::string load_key_;
};
```

## Lifecycle

1. `ProcessArg(ImageURI("path"))` → store uri, call `LoadImage()`
2. `LoadImage()` → check `Glide::Default()` → if no loader, set `state_ = kError` and return
3. Cache lookup → hit → set `loaded_image_`, `state_ = kLoaded`, `RequestRedraw()`
4. Cache miss → set `state_ = kLoading`, show placeholder → `Glide::Load()` with callback
5. Callback on main thread → if `load_key_` matches → update `loaded_image_`/`state_` → `RequestRedraw()`
6. `CancelLoad()` or `OnUnmount()` → `Glide::Cancel(request_id_)`, clear `load_key_`
7. URI changes → `CancelLoad()` then `LoadImage()` again
