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
  root->Measure({800, 600});
  root->Arrange({800, 600});
  auto surface = ui::Surface::Create(800, 600);
  if (!surface) return;
  {
    ui::Canvas canvas(*surface);
    root->Draw(canvas);
  }
  surface->Flush();
  WriteSkSurfaceToPNG(surface->sk_surface(), path);
}

int main() {
  auto state = std::make_shared<CounterState>();
  state->count = "Count: 0";

  auto label = std::make_unique<ui::Text>(ui::Content{"Count: 0"}, ui::Id{"label"});
  auto* label_raw = label.get();

  std::vector<std::unique_ptr<ui::Widget>> children;
  children.push_back(std::move(label));
  children.push_back(std::make_unique<ui::Button>(ui::Label{"Increment"}, ui::Id{"btn"}));

  auto tree = std::make_unique<ui::Container>(
      ui::Direction{ui::Direction::kRow},
      ui::Padding{16},
      ui::Gap{8},
      ui::Container::Children{std::move(children)});

  label_raw->Watch(state->count);

  // Frame 000: count = 0
  RenderAndSave(tree.get(), "/tmp/frame_000.png");

  // Frame 001: count = 1
  state->count = "Count: 1";
  state->Flush();
  RenderAndSave(tree.get(), "/tmp/frame_001.png");

  // Frame 002: count = 2
  state->count = "Count: 2";
  state->Flush();
  RenderAndSave(tree.get(), "/tmp/frame_002.png");

  printf("Generated: /tmp/frame_000.png, /tmp/frame_001.png, /tmp/frame_002.png\n");
  return 0;
}
