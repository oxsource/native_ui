# Feature Specification: Architecture & Engineering Design

**Feature Branch**: `002-architecture-engineering-design`

**Created**: 2026-07-29

**Status**: Draft

**Input**: User description: "Architecture & Engineering Design"

## User Scenarios & Testing

### User Story 1 - Developer Understands Module Architecture (Priority: P1)

A developer reads the architecture design records to understand how the system is organised — what modules exist, how they depend on each other, how errors propagate, and how memory is managed.

**Why this priority**: Clear architecture documentation is the foundation for all subsequent implementation phases. Without it, developers cannot make consistent design decisions across modules.

**Independent Test**: A developer can answer specific questions about module dependencies, error handling strategy, and memory ownership model by reading the architecture documents alone, without consulting the original author.

**Acceptance Scenarios**:

1. **Given** the architecture documents are published, **When** a developer reads the module dependency diagram, **Then** they can identify which modules depend on Skia and which must not
2. **Given** the error handling document, **When** a developer reads it, **Then** they understand whether exceptions or StatusOr is used and where
3. **Given** the memory model document, **When** a developer reads it, **Then** they understand Widget ownership rules and pointer conventions
4. **Given** the lifecycle document, **When** a developer reads it, **Then** they understand the mount/unmount lifecycle state machine
5. **Given** the data binding document, **When** a developer reads it, **Then** they understand the State pattern, property notification, and how to watch a State from a Widget
6. **Given** the threading model document, **When** a developer reads it, **Then** they understand which work runs on the main thread (rendering, layout, event dispatch) and which runs on worker threads (logic, data processing)
7. **Given** the logging slot document, **When** a developer reads it, **Then** they know how to plug in a custom log implementation

---

### User Story 2 - Widget Implementer Follows Interface Contracts (Priority: P1)

A widget implementer reads the API & interface contracts to understand how to create a new custom widget, what virtual methods to override, and how the layout/measure protocol works.

**Why this priority**: The widget interface contract defines the extension point for all future widget development. Getting this contract right early prevents costly refactoring later.

**Independent Test**: A widget implementer can create a new custom widget by following the contract documentation alone, correctly implementing all required methods and protocols.

**Acceptance Scenarios**:

1. **Given** the widget contract document, **When** an implementer creates a custom widget, **Then** they know which methods to override and what each does
2. **Given** the layout contract, **When** an implementer integrates a new layout, **Then** they know the measure/arrange protocol
3. **Given** the render contract, **When** an implementer draws content, **Then** they understand Skia isolation rules and which APIs are available
4. **Given** the event contract, **When** an implementer handles input, **Then** they understand the event dispatch protocol (bubble/capture)

---

### User Story 3 - CI Pipeline Validates Builds Automatically (Priority: P2)

A developer pushes code and the CI pipeline automatically builds all targets, runs all tests, checks formatting, and performs linting — reporting results without manual intervention.

**Why this priority**: CI automation ensures code quality and prevents regressions as the project grows. Setting it up early avoids the accumulation of undetected issues.

**Independent Test**: A pull request triggers the CI pipeline, which executes all checks and reports a pass or fail status within a reasonable time, without manual intervention.

**Acceptance Scenarios**:

1. **Given** a pull request is opened, **When** the CI pipeline triggers, **Then** it builds all targets and reports success or failure
2. **Given** the CI pipeline runs, **When** formatting is incorrect, **Then** the pipeline fails with a formatting error message
3. **Given** the CI pipeline runs, **When** all checks pass, **Then** the pull request shows a green status

---

### User Story 4 - Release Process Is Documented and Repeatable (Priority: P3)

A release manager follows the documented release process to create a new version of the native_ui library, producing a shared library artifact and publishing it with release notes.

**Why this priority**: A documented release process ensures that releases are consistent, repeatable, and auditable — critical for external consumers who depend on native_ui.

**Independent Test**: A release manager can produce a versioned shared library artifact by following the release documentation, with all steps automated or clearly described.

**Acceptance Scenarios**:

1. **Given** the release documentation, **When** a release manager follows the steps, **Then** they can produce a versioned shared library
2. **Given** a new version is released, **When** the release workflow runs, **Then** a git tag is created and release notes are published
3. **Given** the CHANGELOG exists, **When** a release is made, **Then** changes are documented in the CHANGELOG

---

### Edge Cases

- What happens when the CI pipeline fails due to infrastructure issues (network, runner availability)?
- How does a developer bypass CI for emergency fixes (documented override process)?
- What happens when a new module is added that needs Skia — does the visibility query detect it?
- How does the release process handle hotfixes on a previous release branch?
- What happens when a State is destroyed while widgets are still watching it?
- How does the system handle rapid property changes — does it batch RequestRedraw calls?
- What happens when a worker thread updates a State property while the main thread is rendering?
- How does the framework prevent main thread starvation when worker threads flood property updates?
- What happens when no LogSlot is plugged in — does the framework silently drop logs, or fall back to stderr?
- What happens when PostTask or PostNextFrame callbacks throw exceptions?
- What happens when ScheduleTimer fires while a heavy rendering frame is in progress?

## Requirements

### Functional Requirements

