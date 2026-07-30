#include "gtest/gtest.h"
#include "canvas.h"
#include "container.h"
#include "paint.h"
#include "surface.h"
#include "text.h"

namespace native::ui {

// Verify the hello_world pipeline pattern: widget tree → layout → render
TEST(ExamplesTest, WidgetTreeRender) {
  std::vector<std::unique_ptr<Widget>> children;
  children.push_back(std::make_unique<Text>(Content{"Count: 0"}, Id{"label"}));
  children.push_back(std::make_unique<Text>(Content{"Button"}, Id{"btn"}));

  Container tree(
      Direction{Direction::kRow},
      Padding{16},
      Gap{8},
      Container::Children{std::move(children)});

  tree.Measure({800, 600});
  tree.Arrange({800, 600});

  auto surface = Surface::Create(800, 600);
  ASSERT_NE(surface, nullptr);

  {
    Canvas canvas(*surface);
    tree.Draw(canvas);
  }
  surface->Flush();
}

}  // namespace native::ui
