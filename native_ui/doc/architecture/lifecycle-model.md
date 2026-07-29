# Widget Lifecycle Model

**Last Updated**: 2026-07-29

## Lifecycle State Machine

```
                  ┌─────────────────────────────┐
                  │                             │
                  ▼                             │
┌─────────┐  ┌─────────┐  ┌──────────┐  ┌───────────┐  ┌───────┐
│ Created │→ │ Mounted │→ │ Measured │→ │ Arranged  │→ │ Ready │
└─────────┘  └─────────┘  └──────────┘  └───────────┘  └───┬───┘
                  │                                          │
                  │  ┌───────────────────────────────────────┘
                  │  │           │
                  │  │  RequestLayout()        RequestRedraw()
                  ▼  ▼           │                     │
             ┌──────────┐       │                     │
             │ Unmounted│◄──────┘                     │
             └──────────┘                             │
                                                      ▼
                                                  ┌───────┐
                                                  │ Ready │
                                                  └───────┘
```

| State | Entry Trigger | Description | Allowed Actions |
|-------|--------------|-------------|-----------------|
| `Created` | Constructor called | Widget object exists, not yet in tree | SetId, configure properties, AddChild |
| `Mounted` | Mount() called | Widget attached to parent tree | Measure() can be called |
| `Measured` | Measure() completes | Layout size computed, children measured | Arrange() |
| `Arranged` | Arrange() completes | Position computed, children positioned | Draw() |
| `Ready` | Draw() completes | Widget rendered on screen | RequestLayout()→Measured, RequestRedraw()→Ready |
| `Unmounted` | Unmount() called | Widget removed from tree | Destructor |

## State Transitions

### Mount
```
1. Widget is constructed (Created)
2. Container::AddChild(child) calls child->Mount()
3. Mount() sets parent reference, transitions to Mounted
4. Calls RequestLayout() on the subtree
```

### Measure → Arrange → Draw (Full Frame)
```
1. RequestLayout() marks widget + ancestors as dirty
2. At next frame: Measure() traverses tree, computes sizes
3. Arrange() positions children based on measured sizes
4. Draw() renders via Skia
5. Transition to Ready
```

### Invalidation
```
RequestLayout()   → marks layout dirty → re-M → re-A → re-D
RequestRedraw()   → marks visual dirty → re-D only (same layout)
```

### Unmount
```
1. Container::RemoveChild(child) calls child->Unmount()
2. Unmount() clears parent, transitions to Unmounted
3. Calls RequestLayout() on parent
4. Widget is destroyed (unique_ptr released)
```

## Virtual Methods

| Method | Called During | Override To |
|--------|-------------|-------------|
| `OnMount()` | After Mount | Initialize resources, watch States |
| `OnUnmount()` | Before Unmount | Release resources, unwatch States |
| `OnLayoutRequested()` | When RequestLayout propagates | Custom invalidation logic |
| `Draw(Canvas&)` | Draw phase | Render widget content |
