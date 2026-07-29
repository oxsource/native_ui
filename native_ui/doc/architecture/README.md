# Native UI — Architecture Overview

**Version**: 0.1 | **Last Updated**: 2026-07-29

## Purpose

This document is the entry point for the native_ui framework architecture. It describes the high-level module architecture, key design decisions, and provides cross-references to all detailed architecture documents.

---

## 8-Module Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    native_ui Library                     │
│                                                          │
│  ┌──────┐  ┌────────┐  ┌────────┐  ┌─────────┐         │
│  │ core │  │ layout │  │ render │  │ surface │         │
│  │      │  │        │  │        │  │         │         │
│  │ Rect │  │ Flex   │  │ Canvas │  │ Platform│         │
│  │ Point│  │ Layout │  │ Paint  │  │ Surface │         │
│  │ Size │  │ (Yoga) │  │ Path   │  │ Buffer  │         │
│  │ Color│  │        │  │        │  │ Handle  │         │
│  │ Edge │  │        │  │        │  │         │         │
│  │Insets│  │        │  │        │  │         │         │
│  └──────┘  └────────┘  └────────┘  └─────────┘         │
│       │           │           │            │            │
│       └──────┬────┘           │            │            │
│              │               │            │            │
│  ┌───────────▼────┐  ┌───────▼────────┐   │            │
│  │   viewmodel    │  │    widgets     │   │            │
│  │   (data bind)  │  │ Widget, Text   │   │            │
│   │    State       │  │ Button, Image  │   │            │
│  │   Property Not.│  │ Container,Stack│   │            │
│  └────────────────┘  └───────┬────────┘   │            │
│                              │            │            │
│  ┌───────────────────────────▼────────────▼─────────┐  │
│  │                    event                          │  │
│  │            HitTester, Event Dispatch              │  │
│  └───────────────────────┬──────────────────────────┘  │
│                          │                             │
│  ┌───────────────────────▼──────────────────────────┐  │
│  │                    public                         │  │
│  │        Umbrella header, export macro             │  │
│  └──────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

## Key Decisions Index

| # | Decision | Document |
|---|----------|----------|
| 1 | C++17 language standard, no exceptions | [error-handling.md](error-handling.md) |
| 2 | Module isolation via Bazel visibility | [module-dependencies.md](module-dependencies.md) |
| 3 | Skia isolated to render/ + surface/ only | [module-dependencies.md](module-dependencies.md) |
| 4 | unique_ptr ownership for widget tree | [memory-model.md](memory-model.md) |
| 5 | Widget lifecycle: Created → Mounted → Measured → Arranged → Ready → Unmounted | [lifecycle-model.md](lifecycle-model.md) |
| 6 | React-inspired `State` for data binding | [data-binding.md](data-binding.md) |
| 7 | Main-thread rendering + worker-thread logic | [threading.md](threading.md) |
| 8 | LogSlot slot interface for logging | [logging-slot.md](logging-slot.md) |
| 9 | StatusOr for recoverable errors | [error-handling.md](error-handling.md) |
| 10 | Tagged-parameter constructor pattern for widgets | See API contracts at `doc/api/` |

## Cross-References

| Document | Covers |
|----------|--------|
| [module-dependencies.md](module-dependencies.md) | Module diagram, dependency graph, Bazel visibility rules, Skia isolation policy |
| [error-handling.md](error-handling.md) | StatusOr strategy, no-exceptions convention, LogSlot-based diagnostics |
| [memory-model.md](memory-model.md) | unique_ptr ownership, raw pointer observation, shared_ptr avoidance |
| [lifecycle-model.md](lifecycle-model.md) | Widget lifecycle state machine, mount/unmount semantics |
| [data-binding.md](data-binding.md) | `State` pattern, property notification, batch RequestRedraw |
| [threading.md](threading.md) | Frame loop, worker threads, scheduling primitives |
| [logging-slot.md](logging-slot.md) | LogSlot interface, log levels, plug-in registration |

See `doc/api/` for interface contracts for each module.
