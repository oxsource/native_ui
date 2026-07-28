---

description: "Task list for Project Scaffolding & Skia Spike"

---

# Tasks: Project Scaffolding & Skia Spike

**Input**: Design documents from `specs/001-project-scaffold/`

**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/public-api.md, quickstart.md

**Tests**: Tests are NOT requested in this feature specification (P1 is build system only; application tests come in later phases).

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

All paths are relative to the Bazel workspace root (`native_ui/`).

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Initialize Bazel workspace and build system configuration

- [x] T001 Create workspace root `native_ui/` directory structure with all module subdirectories
- [x] T002 Create `.bazelversion` with content `6.5.0` at workspace root
- [x] T003 Create `.bazelignore` to ignore example workspaces at workspace root
- [x] T004 Create `.bazelrc` with C++17 standard, hidden visibility, platform aliases, and test config at workspace root

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Platform definitions, dependency management, and third-party wrappers that ALL user stories depend on

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [x] T005 [P] Create `WORKSPACE` with `workspace(name = "native_ui")` and `native_ui_setup()` call at workspace root
- [x] T006 [P] Create `native_ui_deps.bzl` with `native_ui_setup()` function declaring Skia, caflex, googletest, and bazel_skylib dependencies
- [x] T007 [P] Create `platforms/platforms.bzl` with `config_setting_and_platform` helper macro
- [x] T008 [P] Create `platforms/BUILD` with `config_setting_and_platform` targets for `macos_arm64` and `linux_x86_64`
- [x] T009 Create `third_party/skia/BUILD.bazel` as `cc_library` wrapper for Skia with platform-specific linkopts
- [x] T010 Create `third_party/caflex/BUILD.bazel` as `cc_library` wrapper for caflex

**Checkpoint**: Foundation ready — user story implementation can now begin in parallel

---

## Phase 3: User Story 1 - Developer Builds Project from Source (Priority: P1) 🎯 MVP

**Goal**: A developer can clone the repo and build the entire project with a single command (`bazel build //...`), producing all expected module targets.

**Independent Test**: Run `bazel build //...` from a clean clone — all targets compile successfully with zero errors.

### Implementation for User Story 1

- [x] T011 [P] [US1] Create root `BUILD.bazel` with `alias` target `//:native_ui` pointing to `//src/framework/public:native_ui`
- [x] T012 [P] [US1] Create `src/framework/core/BUILD.bazel` as empty `cc_library` target
- [x] T013 [P] [US1] Create `src/framework/layout/BUILD.bazel` as empty `cc_library` target
- [x] T014 [P] [US1] Create `src/framework/render/BUILD.bazel` as empty `cc_library` target
- [x] T015 [P] [US1] Create `src/framework/surface/BUILD.bazel` as empty `cc_library` target
- [x] T016 [P] [US1] Create `src/framework/widgets/BUILD.bazel` as empty `cc_library` target
- [x] T017 [P] [US1] Create `src/framework/event/BUILD.bazel` as empty `cc_library` target
- [x] T018 [US1] Create `src/framework/public/BUILD.bazel` as umbrella `cc_library` aggregating all module targets
- [x] T019 [US1] Create `src/framework/public/include/native_ui/native_ui_export.h` with `NATIVE_UI_API` visibility macro

**Checkpoint**: `bazel build //...` succeeds. All module stubs compile. Root alias resolves correctly.

---

## Phase 4: User Story 3 - Skia Spike Validates Rendering Pipeline (Priority: P1)

**Goal**: A developer builds and runs the Skia spike binary to confirm Skia integrates correctly — compiles, links, draws to a surface, and produces a valid PNG.

**Independent Test**: Run `bazel run //src/spike:skia_spike` — it executes and produces a valid PNG image file on disk.

### Implementation for User Story 3

- [ ] T020 [US3] Create `src/spike/skia_spike.cc` with minimal Skia surface creation, canvas draw (red rectangle), and PNG encode via `SkPngEncoder`
- [ ] T021 [US3] Create `src/spike/BUILD.bazel` as `cc_binary` depending on `@skia//:skia`

**Checkpoint**: `bazel run //src/spike:skia_spike` produces a valid PNG with the expected drawn content.

---

## Phase 5: User Story 2 - Integrator Depends on Native UI Library (Priority: P1)

**Goal**: An external Bazel project declares a dependency on `@native_ui//:native_ui` and successfully compiles and links a binary that includes the umbrella header.

**Independent Test**: Create a minimal external Bazel project with `deps = ["@native_ui//:native_ui"]` — `bazel build` resolves dependency and links successfully.

### Implementation for User Story 2

- [ ] T022 [US2] Verify public `cc_library` target in `src/framework/public/BUILD.bazel` has `strip_include_prefix = "include"` and correct `deps` aggregation of all modules
- [ ] T023 [US2] Verify public target is `//visibility:public` and root alias `//:native_ui` resolves correctly
- [ ] T024 [US2] Validate external dependency by creating a temporary test project that `#include "native_ui/native_ui_export.h"` and links against `@native_ui//:native_ui`

**Checkpoint**: An external Bazel project can depend on native_ui and compile/link a binary without errors.

