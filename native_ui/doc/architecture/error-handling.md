# Error Handling Strategy

**Last Updated**: 2026-07-29

## Principles

1. **No C++ exceptions.** Follow Google C++ Style Guide — exceptions are prohibited.
2. **StatusOr for recoverable errors.** Functions that can fail return `StatusOr<T>`.
3. **LogSlot for diagnostics.** Non-fatal diagnostic messages go through the LogSlot interface (see [logging-slot.md](logging-slot.md)).
4. **Fail fast for programming errors.** Assertions (`DCHECK`) for invariant violations in debug builds.

## StatusOr Pattern

```cpp
namespace native::ui {

template<typename T>
class StatusOr {
  bool ok() const;
  T value() const;    // UB if !ok()
  Status status() const;
};

// Usage
StatusOr<Size> Measure(Size available);

}  // namespace native::ui
```

| Scenario | Handling |
|----------|----------|
| Layout constraint overflow | StatusOr with error status, logged via LogSlot |
| Resource load failure (image, font) | StatusOr with error status, caller decides fallback |
| Invalid widget tree (cycle, null parent) | DCHECK in debug, LogSlot warning in release |
| State property type mismatch | LogSlot error, no crash |

## LogSlot Integration

Errors are NOT logged directly. Use the LogSlot interface:

```cpp
namespace native::ui {

// Framework calls this — consumer provides the implementation
class LogSlot {
public:
  virtual ~LogSlot() = default;
  virtual void Log(LogLevel level, const std::string& message,
                   const KeyValue* metadata = nullptr) = 0;
};

// Framework-wide registration
void SetLogSlot(LogSlot* sink);  // null sink = no-op

}  // namespace native::ui
```

See [logging-slot.md](logging-slot.md) for the full LogSlot specification.

## Assertions

```cpp
// Internal invariant checks — compiled out in release builds
#define DCHECK(expr) if (!(expr)) { /* log + abort in debug */ }
```

Use `DCHECK` for:
- Widget tree structural invariants (no cycles)
- Method preconditions (non-null pointers)
- State machine transitions (valid state for operation)
