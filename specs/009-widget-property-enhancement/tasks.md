---

description: "Task list for Phase 9: Widget Property Enhancement"

---

# Tasks: Widget Property Enhancement

**Input**: Design documents from `specs/009-widget-property-enhancement/`

**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md, contracts/

**Tests**: Test tasks are included per the spec's acceptance criteria.

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

All paths are relative to `native_ui/` under the repository root.

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Create shared data types and utility headers used by all phases

- [ ] T001 Create `Gradient` type in `native_ui/src/framework/core/gradient.h` with `Linear(from, to, stops)` and `Radial(center, radius, stops)` factories, `ColorStop` struct; update `native_ui/src/framework/core/BUILD.bazel` glob picks it up automatically
- [ ] T002 [P] Create `LRUCache` template in `native_ui/src/framework/widgets/lru_cache.h` — `list` + `unordered_map` backed, max_bytes capacity, `Get(key)`, `Put(key, value)`, `Clear()`

---

## Phase 2: Foundation — Style Class (Priority: P1)

**Purpose**: Style with StylePriority, per-property is_set flags, Merge algorithm, SetDefault global singleton. BLOCKS all property work since ApplyStyle depends on it.

- [ ] T003 Create `Style` header in `native_ui/src/framework/widgets/style.h` with `StylePriority` enum (kGlobal=100→kExplicit=500), chainable setters for all 22+ properties, per-property `is_set` storage, `priority()`, `setPriority()`
- [ ] T004 Implement `Style` in `native_ui/src/framework/widgets/style.cc` — `SetDefault()` static (main-thread-only `Glide*` global), `Default()` accessor, `Merge(base, overlay)` free function (per-property: if overlay.is_set && overlay.priority >= base.priority → overlay wins)

**Checkpoint**: Style class compiles, Merge works, SetDefault/Default pair works.

---

## Phase 3: User Story 1 - Widget Base Properties (Priority: P1)

**Goal**: All widget types support Width, Height, MinWidth, MaxWidth, Padding, Background, BackgroundGradient, Enabled, Visible, Opacity, CornerRadius, BorderWidth, BorderColor, Shadow. ApplyStyle integrates with Style.

**Independent Test**: Create a Text widget with Width(200), Height(48), Background(kBlue), CornerRadius(8), verify layout respects constraints and background renders correctly.

- [ ] T005 [P] [US1] Add common visual property tags (`Width`, `Height`, `MinWidth`, `MaxWidth`, `Padding`, `Background`, `BackgroundGradient`, `Enabled`, `Visible`, `Opacity`, `CornerRadius`, `BorderWidth`, `BorderColor`, `ShadowOffset`, `ShadowRadius`, `ShadowColor`) to `native_ui/src/framework/widgets/widget.h` and `widget.cc`
- [ ] T006 [US1] Add `ApplyStyle(const Style&)` method to Widget base in `native_ui/src/framework/widgets/widget.h/cc` — merges incoming Style's set properties into widget's stored fields (overwrites if is_set and priority allows)
- [ ] T007 [US1] Update `Container` in `native_ui/src/framework/widgets/container.h/cc` — remove `Size{}` tag and `layout_size_` field (replaced by inherited Widget base Width/Height properties); update `Layout()` to use `Width()`/`Height()` tags instead
- [ ] T008 [US1] Implement Widget base Draw-time property rendering in `native_ui/src/framework/widgets/widget.cc` — draw background rect, apply CornerRadius, apply Opacity via canvas alpha, draw border, draw shadow (Skia shadow API)

**Checkpoint**: Widget base properties render correctly — background, border, corner radius, opacity all visible in output.

---

## Phase 4: User Story 2 - Text Typography Properties (Priority: P1)

**Goal**: Text widget owns FontSize, TextColor, TextAlign, FontFamily, FontWeight, LineHeight, MaxLines, TextDecoration. All tags work via constructor and Style.

**Independent Test**: Create Text with FontSize(24), TextColor(kRed), FontWeight(700), TextAlign(kCenter), render and verify pixel output matches expected glyph size, color, weight, and horizontal centering.

- [ ] T009 [P] [US2] Add typography tags (`FontSize`, `TextColor`, `TextAlign`, `FontFamily`, `FontWeight`, `LineHeight`, `MaxLines`, `TextDecoration`) to `Text` in `native_ui/src/framework/widgets/text.h` and `text.cc`
- [ ] T010 [US2] Update `Text::Draw` in `native_ui/src/framework/widgets/text.cc` — use stored FontSize/TextColor/FontFamily for SkFont construction, apply TextAlign for horizontal positioning, apply LineHeight/MaxLines for multi-line layout, apply TextDecoration
- [ ] T011 [US2] Update `native_ui/src/framework/render/canvas.cc` — add shadow rendering support (Skia `SkDrawShadowRec` or `SkCanvas::drawShadow`), add gradient shader support via `SkGradientShader` for BackgroundGradient rendering

