# Developer Quickstart: Flexbox Layout Engine

## Build & Test

```bash
bazel build //src/framework/layout
bazel test //tests:layout_test
```

## Usage

```cpp
using namespace native::ui;

FlexLayout layout(
    Direction(kRow),
    Gap(8),
    Padding(12)
);

YGNodeRef child1 = YGNodeNew();
YGNodeStyleSetWidth(child1, 60);
YGNodeStyleSetHeight(child1, 40);

YGNodeRef child2 = YGNodeNew();
YGNodeStyleSetWidth(child2, 80);
YGNodeStyleSetHeight(child2, 40);

layout.SetChildren({child1, child2});
auto result = layout.Measure({300, 100});
layout.Arrange(result, {300, 100});

// result[0].position = (12, 12), result[0].size = (60, 40)
// result[1].position = (80, 12), result[1].size = (80, 40)

YGNodeFree(child1);
YGNodeFree(child2);
```
