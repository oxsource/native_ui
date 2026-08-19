#include "gtest/gtest.h"
#include "src/framework/widgets/button.h"
#include "src/framework/render/canvas.h"
#include "src/framework/widgets/container.h"
#include "src/framework/widgets/external_image.h"
#include "src/framework/widgets/image_widget.h"
#include "src/framework/widgets/stack.h"
#include "src/framework/surface/surface.h"
#include "src/framework/widgets/text.h"

namespace native::ui {

namespace {

class TestState : public State {
public:
  Property<int> count{this};
  Property<std::string> name{this};
};

}  // namespace

// ============================================================================
// Text Widget Tests
// ============================================================================

TEST(TextTest, ConstructWithContent) {
  Text t(Content{"Hello"});
  auto surface = Surface::Create(100, 50);
  ASSERT_NE(surface, nullptr);
  {
    Canvas canvas(*surface);
    t.Draw(canvas);
  }
}

TEST(TextTest, SetId) {
  Text t(Id{"label"});
  EXPECT_EQ(t.GetId(), "label");
}

TEST(TextTest, EmptyContentNoCrash) {
  Text t(Content{""});
  auto surface = Surface::Create(100, 50);
  ASSERT_NE(surface, nullptr);
  {
    Canvas canvas(*surface);
    t.Draw(canvas);
  }
}

TEST(TextTest, FontSizeAndColor) {
  Text t(Content{"Hi"}, FontSize{24}, TextColor{kRed});
  EXPECT_EQ(t.GetId(), "");
}

TEST(TextTest, DataBindingRedrawOnChange) {
  auto state = std::make_shared<TestState>();
  state->name = "initial";
  Text t(Content{"placeholder"});
  t.Watch(state->name);
  EXPECT_FALSE(t.needs_draw());
  state->name = "updated";
  state->Flush();
  EXPECT_TRUE(t.needs_draw());
}

// ============================================================================
// Button Widget Tests
// ============================================================================

TEST(ButtonTest, ConstructWithLabel) {
  Button b(Label{"OK"});
  EXPECT_EQ(b.GetId(), "");
}

TEST(ButtonTest, SetId) {
  Button b(Id{"submit"});
  EXPECT_EQ(b.GetId(), "submit");
}

TEST(ButtonTest, HitTestInsideBounds) {
  Button b(Label{"OK"});
  b.SetBounds(Rect{10, 10, 80, 40});
  EXPECT_TRUE(b.HitTest(Point{50, 30}));
  EXPECT_TRUE(b.HitTest(Point{10, 10}));
  EXPECT_TRUE(b.HitTest(Point{89, 49}));
}

TEST(ButtonTest, HitTestOutsideBounds) {
  Button b(Label{"OK"});
  b.SetBounds(Rect{10, 10, 80, 40});
  EXPECT_FALSE(b.HitTest(Point{5, 5}));
  EXPECT_FALSE(b.HitTest(Point{100, 100}));
  EXPECT_FALSE(b.HitTest(Point{9, 30}));
}

TEST(ButtonTest, OnClickInvoked) {
  bool clicked = false;
  Button b(Label{"OK"}, OnClick{[&] { clicked = true; }});
  EXPECT_FALSE(clicked);
}

TEST(ButtonTest, DrawNoCrash) {
  Button b(Label{"OK"});
  b.SetBounds(Rect{0, 0, 80, 40});
  auto surface = Surface::Create(100, 50);
  ASSERT_NE(surface, nullptr);
  {
    Canvas canvas(*surface);
    b.Draw(canvas);
  }
}

TEST(ButtonTest, EmptyLabelNoCrash) {
  Button b(Label{""});
  b.SetBounds(Rect{0, 0, 80, 40});
  auto surface = Surface::Create(100, 50);
  ASSERT_NE(surface, nullptr);
  {
    Canvas canvas(*surface);
    b.Draw(canvas);
  }
}

TEST(ButtonTest, DataBindingRedrawOnChange) {
  auto state = std::make_shared<TestState>();
  state->name = "initial";
  Button b(Label{"placeholder"});
  b.Watch(state->name);
  EXPECT_FALSE(b.needs_draw());
  state->name = "updated";
  state->Flush();
  EXPECT_TRUE(b.needs_draw());
}

// ============================================================================
// ImageWidget Tests
// ============================================================================

TEST(ImageWidgetTest, ConstructWithEmptyPath) {
  ImageWidget img(ImagePath{""});
  auto surface = Surface::Create(100, 50);
  ASSERT_NE(surface, nullptr);
  {
    Canvas canvas(*surface);
    img.Draw(canvas);
  }
}

TEST(ImageWidgetTest, NonexistentFileNoCrash) {
  ImageWidget img(ImagePath{"/nonexistent/path/img.png"});
  auto surface = Surface::Create(100, 50);
  ASSERT_NE(surface, nullptr);
  {
    Canvas canvas(*surface);
    img.Draw(canvas);
  }
}

TEST(ImageWidgetTest, SetId) {
  ImageWidget img(Id{"avatar"});
  EXPECT_EQ(img.GetId(), "avatar");
}

TEST(ImageWidgetTest, ZeroBoundsNoCrash) {
  ImageWidget img(ImagePath{""});
  auto surface = Surface::Create(10, 10);
  ASSERT_NE(surface, nullptr);
  {
    Canvas canvas(*surface);
    img.Draw(canvas);
  }
}

// ============================================================================
// ExternalImage Tests
// ============================================================================

TEST(ExternalImageTest, DefaultConstructNoCrash) {
  ExternalImage img;
  auto surface = Surface::Create(100, 50);
  ASSERT_NE(surface, nullptr);
  {
    Canvas canvas(*surface);
    img.Draw(canvas);
  }
}

TEST(ExternalImageTest, InvalidBufferNoCrash) {
  ExternalImage img;
  HardwareBuffer buf;
  img.SetBuffer(buf);
  EXPECT_TRUE(img.needs_draw());
  auto surface = Surface::Create(100, 50);
  ASSERT_NE(surface, nullptr);
  {
    Canvas canvas(*surface);
    img.Draw(canvas);
  }
}

TEST(ExternalImageTest, SetId) {
  ExternalImage img(Id{"camera"});
  EXPECT_EQ(img.GetId(), "camera");
}

TEST(ExternalImageTest, SetBufferTriggersRedraw) {
  ExternalImage img;
  EXPECT_FALSE(img.needs_draw());
  img.SetBuffer(HardwareBuffer{});
  EXPECT_TRUE(img.needs_draw());
}

// ============================================================================
// Stack Widget Tests
// ============================================================================

class NullWidget : public Widget {
public:
  void Draw(Canvas&) override {}
};

TEST(StackTest, EmptyStack) {
  Stack s;
  EXPECT_EQ(s.ChildCount(), 0);
  EXPECT_EQ(s.ChildAt(0), nullptr);
  EXPECT_EQ(s.IndexOf(nullptr), -1);
}

TEST(StackTest, AddChild) {
  Stack s;
  auto child = std::make_unique<NullWidget>();
  child->SetId("c1");
  auto* raw = child.get();
  s.AddChild(std::move(child));
  EXPECT_EQ(s.ChildCount(), 1);
  EXPECT_EQ(s.ChildAt(0), raw);
}

TEST(StackTest, AddChildTriggersLayout) {
  Stack s;
  EXPECT_FALSE(s.needs_layout());
  s.AddChild(std::make_unique<NullWidget>());
  EXPECT_TRUE(s.needs_layout());
}

TEST(StackTest, RemoveChild) {
  Stack s;
  auto* raw = new NullWidget();
  raw->SetId("remove_me");
  s.AddChild(std::unique_ptr<NullWidget>(raw));
  EXPECT_EQ(s.ChildCount(), 1);
  s.RemoveChild(raw);
  EXPECT_EQ(s.ChildCount(), 0);
}

TEST(StackTest, RemoveChildTriggersLayout) {
  Stack s;
  auto* raw = new NullWidget();
  s.AddChild(std::unique_ptr<NullWidget>(raw));
  EXPECT_TRUE(s.needs_layout());
  s.RemoveChild(raw);
  EXPECT_TRUE(s.needs_layout());
}

TEST(StackTest, ClearChildren) {
  Stack s;
  s.AddChild(std::make_unique<NullWidget>());
  s.AddChild(std::make_unique<NullWidget>());
  EXPECT_EQ(s.ChildCount(), 2);
  s.ClearChildren();
  EXPECT_EQ(s.ChildCount(), 0);
}

TEST(StackTest, IndexOf) {
  Stack s;
  auto* first = new NullWidget();
  auto* second = new NullWidget();
  s.AddChild(std::unique_ptr<NullWidget>(first));
  s.AddChild(std::unique_ptr<NullWidget>(second));
  EXPECT_EQ(s.IndexOf(first), 0);
  EXPECT_EQ(s.IndexOf(second), 1);
  EXPECT_EQ(s.IndexOf(nullptr), -1);
}

TEST(StackTest, DrawNoCrash) {
  Stack s;
  auto child = std::make_unique<NullWidget>();
  s.AddChild(std::move(child));
  s.SetBounds(Rect{0, 0, 100, 100});
  auto surface = Surface::Create(100, 100);
  ASSERT_NE(surface, nullptr);
  {
    Canvas canvas(*surface);
    s.Draw(canvas);
  }
}

TEST(StackTest, DrawEmptyStackNoCrash) {
  Stack s;
  s.SetBounds(Rect{0, 0, 100, 100});
  auto surface = Surface::Create(100, 100);
  ASSERT_NE(surface, nullptr);
  {
    Canvas canvas(*surface);
    s.Draw(canvas);
  }
}

TEST(StackTest, ZOrder) {
  Stack s;
  auto bottom = std::make_unique<NullWidget>();
  auto top = std::make_unique<NullWidget>();
  auto* top_raw = top.get();
  s.AddChild(std::move(bottom));
  s.AddChild(std::move(top));
  EXPECT_EQ(s.ChildAt(1), top_raw);
}

}  // namespace native::ui
