# Memory Model & Ownership

**Last Updated**: 2026-07-29

## Ownership Rules

| Ownership Pattern | Used For | Rationale |
|-------------------|----------|-----------|
| `std::unique_ptr<T>` | Widget child lists, exclusive ownership | Clear single-owner semantics, zero overhead |
| `T*` (raw pointer) | FindById results, event targets, parent references | Non-owning observation, no lifetime extension |
| `std::shared_ptr<T>` | **Avoid** | Only if truly shared ownership is required (State shared between widgets) |

## Widget Tree Ownership

```
Root Widget (unique_ptr owned by application)
  ├── Child A (unique_ptr in parent's children_ vector)
  ├── Child B (unique_ptr)
  │   └── Grandchild (unique_ptr)
  └── Child C (unique_ptr)
```

- Each widget **exclusively owns** its children via `std::unique_ptr`
- The root widget is owned by the application via `std::unique_ptr`
- Destruction is recursive: destroying a parent destroys all children

## Observation (Non-Owning)

```cpp
using namespace native::ui;

// FindById returns raw pointer — caller does not own
Widget* root->FindById("submit_btn");

// Event dispatch delivers raw pointer to target
void OnEvent(Widget* target, const Event& event);

// Parent reference is raw pointer
Widget* parent_;  // non-owning back-reference
```

- `FindById` returns `Widget*` — a non-owning reference
- Event handlers receive `Widget*` — must not delete
- Widget's `parent_` pointer must be cleared in `OnChildRemoved`

## Shared State

State objects may be shared between multiple widgets:

```cpp
using namespace native::ui;

auto state = std::make_shared<CounterState>();
// Both widgets watch the same State via Property<T> reference
text1->Watch(state->count);
text2->Watch(state->count);
```

This is one of the few legitimate uses of `shared_ptr` in the framework.

## Rules of Thumb

- If a widget creates it, it owns it (`unique_ptr`)
- If a widget finds it, it observes it (raw pointer)
- If two widgets need the same data, share a `State` (`shared_ptr`)
- Never `delete` a raw pointer received from the framework
