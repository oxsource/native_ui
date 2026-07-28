# Native UI - Project Bootstrap

> Version: 0.1
>
> Status: Draft
>
> Purpose: Initial project definition for AI-assisted specification-driven development.

---

# 1. Vision (Why)

## 1.1 Project Vision

Native UI 是一套基于 Skia 的原生声明式渲染框架（Native Declarative Rendering Framework）。

Skia 提供了强大的 2D 图形渲染能力，但其 API 偏底层，缺少 UI 开发所需的布局系统、控件抽象和声明式描述能力。Native UI 的目标是在 Skia 之上构建一**套简化的、支持 Layout 的声明式 UI 框架**，提供更易用的 API，用于快速开发原生 UI 界面。

### Design Philosophy

- **Declarative**：通过声明式描述 UI 结构，而非命令式逐像素绘制
- **Flexbox Layout**：采用 W3C Flexbox 规范子集作为布局引擎，简洁统一
- **Skia-Native**：直接构建于 Skia Canvas 之上，无额外中间层
- **Cross-Platform**：一期覆盖 macOS ARM64 / Linux x86_64，后续可扩展

---

## 1.2 Goals

- 提供声明式 Widget Tree 描述能力
- 提供基于 Flexbox 的 Layout 引擎
- 提供 Skia 渲染封装层，简化 Canvas / Paint / Path 等底层操作
- 提供基础 Widget 控件库（Text, Button, Image, Container, etc.）
- 提供事件处理机制（输入事件、事件冒泡、状态管理）
- 作为独立基础库，供其他 Bazel 项目依赖使用
- 保持模块化设计，便于持续扩展

---

## 1.3 Non-Goals (Phase 1)

- 可视化编辑器 / UI Designer
- 动画系统（Phase 2+）
- 国际化 / RTL 布局（Phase 2+）
- 复杂表单控件（Table, Tree, ListView 等 Phase 2+）
- GPU 渲染管线优化（Phase 2+）
- 嵌入式平台支持（Phase 2+）

---

# 2. Requirements (What)

## 2.1 Functional Requirements

### FR-001 Declarative Widget Tree

UI 结构通过声明式方式描述，支持嵌套组合：

```cpp
auto root = Container(
    Direction(kRow),
    Padding(16),
    Children{
        Text(Content("Hello")),
        Button(Label("Click"))
    }
);
```

### FR-002 Flexbox Layout Engine

采用 W3C Flexbox 规范作为布局模型，支持：

- flex-direction: row / column
- justify-content / align-items / align-self
- flex-wrap
- flex-grow / flex-shrink / flex-basis
- gap, padding, margin

### FR-003 Skia Render Wrapper

提供 Skia Canvas 的易用封装：

- `Canvas` — 自动 save/restore 的 scope 式画布
- `Paint` — 链式调用的画刷
- `Path` — 简化的路径构造
- `TextLayout` — 文本排版

### FR-004 Basic Widgets

- Container（布局容器）
- Text（文本标签）
- Button（按钮）
- Image（图片）
- Stack（层叠布局）

### FR-005 Event Handling

- Hit testing 机制
- 事件冒泡 / 捕获
- 鼠标 / 触摸输入适配

### FR-006 Public Library

Native UI 应作为 Bazel Library 对外提供：

```python
deps = ["@native_ui//src/framework/public:native_ui"]
```

### FR-007 Platform Buffer Support

框架必须提供对平台原生 Buffer 的高效绘制支持，通过专用控件简化 Skia 的 `SkSurface` 创建和管理。支持的 Buffer 类型：

- **AHardwareBuffer (Android)**：零拷贝共享 GPU/NPU 缓冲区，用于相机预览、视频解码、AI 推理结果叠加
- **IOSurface (macOS/iOS)**：跨进程/跨 framework 的共享像素缓冲区，用于 Metal/OpenGL 互操作
- **DMA-BUF (Linux)**：内核级缓冲区共享，用于嵌入式显示流水线

框架应提供：

- `PlatformSurface` 控件 — 接收外部 Buffer 描述符（AHardwareBuffer / IOSurface / DMA-BUF fd），自动创建 Skia `SkSurface` 并管理生命周期
- Buffer 到达回调 — 外部生产者在 Buffer 就绪时通知 Widget 更新
- 高效的 texture 导入路径 — 避免 CPU 回读，直接在 GPU 侧完成合成

### FR-008 Example

项目必须提供一个完整示例，验证声明式 Widget 创建、Layout 测量和 Skia 渲染全流程。

