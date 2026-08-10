# Contract: Android AHardwareBuffer Utility (`AHwb`)

**Scope**: Android-only helper layer in `native_ui/src/framework/surface/ahwb.h/.cc`, modeled on `falcon/core/utils/ahwb.cc`.

## Status Codes

| Code | Meaning |
|------|---------|
| `1` | Success |
| `0` | Success, no-op |
| `< 0` | Error (negative; `-1` invalid arg, `-2` empty/unsupported, `-3` allocation/lock failure, `-5` non-Android platform) |

All functions are guarded by `#if defined(__ANDROID__)`; on host builds they compile to stubs returning `-5`.

## API

| Signature | Contract |
|-----------|----------|
| `bool Describe(AHardwareBuffer*, uint32_t& w, uint32_t& h, uint32_t& stride, int& format)` | Wraps `AHardwareBuffer_describe`; returns false on null buffer or failure. |
| `int Lock(AHardwareBuffer*, uint64_t usage, void** data)` | `AHardwareBuffer_lock` with fence `-1`, rect `nullptr`. |
| `int Unlock(AHardwareBuffer*)` | `AHardwareBuffer_unlock` with fence `nullptr`. |
| `int Pixels(AHardwareBuffer*, uint64_t usage, fn<void(void*)>)` | Lock → invoke → unlock. **Unlock is guaranteed** even if `fn` throws (no exceptions) or returns early. |
| `AHardwareBuffer* AllocateRGBA(uint32_t w, uint32_t h)` | `AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM`; usage `CPU_READ_OFTEN \| CPU_WRITE_OFTEN \| GPU_SAMPLED_IMAGE \| GPU_COLOR_OUTPUT`; layers=1. Returns null on failure. |
| `int WriteRGBA(AHardwareBuffer*, const uint8_t* src, size_t src_row_bytes)` | Lock `CPU_WRITE_OFTEN`; copy `height` rows of `min(width*4, src_row_bytes, dst_row_bytes)`; unlock. |
| `void Release(AHardwareBuffer*)` | `AHardwareBuffer_release`; null-safe. |
| `sk_sp<SkImage> ToCpuImage(AHardwareBuffer*, bool copy=true)` | Describe → lock `CPU_READ_OFTEN` → build RGBA8888 `SkImage` (owned copy honoring `desc.stride`) → unlock. Null on failure. |
| `sk_sp<SkImage> ToGpuImage(AHardwareBuffer*, GrDirectContext*)` | Wrap as `GrBackendTexture` (`GrAHardwareBufferUtils::GetBackendTexture`) → `SkImages::AdoptTextureFrom`. Zero-copy. Requires non-null context; else null. |
| `int DumpPng(AHardwareBuffer*, const char* path)` | Read via CPU lock → encode PNG → write. Diagnostics only (FR-010). |

## Constraints

- Producer owns the buffer: `AHwb` never releases a buffer it did not allocate.
- Supported format: `R8G8B8A8_UNORM`. Other formats are rejected (`ToCpuImage`/`ToGpuImage` return null) — defined error state, never corrupt output.
- Stride: all row copies use the destination stride; padding bytes are never interpreted as pixels.
- No exceptions; all failures surfaced via status codes / null returns.
