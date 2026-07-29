# Threading Model

**Last Updated**: 2026-07-29

## Architecture

```
┌──────────────────────────────────────────────────────────────┐
│                    MAIN THREAD (Frame Loop)                    │
│                                                               │
│  ┌──────────┐  ┌───────┐  ┌───────────┐  ┌────────┐         │
│  │ PostTask │→│ Event  │→│ Batch View │→│ Layout │         │
│  │ Queue    │  │Dispatch│  │Model Change│  │Measure  │         │
│  └──────────┘  └───────┘  └───────────┘  │Arrange │         │
│                                           └────────┘         │
│                                               │               │
│  ┌──────────────┐  ┌──────────────────┐       │               │
│  │PostNextFrame  │←│Skia Render (Draw)│◄──────┘               │
│  │Callbacks      │  └──────────────────┘                       │
│  └──────┬───────┘                                              │
│         │                                                      │
│         ▼                                                      │
│  ┌──────────────┐                                              │
│  │ Wait vsync   │──→ (next frame) → PostTask Queue            │
│  └──────────────┘                                              │
└──────────────────────────────────────────────────────────────────┘
                         ▲
                          │  State Property Notification
                         │  (cross-thread, delivered on main thread)
                         │
┌────────────────────────────────────────────────────────────────┐
│                    WORKER THREADS                                │
│                                                                 │
│  ┌────────────────────────────┐  ┌─────────────────────────┐   │
│  │ Business Logic             │  │ Data Processing / I/O   │   │
│  │  (state->x = val)          │  │  (state->x = val)     │   │
│  └────────────────────────────┘  └─────────────────────────┘   │
└──────────────────────────────────────────────────────────────────┘
```

## Frame Loop (React-Style Batch Model)

The main thread runs a continuous frame loop:

```text
1. Process PostTask queue        (high priority, FIFO)
2. Dispatch pending events       (HitTest → Bubble/Capture)
3. Batch State changes           (React-style setState coalescing)
4. If layout dirty:
   a. Measure(available_size)
   b. Arrange(measured)
5. If visual dirty:
   a. Draw(Canvas) for each dirty widget
   b. Children Draw recursively
6. Fire PostNextFrame callbacks  (post-render)
7. Wait for next vsync
8. Go to 1
```

## Thread Responsibility

| Work | Thread | Must Not |
|------|--------|----------|
| Event dispatch | Main | Block on I/O |
| Layout (Measure + Arrange) | Main | Assign State properties (state->x = val) |
| Skia rendering (Draw) | Main | Mutate widget tree |
| State property notification | Main (delivery) | — |
| Business logic | Worker | Call Skia APIs |
| Data processing / I/O | Worker | Access widget tree directly |
| State property mutation | Worker (thread-safe) | Block main thread |

## Scheduling Primitives

| Primitive | Behavior | React Analog |
|-----------|----------|-------------|
| `PostTask(callback)` | Queued before next frame's render phase | `queueMicrotask` |
| `PostNextFrame(callback)` | Executed after current frame's render completes | `useEffect` |
| `ScheduleTimer(delay_ms, callback)` | Executed after delay, may cross multiple frames | `setTimeout` |

## State Cross-Thread Bridge

```
Worker Thread                     Main Thread
─────────────────                ─────────────────
    state->count = 42;  // Property<T>::operator=
        │
        ▼
Lock mutex, update value
Signal(key)
Queue notification
        │
        │  ─────────────────────►  Frame loop step 3:
        │                           Batch → coalesce → RequestRedraw
        │
        │  ◄────────────────────  Frame loop step 6:
                                   PostNextFrame (if registered)
```

## Thread Safety Rules

1. `Property<T>::operator=` is thread-safe (mutex-protected inside State)
2. State property change notification is always delivered on the main thread
3. Widget tree mutations (AddChild, RemoveChild) are main-thread only
4. Skia APIs are main-thread only
5. LogSink::Log must be thread-safe (called from any thread)
6. `PostTask` and `PostNextFrame` are main-thread only
