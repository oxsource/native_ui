# Logging Slot Interface (LogSlot)

**Last Updated**: 2026-07-29

## Overview

Logging uses a **slot interface** (plugin pattern) — the framework defines the abstract interface but does **not** provide an implementation. The consumer creates a concrete `LogSlot` subclass and registers it at startup. If no sink is registered, log calls are no-ops.

## Interface

```cpp
namespace native::ui {

// log_level.h
enum class LogLevel {
  kInfo,    // 0 — lowest severity
  kDebug,   // 1
  kWarn,    // 2
  kError    // 3 — highest severity
};

// logging_slot.h
class LogSlot {
public:
  virtual ~LogSlot() = default;

  // Called by framework modules to emit a log message.
  // Must be thread-safe — can be called from any thread.
  virtual void Log(LogLevel level,
                   const std::string& message,
                   const KeyValue* metadata = nullptr) = 0;
};

// Registration — called once at application startup
// Passing nullptr sets the sink to no-op mode
void SetLogSlot(LogSlot* sink);

}  // namespace native::ui
```

## Log Levels

| Level | Severity | Usage | Example |
|-------|----------|-------|---------|
| `kInfo` | 0 — lowest | Normal operational events | "Widget tree mounted (12 widgets)" |
| `kDebug` | 1 | Development diagnostics | "Layout measure took 2.3ms" |
| `kWarn` | 2 | Unexpected but recoverable | "Image not found at path, using placeholder" |
| `kError` | 3 — highest | Error conditions, not crashes | "StatusOr returned error in layout measure" |

## Structured Metadata (Optional)

```cpp
namespace native::ui {

struct KeyValue {
  const char* key;
  const char* value;
};

// Usage
KeyValue meta[] = {{"widget_id", "btn_submit"}, {"duration_ms", "42"}};
sink->Log(LogLevel::kDebug, "Draw completed", meta);

}  // namespace native::ui
```

## No-Op Behavior

When no `LogSlot` is registered (`SetLogSlot(nullptr)` or never called):

```cpp
// Compile-time no-op when sink is null
if (sink_) {
  sink_->Log(level, message, metadata);
}
```

This ensures zero overhead when logging is not configured — important for
embedded or performance-sensitive deployments.

## Thread Safety

`LogSlot::Log` must be thread-safe. The framework calls it from:
- Main thread (during layout, render, event dispatch)
- Worker threads (during business logic, data processing)

The sink implementation is responsible for its own synchronization (e.g.,
mutex-protected write to file, or lock-free ring buffer for in-memory logging).

## Consumer Example

```cpp
using namespace native::ui;

class StderrLogSlot : public LogSlot {
public:
  void Log(LogLevel level, const std::string& message,
           const KeyValue* metadata) override {
    std::fprintf(stderr, "[%s] %s\n", LevelToString(level), message.c_str());
  }

private:
  static const char* LevelToString(LogLevel level) {
    switch (level) {
      case LogLevel::kInfo:  return "INFO";
      case LogLevel::kDebug: return "DEBUG";
      case LogLevel::kWarn:  return "WARN";
      case LogLevel::kError: return "ERROR";
    }
    return "UNKNOWN";
  }
};

// In application startup:
SetLogSlot(new StderrLogSlot());
```