---

## Phase 6: User Story 4 - Developer Runs All Tests (Priority: P2)

**Goal**: A developer runs `bazel test //...` and the test infrastructure discovers and executes all tests (initially empty), reporting zero failures.

**Independent Test**: Run `bazel test //...` — test discovery works and empty test suite passes.

### Implementation for User Story 4

- [ ] T025 [P] [US4] Create `tests/BUILD.bazel` with minimal test infrastructure (empty test suite or placeholder)
- [ ] T026 [US4] Verify `bazel test //...` discovers test targets and passes with zero failures

**Checkpoint**: Test pipeline is operational — `bazel test //...` passes.

---

## Phase 7: Polish & Cross-Cutting Concerns

**Purpose**: Build conventions documentation and validation

- [ ] T027 [P] Create `spec/native_ui/build.yaml` with build convention spec for agents
- [ ] T028 Run full validation: `bazel build //...`, `bazel test //...`, verify Skia isolation with `bazel query`

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies — can start immediately
- **Foundational (Phase 2)**: Depends on Setup completion — BLOCKS all user stories
- **User Story 1 (Phase 3)**: Depends on Foundational completion — creates module stubs and public API
- **User Story 3 (Phase 4)**: Depends on Foundational completion — spike binary can proceed independently of module stubs
- **User Story 2 (Phase 5)**: Depends on User Story 1 (public API target must exist)
- **User Story 4 (Phase 6)**: Depends on Foundational completion — test infra independent of source code
- **Polish (Phase 7)**: Depends on all user stories being complete

### User Story Dependencies

- **User Story 1 (P1)**: Can start after Foundational — No dependencies on other stories
- **User Story 3 (P1)**: Can start after Foundational — No dependencies on other stories (parallel with US1)
- **User Story 2 (P1)**: Depends on US1 — public API target must exist
- **User Story 4 (P2)**: Can start after Foundational — No dependencies on other stories

### Within Each User Story

- Foundational infrastructure (platforms, deps) before module-specific work
- BUILD files before source files
- Empty targets before content expansion
- For US3: binary source before BUILD wrapper for spike
- For US2: public target validation before external dependency verification

### Parallel Opportunities

- All Setup tasks T001-T004 can run sequentially (each builds on previous)
- Foundational tasks T005-T008 (marked [P]) can run in parallel
- Foundational tasks T009-T010 (third_party wrappers) can run in parallel
- All module stub tasks T012-T017 (marked [P]) can run in parallel
- US1 (Phase 3) and US3 (Phase 4) can run in parallel after Foundational completes
- US4 (Phase 6) can run in parallel with US1 and US3
- Polish tasks T027-T028 can run in parallel

---

## Parallel Example: Phase 2 Foundational

```bash
# Launch all independent foundational tasks together:
Task: "T005 Create WORKSPACE"
Task: "T006 Create native_ui_deps.bzl"
Task: "T007 Create platforms/platforms.bzl"
Task: "T008 Create platforms/BUILD"

# After T005-T008 complete, launch third-party wrappers:
Task: "T009 Create third_party/skia/BUILD.bazel"
Task: "T010 Create third_party/caflex/BUILD.bazel"
```

---

## Parallel Example: User Stories 1 & 3 (Concurrent)

```bash
# US1: All module stub BUILD files (parallel):
Task: "T012 core/BUILD.bazel"
Task: "T013 layout/BUILD.bazel"
Task: "T014 render/BUILD.bazel"
Task: "T015 surface/BUILD.bazel"
Task: "T016 widgets/BUILD.bazel"
Task: "T017 event/BUILD.bazel"

# US3: Skia spike (parallel with US1 stubs):
Task: "T020 Create skia_spike.cc"
Task: "T021 Create spike/BUILD.bazel"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup
2. Complete Phase 2: Foundational (CRITICAL — blocks all stories)
3. Complete Phase 3: User Story 1 (module stubs + public API)
4. **STOP and VALIDATE**: `bazel build //...` must succeed
5. Deploy/demo if ready

### Incremental Delivery

1. Complete Setup + Foundational → Foundation ready
2. Add User Story 1 (module stubs) → `bazel build //...` succeeds → Demo (MVP!)
3. Add User Story 3 (Skia spike) → Spike binary runs → De-risk complete
4. Add User Story 2 (integrator) → External dependency validated
5. Add User Story 4 (tests) → Test pipeline green
6. Polish → Build conventions documented

### Parallel Team Strategy

With multiple developers:

1. Team completes Setup + Foundational together
2. Once Foundational is done:
   - Developer A: User Story 1 (module stubs + public API)
   - Developer B: User Story 3 (Skia spike)
   - Developer C: User Story 4 (test infrastructure)
3. Developer A continues to User Story 2 (integrator) after US1 completes
4. All stories integrate independently

---

## Notes

- [P] tasks = different files, no dependencies
- [Story] label maps task to specific user story for traceability
- Each user story is independently completable and testable
- No test tasks are generated (feature spec does not request tests for P1)
- Commit after each task or logical group
- Stop at any checkpoint to validate story independently
- Avoid: vague tasks, same file conflicts, cross-story dependencies that break independence
