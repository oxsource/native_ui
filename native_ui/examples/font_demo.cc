#include <cstdio>
#include <memory>
#include <string>

#include "src/framework/widgets/container.h"
#include "src/framework/core/edge_insets.h"
#include "src/framework/render/canvas.h"
#include "src/framework/render/font_manager.h"
#include "src/framework/render/paint.h"
#include "src/framework/surface/surface.h"
#include "src/framework/widgets/text.h"

namespace ui = native::ui;

int main(int argc, char** argv) {
  // Font file path: argv[1] (device path when run under android-font-demo),
  // defaulting to a conventional Android shell path.
  std::string font_path = argc > 1 ? argv[1] : "/data/local/tmp/fonts/roboto.ttf";

  // Register the font by file path (FR-001); first registration is the default.
  if (!ui::FontManager::Default().RegisterFont("demo", font_path, 400)) {
    std::fprintf(stderr, "failed to register font %s: %s\n",
                 font_path.c_str(),
                 ui::FontManager::Default().last_error().c_str());
    return 1;
  }

  // A text widget referencing the registered family, plus a default-family one.
  auto text = std::make_unique<ui::Text>(
      ui::Content{"External font: Roboto"},
      ui::FontFamily{"demo"}, ui::FontWeight{700}, ui::FontSize{32},
      ui::TextColor{ui::Color{220, 60, 40}},
      ui::TextAlign{ui::TextAlign::kCenter});

  std::vector<std::unique_ptr<ui::Widget>> children;
  children.push_back(std::move(text));

  auto container = std::make_unique<ui::Container>(
      ui::Direction{ui::Direction::kColumn},
      ui::Width{480}, ui::Height{180},
      ui::Background{ui::Color{250, 250, 255}},
      ui::Padding{ui::EdgeInsets::All(24)},
      ui::Container::Children{std::move(children)});

  container->Layout();
  int w = static_cast<int>(container->style().width());
  int h = static_cast<int>(container->style().height());
  if (w <= 0 || h <= 0) {
    std::fprintf(stderr, "invalid demo surface size %dx%d\n", w, h);
    return 1;
  }

  auto surface = ui::Surface::Create(w, h);
  if (!surface) {
    std::fprintf(stderr, "failed to create surface %dx%d\n", w, h);
    return 1;
  }
  {
    ui::Canvas canvas(*surface);
    container->Draw(canvas);
  }
  surface->Flush();

  const char* out = argc > 2 ? argv[2] : "/data/local/tmp/font_demo.png";
  if (!surface->Dump(out)) {
    std::fprintf(stderr, "failed to dump PNG: %s\n", out);
    return 1;
  }
  std::printf("font demo rendered with registered font to %s\n", out);
  return 0;
}