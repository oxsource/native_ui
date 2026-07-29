# Error Handling Strategy

**Last Updated**: 2026-07-29

## Principles

1. **No C++ exceptions.** Follow Google C++ Style Guide — exceptions are prohibited.
2. **StatusOr for recoverable errors.** Functions that can fail return `StatusOr<T>`.
3. **LogSink for diagnostics.** Non-fatal diagnostic messages go through the LogSink interface (see [logging-slot.md](logging-slot.md)).
4. **Fail fast for programming errors.** Assertions (`DCHECK`) for invariant violations in debug builds.

## StatusOr Pattern

```cpp
class StatusOr<T> {
  bool ok() const;
  T value() const;    // UB if !ok()
  Status status() const;
};

// Usage
StatusOr<Size> Measure(Size available);
```

| Scenario | Handling |
|----------|----------|
| Layout constraint overflow | StatusOr with error status, logged via LogSink |
| Resource load failure (image, font) | StatusOr with error status, caller decides fallback |
| Invalid widget tree (cycle, null parent) | DCHECK in debug, LogSink warning in release |
| State property type mismatch | LogSink error, no crash |

## LogSink Integration

Errors are NOT logged directly. Use the LogSink interface:

```cpp
// Framework calls this — consumer provides the implementation
class LogSink {
public:
  virtual ~LogSink() = default;
  virtual void Log(LogLevel level, const std::string& message,
                   const KeyValue* metadata = nullptr) = 0;
};

// Framework-wide registration
void SetLogSink(LogSink* sink);  // null sink = no-op
```

See [logging-slot.md](logging-slot.md) for the full LogSink specification.

## Assertions

```cpp
// Internal invariant checks — compiled out in release builds
#define DCHECK(expr) if (!(expr)) { /* log + abort in debug */ }
```

Use `DCHECK` for:
- Widget tree structural invariants (no cycles)
- Method preconditions (non-null pointers)
- State machine transitions (valid state for operation)