---

## 2.2 Non-Functional Requirements

- **轻量**：核心库体积可控，不引入不必要的依赖
- **模块化**：core/layout/render/widgets/event 各模块独立，接口清晰
- **易扩展**：Widget 可自定义组合，Layout 可扩展
- **易测试**：核心逻辑有单元测试覆盖，渲染层可 mock
- **跨平台**：架构层面预留跨平台支持

---

# 3. Design (How)

## 3.1 Overall Architecture

```
Application Code (declarative UI description)
    │
    ▼
Widget Tree Construction
    │
    ▼
Layout Engine (measure → arrange, Flexbox)
    │
    ▼
Render Tree (Skia paint)
    │
    ▼
Skia Canvas → Pixel Output
```

## 3.2 Core Modules

### Module Overview

```
native_ui/src/framework/
│
├── core/          # 基础类型：Rect, Point, Size, Color, Matrix, EdgeInsets
├── layout/        # 布局引擎：Flexbox measure + arrange
├── render/        # Skia 渲染封装：Canvas, Paint, Path, TextLayout
├── surface/       # 平台 Buffer 封装：PlatformSurface, BufferHandle, SurfaceFactory
├── widgets/       # 基础控件：Text, Button, Image, Container, Stack, PlatformSurface
├── event/         # 事件处理：Event, HitTester, InputHandler
└── public/        # 公开 API 汇总入口
```

### Module Responsibility

| Module | Responsibility | External Deps |
|--------|---------------|---------------|
| `core` | 几何类型、颜色、矩阵运算、EdgeInsets | — |
| `layout` | Flexbox measure/arrange 算法 | caflex |
| `render` | Skia Canvas 封装、Paint、Path、Text | Skia |
| `surface` | 平台 Buffer 封装：PlatformSurface, BufferHandle, SurfaceFactory | Skia, platform headers |
| `widgets` | 基础控件、Widget 基类、组合规则 | core, layout, render, event, surface |
| `event` | Event 类型定义、HitTesting、分发机制 | core |
| `public` | Umbrella header、export macro、汇总 target | 所有模块 |

### Naming Scope

C++ namespace `native::ui`，其下按模块分二层 namespace：

```cpp
namespace native {
namespace ui {

// Core types
class Rect;
class Color;

// Layout
class FlexLayout;

// Widgets
class Widget;
class Text;

}  // namespace ui
}  // namespace ui
```

## 3.3 Directory Structure

```
native_ui/                        # Bazel workspace root
│
├── doc/
│   └── project_bootstrap.md      # This file
│
├── platforms/
│   ├── BUILD                     # config_setting + platform 定义
│   └── platforms.bzl             # config_setting_and_platform 宏
│
├── third_party/
│   ├── skia/
│   │   └── BUILD.bazel           # Skia cc_library wrapper
│   └── caflex/
│       └── BUILD.bazel           # caflex cc_library wrapper
│
├── src/
│   └── framework/
│       ├── core/
│       │   ├── BUILD.bazel
│       │   ├── rect.h / rect.cc
│       │   ├── point.h / point.cc
│       │   ├── size.h / size.cc
│       │   ├── color.h / color.cc
│       │   └── edge_insets.h / edge_insets.cc
│       ├── layout/
│       │   ├── BUILD.bazel
│       │   ├── flex_layout.h / flex_layout.cc
│       │   └── layout_result.h
│       ├── render/
│       │   ├── BUILD.bazel
│       │   ├── canvas.h / canvas.cc
│       │   ├── paint.h / paint.cc
│       │   ├── path.h / path.cc
│       │   └── text_layout.h / text_layout.cc
│       ├── surface/
│       │   ├── BUILD.bazel
│       │   ├── platform_surface.h / platform_surface.cc
│       │   ├── buffer_handle.h
│       │   └── surface_factory.h / surface_factory.cc
│       ├── widgets/
│       │   ├── BUILD.bazel
│       │   ├── widget.h / widget.cc
│       │   ├── container.h / container.cc
│       │   ├── text.h / text.cc
│       │   ├── button.h / button.cc
│       │   └── image.h / image.cc
│       ├── event/
│       │   ├── BUILD.bazel
│       │   ├── event.h / event.cc
│       │   └── hit_tester.h / hit_tester.cc
│       └── public/
│           ├── BUILD.bazel
│           └── include/
│               └── native_ui/
│                   ├── native_ui.h           # Umbrella header
│                   ├── native_ui_export.h    # NATIVE_UI_API macro
│               ├── core.h
│               ├── layout.h
│               ├── render.h
│               ├── surface.h
│               ├── widgets.h
│               └── event.h
│
├── examples/
│   ├── BUILD.bazel
│   └── hello_world.cc              # MVP demo
│
├── tests/
│   ├── BUILD.bazel
│   ├── core_test.cc
│   ├── layout_test.cc
│   ├── render_test.cc
│   └── widgets_test.cc
│
├── BUILD.bazel                     # Root alias
├── WORKSPACE                       # workspace(name = "native_ui")
├── native_ui_deps.bzl              # External dependency bootstrap
├── .bazelversion                   # 6.5.0
├── .bazelrc
├── .bazelignore
└── .gitignore

spec/                               # spec-kit 规格文件（与 workspace 同级）
├── native_ui/
│   ├── core.yaml
│   ├── flex_layout.yaml
│   ├── render_canvas.yaml
│   ├── widgets_text.yaml
│   └── ...
```

