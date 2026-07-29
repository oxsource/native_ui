---

description: "Task list for Architecture & Engineering Design"

---

# Tasks: Architecture & Engineering Design

**Input**: Design documents from `specs/002-architecture-engineering-design/`

**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/

**Tests**: Tests are NOT requested in this feature specification — this phase produces design artifacts and CI configuration only (FR-009). No runtime code.

**Organization**: Tasks are grouped by user story to enable independent implementation and validation of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

All paths are relative to the Bazel workspace root (`native_ui/`).

---

## Phase 1: Architecture Design Records (User Story 1) 🎯 MVP

**Purpose**: Document 8-module architecture, dependency graph, error handling (StatusOr + LogSink), memory ownership (unique_ptr), widget lifecycle, data binding (React-inspired ViewModel), threading (main-thread frame loop + worker threads), and logging slot (LogSink interface)

- [ ] T001 [P] [US1] Create `native_ui/doc/architecture/README.md` with architecture overview (8-module diagram), key decisions index, and cross-references to all other architecture docs
- [ ] T002 [P] [US1] Create `native_ui/doc/architecture/module-dependencies.md` with formal module dependency graph (core → layout/render/viewmodel → widgets → public), Bazel visibility rules, and Skia isolation policy
- [ ] T003 [P] [US1] Create `native_ui/doc/architecture/error-handling.md` with StatusOr strategy for recoverable errors, no-exceptions convention, and LogSink-based diagnostic logging (cross-ref logging-slot.md)
- [ ] T004 [P] [US1] Create `native_ui/doc/architecture/memory-model.md` with unique_ptr ownership for widget tree, raw pointer observation (FindById, event dispatch), and no shared_ptr rule
- [ ] T005 [P] [US1] Create `native_ui/doc/architecture/lifecycle-model.md` with Widget lifecycle state machine (Created → Mounted → Measured → Arranged → Ready → Unmounted) and mount/unmount semantics
- [ ] T006 [US1] Create `native_ui/doc/architecture/data-binding.md` with React-inspired ViewModel pattern — property notification, Bind/Unbind lifecycle, automatic batch RequestRedraw (React-style setState coalescing within frame), and unidirectional data flow (props ↓, events ↑)
- [ ] T007 [US1] Create `native_ui/doc/architecture/threading.md` with main-thread frame loop (React-style batch model: PostTask → Event → Batch VM changes → Layout → Render → PostNextFrame → Wait vsync), worker-thread logic processing, ViewModel cross-thread bridge (thread-safe property update, main-thread notification), and scheduling primitives (PostTask pre-render, PostNextFrame post-render, ScheduleTimer cross-frame)
- [ ] T008 [US1] Create `native_ui/doc/architecture/logging-slot.md` with LogSink abstract interface (Log(level, message, metadata)), 4 log levels (debug/info/warn/error), no-op fallback when no sink registered, thread-safe contract, and SetLogSink registration API
- [ ] T009 [US1] Add viewmodel module stub at `src/framework/viewmodel/BUILD.bazel` as empty `cc_library` depending on `//src/framework/core`

**Checkpoint**: All 8 architecture design documents (README, module-dependencies, error-handling, memory-model, lifecycle-model, data-binding, threading, logging-slot) are written and internally consistent. A developer can understand the full 8-module architecture by reading them.

---

## Phase 2: API & Interface Contracts (User Story 2)

**Purpose**: Define public API contracts for all 8 framework modules (core, layout, render, surface, viewmodel, widgets, event, public)

- [ ] T010 [P] [US2] Create `native_ui/doc/api/widget-contract.md` with Widget virtual interface (Draw, ChildAt, ChildCount), extension points for custom widgets, tagged-parameter constructor convention, and RequestLayout/RequestRedraw invalidation protocol
- [ ] T011 [P] [US2] Create `native_ui/doc/api/layout-contract.md` with FlexLayout interface, Measure/Arrange protocol, tagged parameters (Direction, JustifyContent, AlignItems, Gap, Padding, Margin), and guide for adding new layouts
- [ ] T012 [P] [US2] Create `native_ui/doc/api/render-contract.md` with Canvas RAII wrapper (auto save/restore), Paint chainable builder, Path construction, and Skia isolation rules (only render/ + surface/ may depend on Skia)
- [ ] T013 [P] [US2] Create `native_ui/doc/api/event-contract.md` with event types (Mouse, Key, Touch), DFS hit testing, bubble/capture dispatch protocol, and stop-propagation semantics
- [ ] T014 [US2] Create `native_ui/doc/api/viewmodel-contract.md` with ViewModel base class, property notification API (NotifyPropertyChanged), Bind/Unbind lifecycle, thread-safe update contract, communication protocol between worker threads (update) → main thread (notification + auto batch → RequestRedraw), and LogSink registration API

**Checkpoint**: A widget implementer can create a custom widget by following these contracts without consulting the architecture team.

---

## Phase 3: Engineering Standards & CI/CD (User Story 3)

**Purpose**: Establish testing strategy, build conventions, CI pipeline, agent instructions, and automated quality gates

