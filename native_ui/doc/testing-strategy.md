# Testing Strategy

**Last Updated**: 2026-07-29

## Test Levels

| Level | Framework | Scope | Location |
|-------|-----------|-------|----------|
| Unit | googletest | Single class / function | `tests/*_test.cc` |
| Integration | googletest | Cross-module interaction | `tests/integration/` |
| Golden | googletest + PNG hash | Visual output correctness | `tests/golden/` |

## Unit Tests

- One `*_test.cc` per module
- Mock patterns: `Widget` via subclass with `Draw` override; `Canvas` via pixel readback
- No platform dependencies — tests run headless

| Module | Test File | Key Tests |
|--------|-----------|-----------|
| `core` | `tests/core_test.cc` | Rect, Point, Size, Color, EdgeInsets |
| `state` | `tests/state_test.cc` | State property notification, Watch/Unwatch, thread-safe update |
| `layout` | `tests/layout_test.cc` | FlexLayout Measure/Arrange, direction, gap, wrap |
| `widgets` | `tests/widget_test.cc` | Widget ID, Container add/remove, Draw invalidation |

## Integration Tests

- Test cross-module pipelines: `Container → FlexLayout → Canvas → pixel`
- Location: `tests/integration/container_layout_test.cc`
- Verify: layout output matches expected widget positions

## Golden Image Tests

- Render a known scene → encode PNG → compare hash against `tests/golden/baseline/`
- CI fails if hash diverges (detects visual regressions)
- Platform-specific baselines for macOS vs Linux pixel differences

## Coverage Targets

| Metric | Target |
|--------|--------|
| Core types | 90%+ line coverage |
| Layout engine | 85%+ line coverage |
| Widget lifecycle | 80%+ line coverage |
| CI pipeline gate | < 70% blocks PR |