## 3.4 Widget Model

### Widget Tree

Widget 是 UI 的基本构建单元。每个 Widget 通过 Layout 阶段确定尺寸和位置，通过 Render 阶段绘制到 Skia Canvas。

```
Widget (Abstract Base)
│
├── Leaf Widget (Text, Image, Button)
│   └── No children, implements Draw()
│
└── Container Widget (Container, Stack)
    └── Has children, manages layout + draw
```

### Widget Base Class

Widget 基类提供 ID 存储、子节点遍历、布局失效标记等通用能力：

```cpp
class Widget {
public:
    virtual ~Widget() = default;

    // ID
    void SetId(std::string id) { id_ = std::move(id); }
    const std::string& GetId() const { return id_; }

    // Tree navigation
    Widget* FindById(const std::string& id);
    virtual Widget* ChildAt(int index) { return nullptr; }
    virtual int ChildCount() const { return 0; }
    virtual int IndexOf(Widget* child) const { return -1; }

    // Layout invalidation
    void RequestLayout();
    void RequestRedraw();

    // Lifecycle
    virtual void Draw(Canvas&) = 0;

private:
    std::string id_;
};
```

`FindById` 使用 DFS 遍历子树：

```cpp
Widget* Widget::FindById(const std::string& id) {
    if (id_ == id) return this;
    for (int i = 0; i < ChildCount(); ++i) {
        auto* found = ChildAt(i)->FindById(id);
        if (found) return found;
    }
    return nullptr;
}
```

### Tagged Parameter Constructor

所有 Widget 统一使用 Tagged Parameter 模式构造——构造函数接受任意顺序的强类型标签参数：

```cpp
class Container : public Widget {
public:
    template <typename... Args>
    explicit Container(Args&&... args) {
        (ProcessArg(std::forward<Args>(args)), *this);
    }

private:
    void ProcessArg(Direction tag);
    void ProcessArg(Padding tag);
    void ProcessArg(Children tag);
    void ProcessArg(Id tag);
};
```

标签类型为轻量值类型，零开销：

```cpp
struct Direction { FlexDirection value; };
struct Padding   { float value; };
struct Children  { std::vector<std::unique_ptr<Widget>> value; };
struct Content   { std::string value; };
struct Label     { std::string value; };
struct Id        { std::string value; };
```

### Lifecycle

```
Create → Mount → Measure → Arrange → Draw → Unmount
         ↓
      [Dynamic Update] ← insert / remove / modify
         ↓
      RequestLayout() → Re-Measure → Re-Arrange → Draw
         ↓
      RequestRedraw() → Draw (same layout)
```

### Dynamic Tree Manipulation

Widget 树在构造完成后支持运行时动态调整，支持插入、移除、替换子节点。

### Core API

```cpp
class Container : public Widget {
public:
    // -- Dynamic children manipulation --

    // Insert a widget at a specific index
    void InsertChild(int index, std::unique_ptr<Widget> child);

    // Append a widget to the end of children list
    void AddChild(std::unique_ptr<Widget> child);

    // Remove a child by pointer or index
    void RemoveChild(Widget* child);
    void RemoveChildAt(int index);

    // Replace a child at index with a new widget
    void ReplaceChild(int index, std::unique_ptr<Widget> new_child);

    // Remove all children
    void ClearChildren();

    // -- Layout invalidation --

    // Mark this subtree as needing re-layout (re-measure + re-arrange)
    void RequestLayout();

    // Mark this subtree as needing re-draw (same layout, visual change)
    void RequestRedraw();

protected:
    // Called when a child is inserted/removed/replaced.
    // Subclasses can override to update internal state.
    virtual void OnChildAdded(Widget* child);
    virtual void OnChildRemoved(Widget* child);
};
```

