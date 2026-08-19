#include "gtest/gtest.h"
#include "src/framework/widgets/container.h"

namespace native::ui {

class NullWidget : public Widget {
public:
  void Draw(Canvas&) override {}
};

TEST(ContainerTest, EmptyContainer) {
  Container c;
  EXPECT_EQ(c.ChildCount(), 0);
  EXPECT_EQ(c.ChildAt(0), nullptr);
  EXPECT_EQ(c.IndexOf(nullptr), -1);
}

TEST(ContainerTest, AddChild) {
  Container c;
  auto child = std::make_unique<NullWidget>();
  child->SetId("child1");
  auto* raw = child.get();
  c.AddChild(std::move(child));
  EXPECT_EQ(c.ChildCount(), 1);
  EXPECT_EQ(c.ChildAt(0), raw);
}

TEST(ContainerTest, AddChildTriggersLayout) {
  Container c;
  EXPECT_FALSE(c.needs_layout());
  c.AddChild(std::make_unique<NullWidget>());
  EXPECT_TRUE(c.needs_layout());
}

TEST(ContainerTest, RemoveChild) {
  Container c;
  auto* raw = new NullWidget();
  raw->SetId("remove_me");
  c.AddChild(std::unique_ptr<NullWidget>(raw));
  EXPECT_EQ(c.ChildCount(), 1);

  c.RemoveChild(raw);
  EXPECT_EQ(c.ChildCount(), 0);
}

TEST(ContainerTest, ClearChildren) {
  Container c;
  c.AddChild(std::make_unique<NullWidget>());
  c.AddChild(std::make_unique<NullWidget>());
  c.AddChild(std::make_unique<NullWidget>());
  EXPECT_EQ(c.ChildCount(), 3);

  c.ClearChildren();
  EXPECT_EQ(c.ChildCount(), 0);
}

TEST(ContainerTest, FindByIdInContainer) {
  Container c;
  auto child = std::make_unique<NullWidget>();
  child->SetId("target");
  auto* raw = child.get();
  c.AddChild(std::move(child));
  EXPECT_EQ(c.FindById("target"), raw);
}

TEST(ContainerTest, IndexOfChild) {
  Container c;
  auto a = std::make_unique<NullWidget>();
  auto b = std::make_unique<NullWidget>();
  auto* raw_b = b.get();
  c.AddChild(std::make_unique<NullWidget>());
  c.AddChild(std::move(b));
  EXPECT_EQ(c.IndexOf(raw_b), 1);
}

TEST(ContainerTest, TaggedConstruction) {
  auto child = std::make_unique<NullWidget>();
  child->SetId("btn");
  Container c(
      Direction{Direction::kRow},
      Padding{16},
      Gap{8},
      Container::Children{}
  );
  EXPECT_EQ(c.ChildCount(), 0);
}

}  // namespace native::ui