**Checkpoint**: Text renders with correct font size, color, weight, alignment, and decoration.

---

## Phase 5: User Story 3 - Button Inherits Text + Interactive States (Priority: P2)

**Goal**: Button changes base class from Widget to Text, gaining all text/typography properties. Adds NormalColor, PressedColor. Enabled kicks in from Widget base.

**Independent Test**: Create Button with FontSize(18), TextColor(kWhite), NormalColor(kBlue), PressedColor(kDarkBlue). Push MouseEvent and verify color transitions.

- [ ] T012 [P] [US3] Rebase `Button` from `Widget` to `Text` in `native_ui/src/framework/widgets/button.h` — change `class Button : public Widget` to `class Button : public Text`; remove duplicate Content/Watch code (inherited from Text); add `NormalColor`, `PressedColor` tags
- [ ] T013 [US3] Implement `Button::Draw` in `native_ui/src/framework/widgets/button.cc` — call `Text::Draw` for label rendering, then draw state color overlay (NormalColor normally, PressedColor when pressed); if `Enabled(false)`, apply dimming overlay
- [ ] T014 [US3] Update `Button::OnMouseEvent` in `native_ui/src/framework/widgets/button.cc` — check `Enabled()` before dispatching click; set internal pressed state for visual feedback; clear on release

**Checkpoint**: Button renders with correct state colors, inherits Text properties, disabled blocks clicks.

---

## Phase 6: User Story 1 - Glide Async Image Loading (Priority: P1)

**Goal**: Glide singleton with async file decode, LRU cache. ImageWidget triggers Load on ImageURI, shows Placeholder/ErrorImage, cancels on destruction.

**Independent Test**: Create ImageWidget with ImageURI pointing to a valid PNG, render, verify image appears. Destroy widget before callback fires, verify callback is NOT invoked.

- [ ] T015 [P] [US1] Create `Glide` abstract base class in `native_ui/src/framework/widgets/glide.h` — `Load(path, callback, options)`, `Cancel()`, `ClearCache()`, `SetDefault()`/`Default()` static API; `DefaultGlide` implementation using `std::async` for decode + `LRUCache` for caching
- [ ] T016 [US1] Create `native_ui/src/framework/widgets/glide.cc` — `DefaultGlide` implementation: `Load` submits decode via `std::async`, checks `LRUCache` first, invokes callback on main thread; `Cancel` marks request cancelled, callback checks flag
- [ ] T017 [US1] Add `ImageURI`, `Placeholder`, `ErrorImage`, `ScaleType`, `ScaleGravity` tags to `ImageWidget` in `native_ui/src/framework/widgets/image_widget.h/cc` — integrate with Glide for async loading
- [ ] T018 [US1] Implement `ImageWidget::Draw` in `native_ui/src/framework/widgets/image_widget.cc` — state machine: kLoading→Placeholder, kLoaded→ScaleType transform + draw, kError→ErrorImage; `Load()`/`CancelLoad()` lifecycle with `load_key_` stale callback guard

**Checkpoint**: ImageWidget loads file asynchronously, shows scaled image, cancels properly on destruction.

---

## Phase 7: User Story 5 - Hello World Beautification (Priority: P3) 🎯 FINAL

**Goal**: Redesign Hello World using all new properties — Style theme, Background, CornerRadius, Shadow, Padding, TextColor, FontSize, TextAlign, Button state colors.

**Independent Test**: `bazel run //examples:hello_world` produces a beautiful styled PNG with rounded buttons, shadows, themed colors, centered text, and consistent spacing.

- [ ] T019 [P] [US5] Update `examples/hello_world.cc` — define a `Style` theme (font, colors, spacing), apply to Container (Background, CornerRadius, Shadow, Padding), Text (FontSize, TextColor, TextAlign, FontWeight), Button (NormalColor, PressedColor, CornerRadius)
- [ ] T020 [US5] Add image section to Hello World — use `ImageWidget` with `ScaleType(kCenterCrop)` to display a demo image loaded via `Glide` (synchronous fallback if Glide not initialized)

**Checkpoint**: Hello World output PNG shows polished UI with all new properties in use.

---

## Phase 8: Tests & Verification

**Purpose**: Unit tests for Style, Glide, property rendering, and Hello World verification

