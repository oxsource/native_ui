#pragma once

namespace native::ui {

class HardwareBuffer {
public:
#if __APPLE__
  static HardwareBuffer FromIOSurface(void* iosurface);
#elif __linux__
  static HardwareBuffer FromDmaBuf(int fd);
#endif

  HardwareBuffer() = default;

  bool IsValid() const { return valid_; }

#if __APPLE__
  void* iosurface() const { return iosurface_; }
#elif __linux__
  int dma_buf_fd() const { return dma_buf_fd_; }
#endif

private:
#if __APPLE__
  void* iosurface_ = nullptr;
#elif __linux__
  int dma_buf_fd_ = -1;
#endif
  bool valid_ = false;
};

}  // namespace native::ui