- **FR-001**: Architecture design records must document module dependencies, error handling strategy, memory ownership model, and widget lifecycle state machine
- **FR-002**: API & interface contracts must define the widget virtual interface, layout measure/arrange protocol, Canvas/Paint/Path render contract, and event dispatch protocol
- **FR-003**: Engineering standards must document testing strategy (unit, integration, golden), BUILD file conventions with visibility rules, and agent instruction templates
- **FR-004**: CI pipeline must automatically build all targets, run all tests, check code formatting, and perform linting on every push or pull request
- **FR-005**: CI pipeline must verify that no module outside `render/` or `surface/` depends on Skia directly (visibility query)
- **FR-006**: Release process must be documented with versioning scheme (SemVer), changelog convention, and shared library publishing steps
- **FR-007**: Spec-kit templates must provide YAML and markdown templates for widget/API specifications
- **FR-008**: All design documents must be consistent with each other and with the project bootstrap document — no contradictions
- **FR-009**: No C++ source code is written in this phase — all deliverables are design documents and CI configuration
- **FR-010**: Architecture must define a React-inspired State pattern for data binding — `State` base class with typed `Property<T>` members, property change notification via `Property<T>::operator=`, watching lifecycle (`Watch(Property<T>&)`), automatic batch RequestRedraw, and extension hooks (`OnBeforeSet`, `OnAfterSet`) for value interception
- **FR-011**: Architecture must define the threading model — rendering, layout, and event dispatch run on the main thread; business logic and data processing execute on worker threads. Thread safety boundaries must be documented for State property updates and inter-thread communication. State property changes must be automatically batched within a single frame (React-style), coalescing multiple mutations into one layout + render pass. Three scheduling primitives must be defined: PostTask (pre-render), PostNextFrame (post-render), ScheduleTimer (delayed).
- **FR-012**: Architecture must define a logging slot interface (abstract `LogSlot` base) that the framework calls but does not implement — the consumer provides the concrete log implementation. The interface must support log levels (debug, info, warn, error) and structured metadata.

### Key Entities

- **Architecture Decision Record (ADR)**: A document capturing a significant architectural decision, its context, rationale, and consequences
- **Interface Contract**: A formal specification of a module's public API, including method signatures, preconditions, postconditions, and side effects
- **Engineering Standard**: A documented convention or rule that all code must follow, enforced by code review or automated checks
- **CI Pipeline**: An automated workflow triggered by code changes that builds, tests, and validates the project
- **Release Process**: A documented sequence of steps to produce and publish a new version of the library
- **Spec-kit Template**: A reusable markdown or YAML template for writing feature specifications
- **State**: A data holder object that stores widget state and notifies watching widgets when properties change, following React's state management pattern (unidirectional data flow: props ↓, events ↑)
- **Watch**: A connection between a State property and a Widget — when the State property changes, the watching Widget automatically triggers RequestRedraw
- **LogSlot**: An abstract interface for logging — defines `Log(level, message, metadata)` method; framework calls it but does not provide implementation; consumer plugs in the concrete logger
- **Main Thread**: The thread responsible for rendering (Skia draw), layout calculation, and event dispatch — must never be blocked by business logic
- **Worker Thread**: A background thread for business logic and data processing — communicates results to the main thread via State property updates

## Success Criteria

### Measurable Outcomes

- **SC-001**: A developer can create a new custom widget by following the widget contract document in under 30 minutes
- **SC-002**: CI pipeline completes all checks in under 10 minutes on a standard CI runner
- **SC-003**: A release manager can produce a versioned shared library artifact in under 15 minutes following the release documentation
- **SC-004**: The Skia isolation query (`bazel query`) correctly identifies any module outside `render/` or `surface/` that depends on Skia
- **SC-005**: All architecture documents pass an internal consistency review with zero contradictions against `project_bootstrap.md`
- **SC-006**: Spec-kit templates support at least two format options (YAML and Markdown) for widget specifications
- **SC-007**: A widget implementer can watch a State from a Widget and see automatic redraw on property change by following the data binding documentation

## Clarifications

### Session 2026-07-29

- Q: 是否考虑了页面后续数据状态管理，比如MVVM的架构 → A: 参考 React 单向数据流模式（props ↓, events ↑），框架定义 `State` 基类 + 属性变更通知 + 自动 RequestRedraw，不引入 Vue 式响应式系统避免复杂化
- Q: 还需要考虑一些工程化相关的架构要素 → A: 渲染在主线程（布局、事件分发也在主线程），逻辑处理在工作线程。日志模块预留 LogSlot 抽象接口，框架调用但不实现，由外部提供具体实现
- Q: 是否需要 nextTick/setTimeout 机制 → A: 不需要独立 nextTick。参考 React 批处理模型，State 属性变更自动在帧末合并为一次渲染；PostTask (pre-render) + PostNextFrame (post-render) + ScheduleTimer (跨帧定时) 三个原语即可覆盖所有调度场景

## Assumptions

- The project uses GitHub for source hosting and GitHub Actions for CI/CD
- Developers and release managers have access to the GitHub repository with appropriate permissions
- The primary CI platform is Linux x86_64; macOS ARM64 builds are validated but not required for CI pass
- The release process targets shared library distribution (`.dylib` for macOS, `.so` for Linux)
- Spec-kit templates are written in Markdown for maximum compatibility
- All design documents are written in English (project language)
- Architecture documents may reference graph_runtime as a reference project where conventions align
- Data binding adopts React-inspired pattern (`State` with property notification, unidirectional flow) — no Vue-style computed/reactivity system
- Rendering, layout, and event dispatch execute on the main thread; business logic runs on worker threads. State changes are auto-batched per frame (React-style), no explicit nextTick needed
- Logging uses a slot interface pattern — framework defines `LogSlot` abstract base, consumer provides implementation; no built-in logger