- [ ] T021 [P] Write Style tests in `native_ui/tests/style_test.cc` — chainable setters, is_set flags, Merge priority, SetDefault/Default, ApplyStyle
- [ ] T022 [P] Write Widget base property tests in `native_ui/tests/widgets_test.cc` — Background, CornerRadius, Opacity, Border pixel verification
- [ ] T023 [P] Write Text typography tests in `native_ui/tests/widgets_test.cc` — FontSize, TextColor, TextAlign, FontWeight, MaxLines rendering
- [ ] T024 [P] Write Button state tests in `native_ui/tests/widgets_test.cc` — NormalColor/PressedColor rendering, Enabled(false) blocks click, Text property inheritance
- [ ] T025 [P] Write Glide tests in `native_ui/tests/glide_test.cc` — async load, cache hit, cancel, callback not invoked after destruction
- [ ] T026 [P] Write ImageWidget tests in `native_ui/tests/widgets_test.cc` — ScaleType visual output, Placeholder/ErrorImage states, Glide integration
- [ ] T027 Add `style_test` and `glide_test` cc_test targets in `native_ui/tests/BUILD.bazel` with appropriate deps
- [ ] T028 Verify full build and all tests: `bazel build //...` and `bazel test //...`

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies
- **Foundation (Phase 2)**: Depends on Setup — BLOCKS all property work
- **US1 Widget Base (Phase 3)**: Depends on Foundation (Style used in ApplyStyle)
- **US2 Text (Phase 4)**: Depends on US1 (Widget base properties)
- **US3 Button (Phase 5)**: Depends on US2 (Text is now base class)
- **US1 Glide (Phase 6)**: Depends on Setup (LRU cache) — independent of other widget properties
- **US5 Hello World (Phase 7)**: Depends on US1 + US2 + US3 + US4
- **Tests (Phase 8)**: Depends on all phases

### User Story Dependencies

- **US1 (Widget Base) P1**: Depends on Style foundation
- **US2 (Text Typography) P1**: Depends on Widget base
- **US3 (Button States) P2**: Depends on Text (base class)
- **US4 (Glide + Image) P1**: Depends on Setup only — fully parallel with US1/US2/US3
- **US5 (Hello World) P3**: Depends on all other stories

### Parallel Opportunities

- T001+T002 (Setup) — [P], different files
- T003+T004 (Style) — sequential
- T005+T006+T007+T008 (US1) — T005 is [P], rest sequential
- T009+T010+T011 (US2) — T009+T011 are [P], T010 sequential
- T012+T013+T014 (US3) — T012 is [P], rest sequential
- T015+T016+T017+T018 (US4 Glide+Image) — T015+T017 are [P], rest sequential
- T019+T020 (US5) — sequential within story
- T021~T027 (Tests) — all [P], independent test files
- US4 (Glide) can run IN PARALLEL with US1/US2/US3 since it's independent
- US1 and US2 must be sequential (Widget base → Text)

---

## Parallel Example

```bash
# US1 (Widget Base) + US4 (Glide) in parallel:
Task: "Add base property tags to widget.h and widget.cc"
Task: "Create Glide singleton and DefaultGlide implementation"

# US2 (Text) + US4 (Image) in parallel after US1:
Task: "Add typography tags to text.h and text.cc"
Task: "Add ScaleType and ImageURI to image_widget.h and image_widget.cc"

# US3 (Button) after US2:
Task: "Rebase Button from Widget to Text"
```

---

## Implementation Strategy

### MVP First (Widget Base + Text + Button)

1. Complete Phase 1 + 2 (Setup + Style)
2. Complete Phase 3: US1 Widget Base properties
3. Complete Phase 4: US2 Text typography
4. Complete Phase 5: US3 Button states
5. **STOP and VALIDATE**: `bazel test //tests:widgets_test`

### Incremental Delivery

1. Style system + Widget base properties → foundation ready
2. Text typography → rich text rendering
3. Button states → interactive buttons
4. Glide + Image → async image loading with cache
5. Hello World beautification → visual validation
6. All tests green

---

## Notes

- [P] tasks = different files, no dependencies
- [Story] label maps task to specific user story for traceability
- Style lives in widgets module (accessed via includes from widget.h)
- Gradient type lives in core module (auto-picked by glob)
- Button::Draw calls Text::Draw first, then overlays state color
- Glide::Default() is a raw pointer global (main-thread-only set)
- LRUCache template is header-only (no .cc file)
- Container's existing `Size{}` tag must be removed to avoid ambiguity with Widget base Width/Height tags
- hello_world.cc needs Glide::SetDefault() call or fallback for synchronous load
