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
 │  │ FrameClock::  │──→ (next frame) → PostTask Queue
 │  │WaitNextFrame│            │
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
7. Wait for FrameClock::WaitForNextFrame()
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

## FrameClock (Frame Tick Abstraction)

`FrameClock` is a **utility provided to the consumer** for driving the frame loop. The library does not own or call it — the consumer uses it in their own loop to control frame timing.

```cpp
namespace native::ui {

class FrameClock {
public:
  virtual ~FrameClock() = default;
  // Blocks until the next frame should begin.
  // The caller (frame loop) resumes immediately after this returns.
  virtual void WaitForNextFrame() = 0;
};

}  // namespace native::ui
```

### Implementations

| Implementation | Drive Model | WaitForNextFrame Behavior | Use Case |
|---------------|-------------|--------------------------|----------|
| `VSyncClock` | Clock-driven | Blocks until physical vsync signal via CVDisplayLink (macOS) or KMS/DRM (Linux) | Production GUI, 60fps vertical-synced rendering |
| `TimerClock` | Clock-driven | `std::this_thread::sleep_for(16ms)` — fixed interval, no display hardware | CI tests, headless/server rendering, fallback |
| `SwapChainClock` | **Producer-driven** | Blocks on a semaphore; external producer calls `SwapBuffer()` to release one frame | DVR pipeline, video playback, camera preview, AI inference overlay — any scenario where buffer availability drives render |

### Producer-Driven Flow (SwapChainClock)

```
Producer Thread                    Main Thread (Frame Loop)
─────────────────                  ─────────────────────────
Fill buffer (GPU/CPU)
        │
SwapBuffer()                       WaitForNextFrame()
  → sem_.release()                     │
        │                              ▼ sem_.acquire()
        │                         resumed → render frame
```

`SwapChainClock` is used when rendering is driven by external buffer availability — for example, a DVR pipeline calls `SwapBuffer()` each time a new frame is ready, and the consumer's loop renders it via Skia.

### Manual Clock for Unit Testing

`SwapChainClock` also serves as a manual clock in tests — `WaitForNextFrame()` blocks, test code calls `SwapBuffer()` to advance frame by frame, no real-time dependency.

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

## Frame Loop Ownership

The frame loop is **owned by the consumer**, not by native_ui. The library provides the building blocks (`FrameClock`, `LayoutEngine`, `Canvas`, `EventDispatcher`) but does not run the loop itself. The consumer drives the frame loop at their own pace:

```
EventHub hub;

while (running) {
  clock->WaitForNextFrame();              // consumer's clock
  ProcessPlatformEvents();                // consumer polls platform
  while (auto event = NextPlatformEvent()) {
    hub.Push(ConvertToNativeUIEvent(event)); // single entry point
  }
  if (layout_dirty) { Measure(); Arrange(); }
  if (visual_dirty) { Renderer::Draw(root, canvas); }
}
```

This allows the consumer to:
- Use any clock (vsync, timer, producer-driven `SwapBuffer`)
- Own the platform event loop (NSRunLoop, xcb poll, etc.)
- Integrate native_ui rendering into an existing render pipeline
- Run multiple independent frame loops (one per window) without library involvement

## Thread Safety Rules

1. `Property<T>::operator=` is thread-safe (mutex-protected inside State)
2. State property change notification is always delivered on the main thread
3. Widget tree mutations (AddChild, RemoveChild) are main-thread only
4. Skia APIs are main-thread only
5. LogSlot::Log must be thread-safe (called from any thread)
6. `PostTask` and `PostNextFrame` are main-thread only
7. `SwapChainClock::SwapBuffer` is thread-safe (can be called from any thread); the consumer is responsible for calling `WaitForNextFrame` from their frame loop thread
