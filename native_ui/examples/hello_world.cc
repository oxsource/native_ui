#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include "button.h"
#include "canvas.h"
#include "container.h"
#include "paint.h"
#include "png_writer.h"
#include "state.h"
#include "surface.h"
#include "text.h"

namespace ui = native::ui;

class CounterState : public ui::State {
public:
  ui::Property<std::string> count{this};
};

static void RenderAndSave(ui::Container* root, const char* path) {
  root->Layout();
  auto surface = ui::Surface::Create(
      root->layout_size().width, root->layout_size().height);
  if (!surface) return;
  {
    ui::Canvas canvas(*surface);
    root->Draw(canvas);
  }
  surface->Flush();
  native::ui::PngWriter::Write(surface->sk_surface(), path);
}

int main() {
  auto state = std::make_shared<CounterState>();

  auto label = std::make_unique<ui::Text>(ui::Content{"Count: 0"}, ui::Id{"label"});
  auto* label_raw = label.get();

  std::vector<std::unique_ptr<ui::Widget>> children;
  children.push_back(std::move(label));
  children.push_back(std::make_unique<ui::Button>(ui::Label{"Increment"}, ui::Id{"btn"}));

  auto tree = std::make_unique<ui::Container>(
      ui::Direction{ui::Direction::kRow},
      ui::Padding{16},
      ui::Gap{8},
      ui::Size{800, 600},
      ui::Container::Children{std::move(children)});

  label_raw->Watch(state->count);

  for (int i = 0; i <= 2; i++) {
    state->count = "Count: " + std::to_string(i);
    state->Flush();
    char path[64];
    std::snprintf(path, sizeof(path), "/tmp/frame_%03d.png", i);
    RenderAndSave(tree.get(), path);
    printf("Generated: %s\n", path);
  }
  return 0;
}