### Trigger Mechanism

动态操作自动触发布局更新流程：

```
AddChild(child)
  └→ child.Mount()
  └→ OnChildAdded(child)
  └→ RequestLayout()
       └→ Measure()     // re-measure subtree
       └→ Arrange()     // re-arrange subtree
       └→ Draw()        // re-render
```

### Usage Example

```cpp
auto list = Container(Direction(kColumn));

// Initially empty, add items dynamically
for (int i = 0; i < 10; ++i) {
    list.AddChild(Text(Content("Item " + std::to_string(i))));
}

// Later: remove the third item
list.RemoveChildAt(2);

// Replace first item
list.ReplaceChild(0, Button(Label("Updated")));

// Clear all
list.ClearChildren();
```

### State Invalidation Principles

| Operation        | Invalidation       | Effect                               |
|-----------------|---------------------|---------------------------------------|
| Insert / Remove | RequestLayout()    | Full measure + arrange + redraw       |
| Modify content  | RequestRedraw()    | Re-draw only (layout unchanged)       |
| Modify style    | RequestLayout()    | May affect size → re-measure          |
| Modify visibility | RequestLayout()  | Show/hide affects sibling positions   |

## 3.5 Flexbox Layout

采用 **caflex** 作为 Flexbox 底层实现引擎，在其之上封装 `native::ui::FlexLayout`。

### FlexLayout API

```cpp
class FlexLayout {
public:
    template <typename... Args>
    explicit FlexLayout(Args&&... args) {
        (ProcessArg(std::forward<Args>(args)), *this);
    }

    // Measure: given parent constraints, return child desired sizes
    std::vector<MeasureResult> Measure(Size available_size);

    // Arrange: given measured sizes and positions, place children
    void Arrange(const std::vector<MeasureResult>& measured);

private:
    void ProcessArg(Direction tag);
    void ProcessArg(JustifyContent tag);
    void ProcessArg(AlignItems tag);
    void ProcessArg(FlexWrap tag);
};
```

对应标签类型：

```cpp
struct Direction     { FlexDirection value; };
struct JustifyContent { JustifyContent::Type value; };
struct AlignItems    { AlignItems::Type value; };
struct FlexWrap      { FlexWrap::Type value; };
```

## 3.6 Tagged Parameter Pattern

### Design Motivation

摒弃传统链式 Builder 或 Setter/Getter 模式，采用 **强类型标签参数（Tagged Parameter）** 构造所有 Widget／Layout 对象。原因：

- **声明式直觉**：`Container(Direction(kRow), Padding(16))` 直接描述"是什么"，而非"怎么建"
- **任意顺序**：标签参数不依赖固定位置，用户可按偏好排列
- **类型安全**：编译期检查，`Direction(kRow)` 不会意外赋值给 `Padding`
- **零开销**：标签类型是空壳值类型，编译器优化后与直接成员赋值等价
- **单层嵌套**：`Children{ }` 初始化列表自然表达层级关系

### Mechanism

核心思路：每个可配置属性定义一个独立标签类型，Widget 构造函数使用变参模板 + fold expression 按类型分发：

```cpp
// Tag types (header-only, zero-cost over direct assignment)
struct Direction     { FlexDirection value; };
struct Padding       { float value; };
struct Children      { std::vector<WidgetPtr> value; };
struct Content       { std::string value; };
struct Label         { std::string value; };
struct Gap           { float value; };

// Widget base
class Widget {
public:
    virtual ~Widget() = default;
    virtual void Draw(Canvas&) = 0;
};

// Concrete widget using tagged parameter constructor
class Container : public Widget {
public:
    template <typename... Args>
    explicit Container(Args&&... args) {
        (ProcessArg(std::forward<Args>(args)), *this);
    }

    void Draw(Canvas& canvas) override { /* ... */ }

private:
    void ProcessArg(Direction tag) {
        layout_.SetDirection(tag.value);
    }
    void ProcessArg(Padding tag) {
        layout_.SetPadding(tag.value);
    }
    void ProcessArg(Children tag) {
        children_ = std::move(tag.value);
    }

    std::vector<std::unique_ptr<Widget>> children_;
    FlexLayout layout_;
};
```

