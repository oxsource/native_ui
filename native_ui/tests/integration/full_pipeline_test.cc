#include "gtest/gtest.h"
#include "canvas.h"
#include "container.h"
#include "edge_insets.h"
#include "paint.h"
#include "surface.h"
#include "text.h"

namespace native::ui {

TEST(FullPipelineTest, ContainerLayoutAndRender) {
  std::vector<std::unique_ptr<Widget>> children;
  children.push_back(std::make_unique<Text>(Content{"Hello"}, Id{"t1"}));
  children.push_back(std::make_unique<Text>(Content{"World"}, Id{"t2"}));

  Container tree(
      Direction{Direction::kRow},
      Width{400}, Height{200},
      Gap{8},
      Padding{EdgeInsets::All(16)},
      Container::Children{std::move(children)});

  tree.Layout();

  ASSERT_GE(tree.ChildCount(), 2);
  EXPECT_GT(tree.ChildAt(0)->bounds().width, 0);
  EXPECT_GT(tree.ChildAt(0)->bounds().height, 0);

  auto surface = Surface::Create(
      static_cast<int>(tree.style().width()),
      static_cast<int>(tree.style().height()));
  ASSERT_NE(surface, nullptr);

  {
    Canvas canvas(*surface);
    tree.Draw(canvas);
  }
  surface->Flush();
}

TEST(FullPipelineTest, SurfaceDimensions) {
  std::vector<std::unique_ptr<Widget>> children;
  children.push_back(std::make_unique<Text>(Content{"Test"}, Id{"test"}));

  Container tree(
      Direction{Direction::kColumn},
      Width{100}, Height{100},
      Padding{EdgeInsets::All(8)},
      Container::Children{std::move(children)});

  tree.Layout();

  auto surface = Surface::Create(
      static_cast<int>(tree.style().width()),
      static_cast<int>(tree.style().height()));
  ASSERT_NE(surface, nullptr);

  {
    Canvas canvas(*surface);
    tree.Draw(canvas);
  }
  surface->Flush();

  EXPECT_GT(surface->width(), 0);
  EXPECT_GT(surface->height(), 0);
}

}  // namespace native::ui
