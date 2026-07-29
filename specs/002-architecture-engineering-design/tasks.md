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

**Purpose**: Document module architecture, dependencies, error handling, memory model, lifecycle, and data binding

- [ ] T001 [P] [US1] Create `native_ui/doc/architecture/README.md` with architecture overview and key decisions index
- [ ] T002 [P] [US1] Create `native_ui/doc/architecture/module-dependencies.md` with formal module dependency graph and Bazel visibility rules
- [ ] T003 [P] [US1] Create `native_ui/doc/architecture/error-handling.md` with StatusOr strategy, logging policy, and no-exceptions convention
- [ ] T004 [P] [US1] Create `native_ui/doc/architecture/memory-model.md` with unique_ptr ownership model and raw pointer observation rules
- [ ] T005 [P] [US1] Create `native_ui/doc/architecture/lifecycle-model.md` with Widget lifecycle state machine and mount/unmount semantics
- [ ] T006 [US1] Create `native_ui/doc/architecture/data-binding.md` with React-inspired ViewModel pattern, property notification, Bind/Unbind lifecycle, and batch RequestRedraw strategy
- [ ] T007 [US1] Create `native_ui/doc/architecture/threading.md` with main-thread frame loop (React-style batch model), worker-thread logic processing, ViewModel cross-thread bridge, and scheduling primitives (PostTask, PostNextFrame, ScheduleTimer)
- [ ] T008 [US1] Create `native_ui/doc/architecture/logging-slot.md` with LogSink abstract interface, log levels, structured metadata, and registration API
- [ ] T009 [US1] Add viewmodel module stub at `src/framework/viewmodel/BUILD.bazel` as empty `cc_library`

**Checkpoint**: All 8 architecture design documents are written and internally consistent. A developer can understand the full architecture by reading them.

---

## Phase 2: API & Interface Contracts (User Story 2)

**Purpose**: Define public API contracts for all framework modules

- [ ] T010 [P] [US2] Create `native_ui/doc/api/widget-contract.md` with Widget virtual interface, extension points, and custom widget guide
- [ ] T011 [P] [US2] Create `native_ui/doc/api/layout-contract.md` with FlexLayout interface, Measure/Arrange protocol, and adding new layouts
- [ ] T012 [P] [US2] Create `native_ui/doc/api/render-contract.md` with Canvas/Paint/Path contract and Skia isolation rules
- [ ] T013 [P] [US2] Create `native_ui/doc/api/event-contract.md` with event dispatch protocol, bubble/capture, and adding event types
- [ ] T014 [US2] Create `native_ui/doc/api/viewmodel-contract.md` with ViewModel base class, property notification API, and binding lifecycle

**Checkpoint**: A widget implementer can create a custom widget by following these contracts without consulting the architecture team.

---

## Phase 3: Engineering Standards & CI/CD (User Story 3)

**Purpose**: Establish testing strategy, build conventions, CI pipeline, and agent instructions

- [ ] T015 [P] [US3] Create `native_ui/doc/testing-strategy.md` with unit test structure, mock patterns, integration test scopes, golden test plan, and coverage targets
- [ ] T016 [P] [US3] Create `native_ui/doc/build-conventions.md` with BUILD file conventions, dep prefix rules, and visibility templates
- [ ] T017 [P] [US3] Create `native_ui/doc/agent-instructions.md` with standard prompt template for opencode agents working on native_ui
- [ ] T018 [P] [US3] Create `native_ui/doc/ci-strategy.md` with CI architecture doc — which checks run when, caching strategy, matrix platforms
- [ ] T019 [US3] Create `.github/workflows/ci.yml` with CI pipeline: `bazel build //...`, `bazel test //...`, clang-format, clang-tidy, Skia isolation query
- [ ] T020 [US3] Create `.github/workflows/pr.yml` with PR gate: same checks as CI + mandatory review approval
- [ ] T021 [US3] Create `.github/workflows/release.yml` with release workflow: tag → build shared lib → create GitHub Release

**Checkpoint**: CI pipeline runs successfully on a test push (all checks green). Visibility query validates Skia isolation.

---

## Phase 4: Release & Engineering Polish (User Story 4)

**Purpose**: Document release process and spec-kit templates

- [ ] T022 [US4] Create `native_ui/doc/release-process.md` with versioning (SemVer), changelog convention, and shared library publishing steps
- [ ] T023 [US4] Create `CHANGELOG.md` placeholder at workspace root
- [ ] T024 [P] [US4] Create `spec/native_ui/_template.yaml` with spec-kit YAML template (interface, behavior, bounds, tests)
- [ ] T025 [P] [US4] Create `spec/native_ui/_template_layout.md` with alternative markdown template for complex widget specs

**Checkpoint**: Release process documented; spec-kit templates available for all future feature specifications.

---

## Phase 5: Validation & Consistency

**Purpose**: Verify all deliverables are consistent and complete

- [ ] T026 Run consistency review: verify all architecture documents have zero contradictions with each other and with `project_bootstrap.md` (SC-005)
- [ ] T027 Verify spec-kit templates support both YAML and Markdown formats (SC-006)
- [ ] T028 Verify all framework module stub directories exist with BUILD.bazel files
- [ ] T029 Add `src/framework/viewmodel/BUILD.bazel` to the root `BUILD.bazel` alias deps
- [ ] T030 Final validation: commit and push to trigger CI pipeline smoke test

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
