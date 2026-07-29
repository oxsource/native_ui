# Logging Slot Interface (LogSink)

**Last Updated**: 2026-07-29

## Overview

Logging uses a **slot interface** (plugin pattern) — the framework defines the abstract interface but does **not** provide an implementation. The consumer creates a concrete `LogSink` subclass and registers it at startup. If no sink is registered, log calls are no-ops.

## Interface

```cpp
// log_level.h
enum class LogLevel {
  kDebug,
  kInfo,
  kWarn,
  kError
};

// logging_slot.h
class LogSink {
public:
  virtual ~LogSink() = default;

  // Called by framework modules to emit a log message.
  // Must be thread-safe — can be called from any thread.
  virtual void Log(LogLevel level,
                   const std::string& message,
                   const KeyValue* metadata = nullptr) = 0;
};

// Registration — called once at application startup
// Passing nullptr sets the sink to no-op mode
void SetLogSink(LogSink* sink);
```

## Log Levels

| Level | Usage | Example |
|-------|-------|---------|
| `kDebug` | Development diagnostics | "Layout measure took 2.3ms" |
| `kInfo` | Normal operational events | "Widget tree mounted (12 widgets)" |
| `kWarn` | Unexpected but recoverable | "Image not found at path, using placeholder" |
| `kError` | Error conditions, not crashes | "StatusOr returned error in layout measure" |

## Structured Metadata (Optional)

```cpp
struct KeyValue {
  const char* key;
  const char* value;
};

// Usage
KeyValue meta[] = {{"widget_id", "btn_submit"}, {"duration_ms", "42"}};
sink->Log(LogLevel::kDebug, "Draw completed", meta);
```

## No-Op Behavior

When no `LogSink` is registered (`SetLogSink(nullptr)` or never called):

```cpp
// Compile-time no-op when sink is null
if (sink_) {
  sink_->Log(level, message, metadata);
}
```

This ensures zero overhead when logging is not configured — important for
embedded or performance-sensitive deployments.

## Thread Safety

`LogSink::Log` must be thread-safe. The framework calls it from:
- Main thread (during layout, render, event dispatch)
- Worker threads (during business logic, data processing)

The sink implementation is responsible for its own synchronization (e.g.,
mutex-protected write to file, or lock-free ring buffer for in-memory logging).

## Consumer Example

```cpp
class StderrLogSink : public LogSink {
public:
  void Log(LogLevel level, const std::string& message,
           const KeyValue* metadata) override {
    std::fprintf(stderr, "[%s] %s\n", LevelToString(level), message.c_str());
  }

private:
  static const char* LevelToString(LogLevel level) {
    switch (level) {
      case LogLevel::kDebug: return "DEBUG";
      case LogLevel::kInfo:  return "INFO";
      case LogLevel::kWarn:  return "WARN";
      case LogLevel::kError: return "ERROR";
    }
    return "UNKNOWN";
  }
};

// In application startup:
SetLogSink(new StderrLogSink());
```