Fold expression `(ProcessArg(std::forward<Args>(args)), *this);` 展开为逗号表达式序列，实现 C++17 兼容的单趟分发。

### Tag Naming Convention

| Category     | Tag Type        | Value Type          | Example                            |
|--------------|-----------------|---------------------|------------------------------------|
| Layout       | `Direction`     | `FlexDirection`     | `Direction(kRow)`                  |
| Layout       | `Padding`       | `float`             | `Padding(16.0f)`                   |
| Layout       | `Gap`           | `float`             | `Gap(8.0f)`                        |
| Layout       | `JustifyContent`| `JustifyContent`    | `JustifyContent(kCenter)`          |
| Widget       | `Content`       | `std::string`       | `Content("Hello")`                 |
| Widget       | `Label`         | `std::string`       | `Label("Click")`                   |
| Widget       | `Children`      | `WidgetList`        | `Children{ Text(...), Button(...) }`|
| Widget       | `ImagePath`     | `std::string`       | `ImagePath("/path/to/img.png")`    |
| Widget       | `Id`            | `std::string`       | `Id("submit_btn")`                 |

### Usage Examples

```cpp
// Flexbox row with two children
auto row = Container(
    Direction(kRow),
    Gap(8),
    Padding(12),
    Children{
        Text(Content("Name:")),
        Text(Content("John Doe"))
    }
);

// Stack with padding
auto card = Container(
    Direction(kColumn),
    Padding(16),
    Children{
        Image(ImagePath("avatar.png")),
        Text(Content("User Name")),
        Button(Id("follow_btn"), Label("Follow"))
    }
);

// Find and update a widget by ID
auto* btn = root->FindById("follow_btn");
if (btn) btn->RequestLayout();
```

## 3.7 Render Layer

### Scoped Canvas

```cpp
// Auto save/restore via RAII
void Paint( SkCanvas* sk_canvas) {
    Canvas canvas(sk_canvas);
    Paint paint;
    paint.SetColor(Color::kRed).SetAntiAlias(true);
    canvas.DrawRect(Rect(0, 0, 100, 100), paint);
}
```

### Draw Pipeline

```
Widget::Draw(Canvas&)
    → Canvas save state
    → Apply widget transform + clip
    → Paint background/border/shadow
    → Draw content (Text/Image/Path)
    → Draw children (recursive)
    → Canvas restore state
```

---

# 4. Build System (Bazel 6.5)

## 4.1 Version Requirement

```
6.5.0
```

## 4.2 .bazelversion

```
6.5.0
```

## 4.3 .bazelrc

参考 `graph_runtime` 的 `.bazelrc` 设计：

```text
build --cxxopt=-std=c++17
build --host_cxxopt=-std=c++17
build --features=visibility=hidden

# Platform aliases
build:macos_arm64 --platforms=//platforms:macos_arm64
build:linux_x86_64 --platforms=//platforms:linux_x86_64

# Default to macOS ARM64 for development
build --platforms=//platforms:macos_arm64

test --test_output=errors
```

## 4.4 WORKSPACE

```python
workspace(name = "native_ui")

load("//:native_ui_deps.bzl", "native_ui_setup")

native_ui_setup()
```

## 4.5 Root BUILD.bazel

```python
package(default_visibility = ["//visibility:public"])

alias(
    name = "native_ui",
    actual = "//src/framework/public:native_ui",
)
```

## 4.6 native_ui_deps.bzl

统一管理所有第三方依赖：

```python
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def _skia():
    http_archive(
        name = "skia",
        sha256 = "d9d5558db7006a0bf0ab61e9428c467e47932f6d260eaaba058686a17b1a203e",
        urls = ["https://github.com/google/skia/archive/abc1234567.tar.gz"],
        strip_prefix = "skia-abc1234567",
        build_file = "//third_party/skia:BUILD.bazel",
    )

def _caflex():
    http_archive(
        name = "caflex",
        urls = ["https://github.com/caiof/caflex/archive/<commit>.tar.gz"],
        sha256 = "<sha256>",
        strip_prefix = "caflex-<commit>",
        build_file = "//third_party/caflex:BUILD.bazel",
    )

def _googletest():
    http_archive(
        name = "com_google_googletest",
        sha256 = "8ad598c73ad796e0d8280b082cebd82a630d73e73cd3c70057938a6501bba5d7",
        strip_prefix = "googletest-1.14.0",
        urls = ["https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz"],
    )

def _bazel_skylib():
    http_archive(
        name = "bazel_skylib",
        urls = ["https://github.com/bazelbuild/bazel-skylib/archive/refs/tags/1.6.1.tar.gz"],
        sha256 = "aede1b60709ac12b3461ee0bb3fa097b58a86fbfdb88ef7e9f90424a69043167",
        strip_prefix = "bazel-skylib-1.6.1",
    )

def native_ui_setup():
    if not native.existing_rule("bazel_skylib"):
        _bazel_skylib()
    if not native.existing_rule("skia"):
        _skia()
    if not native.existing_rule("caflex"):
        _caflex()
    if not native.existing_rule("com_google_googletest"):
        _googletest()
```

