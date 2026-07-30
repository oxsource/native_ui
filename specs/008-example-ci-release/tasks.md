---

description: "Task list for Phase 8: Example, CI Polish & Release"

---

# Tasks: Example, CI Polish & Release

**Input**: Design documents from `specs/008-example-ci-release/`

**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md, contracts/

**Tests**: Test tasks are included per the spec's acceptance criteria.

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

All paths relative to `native_ui/` under the repo root, except `.github/workflows/` at the repo root.

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Create build targets and directory structure for the example and tests

- [ ] T001 Create `demo/BUILD.bazel` with `cc_binary` for `hello_world` depending on `//src/framework/public:native_ui` and `//src/framework/render` (for Skia PNG encoding)
- [ ] T002 [P] Create `scripts/` directory with `scripts/build_shared.sh`

---

## Phase 2: User Story 1 - Hello World Example (Priority: P1) 🎯 MVP

**Goal**: End-to-end Hello World demonstrating full MVP pipeline: State data binding → widget tree → FlexLayout → Canvas render → PNG output → event handling → redraw.

**Independent Test**: `bazel run //demo:hello_world` exits with code 0 and produces `hello_world_output.png` with rendered widgets.

- [ ] T003 [P] [US1] Create `demo/hello_world.cc` with CounterState, widget tree (Container + Text + Button with OnClick), Watch binding, Measure/Arrange, Canvas draw to Surface, Flush
- [ ] T004 [US1] Add PNG encoding to `demo/hello_world.cc` — use Skia `surface->image()->encodeToData()` to write PNG to `hello_world_output.png`
- [ ] T005 [US1] Add click simulation to `demo/hello_world.cc` — simulate State update (count++), re-Measure/Arrange/Draw, write second frame PNG

**Checkpoint**: Hello World example builds and runs, producing valid PNG output with rendered widgets.

---

## Phase 3: User Story 2 - CI Pipeline (Priority: P1)

**Goal**: CI already configured from P2 (ci.yml with matrix build, test, formatting, Skia isolation; pr.yml with review gate). Verify it works and add status badge.

**Independent Test**: CI workflows in `.github/workflows/` exist and pass, badge shows green.

- [x] T006 [P] [US2] `.github/workflows/ci.yml` already exists — matrix: [macos-14, ubuntu-22.04], bazel build //..., bazel test //..., Skia isolation query, disk cache
- [x] T007 [P] [US2] `.github/workflows/pr.yml` already exists — PR trigger, same checks, review approval gate
- [x] T008 [US2] `.github/workflows/release.yml` already exists — tag trigger `v*`, build shared lib, GitHub Release
- [ ] T009 [US2] Verify CI workflows reference `//demo:hello_world` in test steps (add if missing)
- [ ] T010 [US2] Create `README.md` at repo root with project description and `![CI](.../workflows/CI/badge.svg)` status badge

**Checkpoint**: CI pipeline verified, badge visible in README.

---

## Phase 4: User Story 3 - Release Engineering (Priority: P2)

**Goal**: Shared library build target, build script, CHANGELOG, and full-pipeline integration test.

**Independent Test**: `bash scripts/build_shared.sh` produces `dist/libnative_ui_shared.dylib` (or `.so`).

- [ ] T011 [P] [US3] Add `native_ui_shared` target to `src/framework/public/BUILD.bazel` — `cc_binary(name = "native_ui_shared", linkshared = True, deps = [":native_ui"])`
- [ ] T012 [US3] Create `scripts/build_shared.sh` — `bazel build //src/framework/public:native_ui_shared && mkdir -p dist && cp bazel-bin/src/framework/public/libnative_ui_shared.* dist/`
- [ ] T013 [US3] Update `CHANGELOG.md` with MVP v0.1.0 entry — list additions across all 8 phases
- [ ] T014 [US3] Create `tests/examples_test.cc` — smoke test: run `//demo:hello_world`, verify exit code 0 and output PNG file exists
- [ ] T015 [US3] Create `tests/integration/full_pipeline_test.cc` — programmatic Container → FlexLayout → Canvas draw → Surface flush → pixel readback verification
- [ ] T016 [US3] Update `tests/examples_test.cc` and add targets: update `tests/BUILD.bazel` with `examples_test` cc_test, update `tests/integration/BUILD.bazel` with `full_pipeline_test` cc_test

**Checkpoint**: Shared library builds, CHANGELOG documents MVP, tests verify full pipeline.

---

## Phase 5: Verification

**Purpose**: Final verification that all targets build and tests pass

- [ ] T017 Verify full build: `bazel build //...`
- [ ] T018 Verify all tests: `bazel test //...`
- [ ] T019 Run Hello World: `bazel run //demo:hello_world && ls -la hello_world_output.png`

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies
- **US1 Hello World (Phase 2)**: Depends on Setup
- **US2 CI (Phase 3)**: Independent — CI already exists from P2
- **US3 Release (Phase 4)**: Depends on Setup + US1 shared lib target
- **Verification (Phase 5)**: Depends on all phases

### Parallel Opportunities

- T001+T002 (Setup) — [P], different paths
- T003+T004+T005 (US1) — sequential within story
- T009+T010 (US2) — [P], independent files
- T011+T012+T013+T014+T015+T016 (US3) — T011/T015 are [P]
- US1, US2, US3 can work in parallel

---

## Implementation Strategy

### MVP (Hello World)

1. Complete Phase 1: Setup
2. Complete Phase 2: Hello World example
3. Validate: `bazel run //demo:hello_world && ls hello_world_output.png`

### Incremental Delivery

1. Hello World runs and produces PNG
2. CI already done — verify badge
3. Shared library + CHANGELOG + integration tests

---

## Notes

- `.github/workflows/` files exist from P2 — only verify and add badge
- `examples/` is in `.bazelignore` (contains external_dep_test with own WORKSPACE) — use `demo/` instead
- Hello World needs `//src/framework/render` dep for Skia `encodeToData()` access
- `tests/integration/BUILD.bazel` already exists — update it, don't recreate
- `examples_test.cc` is a separate smoke test; `full_pipeline_test.cc` is programmatic
