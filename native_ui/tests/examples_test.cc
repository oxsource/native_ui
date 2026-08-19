#include "gtest/gtest.h"
#include "src/framework/render/canvas.h"
#include "src/framework/widgets/container.h"
#include "src/framework/core/edge_insets.h"
#include "src/framework/render/paint.h"
#include "src/framework/surface/surface.h"
#include "src/framework/widgets/text.h"

namespace native::ui {

TEST(ExamplesTest, WidgetTreeRender) {
  std::vector<std::unique_ptr<Widget>> children;
  children.push_back(std::make_unique<Text>(Content{"Count: 0"}, Id{"label"}));
  children.push_back(std::make_unique<Text>(Content{"Button"}, Id{"btn"}));

  Container tree(
      Direction{Direction::kRow},
      Gap{8},
      Width{400},
      Height{200},
      Padding{EdgeInsets::All(16)},
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
}

}  // namespace native::ui