## 4.7 Platforms

### platforms/BUILD

```python
load("//platforms:platforms.bzl", "config_setting_and_platform")

config_setting_and_platform(
    name = "macos_arm64",
    constraint_values = [
        "@platforms//os:macos",
        "@platforms//cpu:aarch64",
    ],
)

config_setting_and_platform(
    name = "linux_x86_64",
    constraint_values = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
)
```

### platforms/platforms.bzl

```python
def config_setting_and_platform(name, constraint_values, parents = None):
    native.config_setting(
        name = name + "_setting",
        constraint_values = constraint_values,
    )
    native.platform(
        name = name,
        constraint_values = constraint_values,
        parents = parents,
    )

def native_ui_select(select_map):
    return select(select_map)
```

## 4.8 Third-Party BUILD Files

### third_party/skia/BUILD.bazel

```python
package(default_visibility = ["//visibility:public"])

cc_library(
    name = "skia",
    hdrs = glob(["include/**/*.h"]),
    srcs = glob([
        "src/**/*.cc",
        "src/**/*.cpp",
    ]),
    includes = [
        "include",
        "include/core",
        "include/gpu",
        "include/config",
    ],
    copts = [
        "-Wno-unused-parameter",
        "-Wno-deprecated-declarations",
    ],
    linkopts = select({
        "//platforms:macos_arm64_setting": [
            "-framework ApplicationServices",
            "-framework CoreGraphics",
            "-framework CoreText",
            "-framework Metal",
        ],
        "//conditions:default": [],
    }),
    visibility = ["//visibility:public"],
)
```

### third_party/caflex/BUILD.bazel

```python
package(default_visibility = ["//visibility:public"])

cc_library(
    name = "caflex",
    hdrs = glob(["include/**/*.h", "src/**/*.h"]),
    srcs = glob(["src/**/*.cc", "src/**/*.cpp"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)
```

## 4.9 Internal Module BUILD Rules

```python
# src/framework/core/BUILD.bazel
cc_library(
    name = "core",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.h"]),
    visibility = ["//src/framework:__subpackages__", "//tests:__subpackages__"],
)

# src/framework/layout/BUILD.bazel
cc_library(
    name = "layout",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.h"]),
    deps = [
        "//src/framework/core",
        "@caflex//:caflex",
    ],
    visibility = ["//src/framework:__subpackages__", "//tests:__subpackages__"],
)

# src/framework/widgets/BUILD.bazel
cc_library(
    name = "widgets",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.h"]),
    deps = [
        "//src/framework/core",
        "//src/framework/layout",
        "//src/framework/render",
        "//src/framework/event",
    ],
    visibility = ["//src/framework:__subpackages__", "//tests:__subpackages__"],
)
```

## 4.10 Public API Target

```python
# src/framework/public/BUILD.bazel
cc_library(
    name = "native_ui",
    hdrs = glob(["include/native_ui/*.h"]),
    strip_include_prefix = "include",
    copts = [
        "-fvisibility=hidden",
        "-fvisibility-inlines-hidden",
        "-DNATIVE_UI_SHARED_LIBRARY",
    ],
    alwayslink = 1,
    deps = [
        "//src/framework/core",
        "//src/framework/layout",
        "//src/framework/render",
        "//src/framework/surface",
        "//src/framework/widgets",
        "//src/framework/event",
    ],
    visibility = ["//visibility:public"],
)

# Shared library for non-Bazel consumers
cc_binary(
    name = "native_ui_shared",
    srcs = ["native_ui_init.cc"],
    linkshared = True,
    linkstatic = True,
    copts = [
        "-fvisibility=hidden",
        "-fvisibility-inlines-hidden",
        "-DNATIVE_UI_SHARED_LIBRARY",
    ],
    deps = [":native_ui"],
    visibility = ["//visibility:public"],
)
```

---

# 5. Code Style

## 5.1 Google C++ Style

本项目严格遵守 [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)。

