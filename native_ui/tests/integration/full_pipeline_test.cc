#include "gtest/gtest.h"
#include "canvas.h"
#include "container.h"
#include "paint.h"
#include "surface.h"
#include "text.h"

namespace native::ui {

// Verify the full rendering pipeline: Container → FlexLayout → Canvas → Surface
TEST(FullPipelineTest, ContainerLayoutAndRender) {
  std::vector<std::unique_ptr<Widget>> children;
  children.push_back(std::make_unique<Text>(Content{"Hello"}, Id{"t1"}));
  children.push_back(std::make_unique<Text>(Content{"World"}, Id{"t2"}));

  Container tree(
      Direction{Direction::kRow},
      Padding{16},
      Gap{8},
      Container::Children{std::move(children)});

  tree.Measure({400, 200});
  tree.Arrange({400, 200});

  ASSERT_GE(tree.ChildCount(), 2);
  EXPECT_GT(tree.ChildAt(0)->bounds().width, 0);
  EXPECT_GT(tree.ChildAt(0)->bounds().height, 0);

  auto surface = Surface::Create(400, 200);
  ASSERT_NE(surface, nullptr);

  {
    Canvas canvas(*surface);
    tree.Draw(canvas);
  }
  surface->Flush();
}

// Verify surface dimensions after render
TEST(FullPipelineTest, SurfaceDimensions) {
  std::vector<std::unique_ptr<Widget>> children;
  children.push_back(std::make_unique<Text>(Content{"Test"}, Id{"test"}));

  Container tree(
      Direction{Direction::kColumn},
      Padding{8},
      Container::Children{std::move(children)});

  tree.Measure({100, 100});
  tree.Arrange({100, 100});

  auto surface = Surface::Create(100, 100);
  ASSERT_NE(surface, nullptr);

  {
    Canvas canvas(*surface);
    tree.Draw(canvas);
  }
  surface->Flush();

  EXPECT_EQ(surface->width(), 100);
  EXPECT_EQ(surface->height(), 100);
}

}  // namespace native::ui