- [ ] T015 [P] [US3] Create `native_ui/doc/testing-strategy.md` with unit test structure (googletest), mock patterns for Widget/Canvas, integration test scopes, golden image test plan (PNG hash comparison), and coverage targets
- [ ] T016 [P] [US3] Create `native_ui/doc/build-conventions.md` with BUILD file conventions (cc_library per module), dep prefix rules (//src/framework for internal, @ for external), visibility templates (__subpackages__ for internal, public only for public target), and Skia isolation enforcement
- [ ] T017 [P] [US3] Create `native_ui/doc/agent-instructions.md` with standard prompt template for opencode agents working on native_ui — including Google C++ style checklist, Bazel build/test commands, and commit convention reminder
- [ ] T018 [P] [US3] Create `native_ui/doc/ci-strategy.md` with CI architecture doc — which checks run when (build → test → format → lint → visibility), Bazel remote caching strategy, matrix platforms (macOS ARM64 + Linux x86_64), and expected run times
- [ ] T019 [US3] Create `.github/workflows/ci.yml` with CI pipeline: `bazel build //...`, `bazel test //...`, clang-format --dry-run --Werror, clang-tidy on changed files, Bazel visibility query (`bazel query 'somepath(//src/framework/..., @skia//:skia)'`) verifying only render/ + surface/ depend on Skia
- [ ] T020 [US3] Create `.github/workflows/pr.yml` with PR gate: same checks as CI + mandatory review approval requirement
- [ ] T021 [US3] Create `.github/workflows/release.yml` with release workflow: git tag (SemVer) → `bazel build //src/framework/public:native_ui_shared` → attach .dylib/.so artifact → create GitHub Release with changelog

**Checkpoint**: CI pipeline runs successfully on a test push (all checks green). Visibility query confirms only render/ + surface/ depend on Skia.

---

## Phase 4: Release & Engineering Polish (User Story 4)

**Purpose**: Document release process, CHANGELOG, and spec-kit templates for future feature specifications

- [ ] T022 [US4] Create `native_ui/doc/release-process.md` with SemVer versioning (MAJOR.MINOR.PATCH), CHANGELOG.md convention, shared library publishing steps (`bazel build //src/framework/public:native_ui_shared`), and hotfix branch strategy
- [ ] T023 [US4] Create `CHANGELOG.md` placeholder at workspace root with semantic versioning sections ([Unreleased], MAJOR, MINOR, PATCH) and changelog entry format
- [ ] T024 [P] [US4] Create `spec/native_ui/_template.yaml` with spec-kit YAML template covering interface signature, behavior description, edge cases, and test points
- [ ] T025 [P] [US4] Create `spec/native_ui/_template_layout.md` with alternative markdown template for complex widget specs (narrative format with sequence diagrams)

**Checkpoint**: Release process documented with SemVer; CHANGELOG.md created; spec-kit templates (YAML + Markdown) available for all future feature specifications.

---

## Phase 5: Validation & Consistency

**Purpose**: Verify all 21+ deliverables are consistent, complete, and cross-reference correctly

- [ ] T026 Run consistency review: verify all 8 architecture documents and 5 API contracts have zero contradictions with each other and with `project_bootstrap.md` (SC-005). Check cross-references between error-handling.md ↔ logging-slot.md, threading.md ↔ data-binding.md, lifecycle-model.md ↔ widget-contract.md
- [ ] T027 Verify spec-kit templates support both YAML and Markdown formats (SC-006), and both templates include interface signature + behavior + edge cases + test points sections
- [ ] T028 Verify all 8 framework module stub directories exist with BUILD.bazel files (core, layout, render, surface, viewmodel, widgets, event, public)
- [ ] T029 Add `src/framework/viewmodel/BUILD.bazel` to the root `//:native_ui` alias deps in `native_ui/BUILD.bazel`
- [ ] T030 Final validation: `bazel build //...` + `bazel test //...` both pass after all documentation. Commit and push to trigger CI pipeline smoke test

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (US1)**: No dependencies — can start immediately
- **Phase 2 (US2)**: No dependencies on US1 — contracts can be written independently
- **Phase 3 (US3)**: No dependencies on US1/US2 — CI pipeline independent of architecture docs
- **Phase 4 (US4)**: No dependencies on other phases — templates and release docs are standalone
- **Phase 5 (Validation)**: Depends on all other phases being complete

### Parallel Opportunities

- All tasks within a phase marked [P] can run in parallel
- Phase 1, Phase 2, Phase 3, and Phase 4 can all run in parallel (independent deliverables)
- Phase 5 is the only serial dependency gate

### Implementation Strategy

1. Complete Phase 1 (architecture docs) → **MVP**: architecture is documented
2. Complete Phase 2 (contracts) → **Demo**: implementers can create widgets
3. Complete Phase 3 (CI) → **Gate**: automated quality enforcement active
4. Complete Phase 4 (release/templates) → **Polish**: release process defined
5. Phase 5 (validation) → **Ship**: consistency verified