### Naming

| Category            | Style              | Example                        |
|---------------------|--------------------|--------------------------------|
| File names          | `snake_case`       | `flex_layout.cc`               |
| Type/Class          | `PascalCase`       | `class FlexLayout`             |
| Function            | `PascalCase`       | `void Measure(Size)`           |
| Variable            | `snake_case`       | `int child_count`              |
| Member variable     | `snake_case_`      | `int child_count_`             |
| Constant            | `kPascalCase`      | `const int kMaxChildren = 64`  |
| Namespace           | `snake_case`       | `namespace native::ui`         |
| Macro               | `UPPER_SNAKE_CASE` | `NATIVE_UI_API`                |

### Formatting

- Indentation: 2 spaces (no tabs)
- Line length: 80 characters
- Use `nullptr`, not `NULL` or `0`
- Use `auto` sparingly, only when type is obvious
- Include order: related header, C++ standard library, third-party, project headers
- Use `//` for inline comments; `/* */` only for documentation blocks

### Ownership & Pointers

- Prefer `std::unique_ptr` over raw pointers
- Use raw pointers only for non-owning references
- Avoid `std::shared_ptr` unless ownership is truly shared

### Header Conventions

- Use `#pragma once` (no include guards)
- Headers must be self-contained (include all deps)
- `.h` / `.cc` file pairs required for each class
- Unit test files: `*_test.cc`

---

# 6. Commit Convention

## 6.1 Conventional Commits

```
<type>(<scope>): <description>

[optional body]

[optional footer]
```

### Types

| Type       | Usage                                    |
|------------|------------------------------------------|
| `feat`     | New feature                              |
| `fix`      | Bug fix                                  |
| `docs`     | Documentation only                        |
| `style`    | Code style, formatting (no logic change) |
| `refactor` | Code restructuring (no bug fix, no feature) |
| `perf`     | Performance improvement                  |
| `test`     | Adding or fixing tests                   |
| `build`    | Build system (Bazel, etc.)               |
| `ci`       | CI/CD changes                            |
| `chore`    | Maintenance, dependencies, etc.          |

### Scopes

| Scope      | Area                        |
|------------|-----------------------------|
| `core`     | Core types module           |
| `layout`   | Flexbox layout engine       |
| `render`   | Skia render wrapper         |
| `widgets`  | Widget controls             |
| `event`    | Event handling              |
| `public`   | Public API layer            |
| `example`  | Example demo                |
| `build`    | Bazel build files           |
| `docs`     | Documentation               |

### Examples

```
feat(core): add Rect, Point, Size base types
feat(layout): implement Flexbox measure/arrange
feat(render): add scoped Canvas RAII wrapper
feat(widgets): add Text and Button widget
build(bazel): integrate Skia via http_archive
docs: add project bootstrap doc
```

### Rules

- Description must be lowercase, imperative mood, no period
- All text must be pure ASCII (English only)
- Body explains what and why, not how
- Footer may reference issues: `Closes #123`

---

# 7. Public API Export Macro

## 7.1 NATIVE_UI_API

```cpp
// native_ui_export.h
#pragma once

#if defined(_WIN32)
  #if defined(NATIVE_UI_SHARED_LIBRARY)
    #define NATIVE_UI_API __declspec(dllexport)
  #else
    #define NATIVE_UI_API __declspec(dllimport)
  #endif
#else
  #if defined(NATIVE_UI_SHARED_LIBRARY)
    #define NATIVE_UI_API __attribute__((visibility("default")))
  #else
    #define NATIVE_UI_API
  #endif
#endif
```

- 编译单元统一使用 `-fvisibility=hidden`
- 仅 `NATIVE_UI_API` 修饰的符号被导出
- `cc_binary(linkshared=True, linkstatic=True)` 构建共享库时定义 `NATIVE_UI_SHARED_LIBRARY`

## 7.2 Umbrella Header

```cpp
// native_ui.h
#pragma once

#include "native_ui/native_ui_export.h"
#include "native_ui/core.h"
#include "native_ui/layout.h"
#include "native_ui/render.h"
#include "native_ui/surface.h"
#include "native_ui/widgets.h"
#include "native_ui/event.h"
```

外部消费者只需 `#include "native_ui/native_ui.h"`。

---

# 8. Skia Integration

## 8.1 Dependency

Skia 通过 `http_archive` 引入，版本锁定为 commit `abc123`：

```python
http_archive(
    name = "skia",
    sha256 = "d9d5558db7006a0bf0ab61e9428c467e47932f6d260eaaba058686a17b1a203e",
    urls = ["https://github.com/google/skia/archive/abc1234567.tar.gz"],
    strip_prefix = "skia-abc1234567",
    build_file = "//third_party/skia:BUILD.bazel",
)
```

> **Note**: The commit hash `abc1234567` above is a placeholder. Replace with the actual Skia commit hash before use.

## 8.2 Wrapper Layer

`src/framework/render/` 模块封装 Skia 底层 API，对外提供 `native_ui` 风格的接口：

- `Canvas` — RAII 封装的 SkCanvas，自动 save/restore
- `Paint` — 链式调用风格的 SkPaint 包装
- `Path` — 简化的路径构造（moveTo / lineTo / cubicTo）
- `TextLayout` — 文本排版布局，基于 SkShaper / SkParagraph

`src/framework/surface/` 模块封装平台原生 Buffer，利用 Skia 的 `SkSurface` 创建能力：

- `PlatformSurface` — 接收 AHardwareBuffer / IOSurface / DMA-BUF 句柄，自动创建 SkSurface
- `BufferHandle` — 跨平台的 Buffer 描述符类型擦除封装
- `SurfaceFactory` — 平台适配工厂，按 `#ifdef` 编译对应后端

项目内部其它模块 **禁止直接依赖 Skia**，必须经由 `render/` 或 `surface/` 模块间接使用。

---

# 9. Agent-Driven Development

## 9.1 Workflow

本项目使用 **opencode + spec-kit** 进行 AI-assisted 开发：

```
┌─────────────────────────────────────────────────────┐
│  1. Human: 需求描述 / Issue                          │
│  2. spec-kit: 规格化 → spec/*.yaml 或 spec/*.md      │
│  3. opencode agent: 读取 spec → 实现代码             │
│  4. agent: 自测 (bazel test //...)                   │
│  5. Human: Code Review                              │
│  6. agent: 根据 review 修改                          │
│  7. 提交 (conventional commit)                       │
└─────────────────────────────────────────────────────┘
```

## 9.2 Spec Files

规格文件位于 `spec/` 顶层目录，命名格式：

```
spec/
├── core.yaml              # Core types spec
├── flex_layout.yaml       # Flexbox layout spec
├── render_canvas.yaml     # Canvas render spec
├── widgets_text.yaml      # Text widget spec
└── ...
```

每个 spec 文件包含：接口签名、行为描述、边界条件、测试要点。

## 9.3 Agent Instructions

Agent 在实现时遵循：

1. 读取对应 `spec/` 文件，理解接口契约
2. 按 Google C++ Style 编写代码（2-space indent, 80-col width, etc.）
3. 每个功能带对应 `*_test.cc` 单元测试
4. 提交前运行 `bazel test //...` 验证
5. Commit message 遵循 Conventional Commits 格式

---

# 10. MVP Deliverables

一期完成后应具备：

- Core Types 库（Rect, Point, Size, Color, EdgeInsets）
- Flexbox Layout 引擎（measure + arrange）
- Skia Render 封装层（Canvas, Paint, TextLayout）
- 基础 Widget 库（Container, Text, Button, Image, Stack）
- 事件处理机制（HitTest, Event dispatch）
- Public API 汇总头文件
- Hello World 示例
- 单元测试覆盖核心逻辑
- 开发者文档

---

# 11. Success Criteria

一期完成时，应满足以下目标：

- 能通过声明式 Widget Tree 描述 UI 结构
- Layout 引擎能正确完成 Flexbox measure + arrange
- Widget 能正确绘制到 Skia Canvas
- 示例程序能完整运行并渲染出 UI
- 可作为独立 Bazel Library 被其他项目依赖
- Public API 清晰、自包含、有文档
- 测试通过率 100%

---

# 12. References

- [Skia Graphics Library](https://skia.org/)
- [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- [Conventional Commits](https://www.conventionalcommits.org/)
- [W3C Flexbox Specification](https://www.w3.org/TR/css-flexbox-1/)
- [caflex - C++ Flexbox Library](https://github.com/caiof/caflex)
- [graph_runtime - Reference Project](${PROJECT_ROOT}/../graph_runtime/graph_runtime)

  本项目 Bazel 配置（WORKSPACE, .bazelrc, platforms, deps.bzl）和
  C++ 代码规范均参考 graph_runtime 的设计。
  参考路径: `/Users/moks/Develop/docker/ubuntu24/codes/graph_runtime/graph_runtime`
