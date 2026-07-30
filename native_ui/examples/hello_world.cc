#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include "button.h"
#include "canvas.h"
#include "container.h"
#include "edge_insets.h"
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
  auto w = static_cast<int>(root->style().width());
  auto h = static_cast<int>(root->style().height());
  if (w <= 0 || h <= 0) return;
  auto surface = ui::Surface::Create(w, h);
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

  // ── Styles ──
  auto countSty = ui::Style()
    .setFontSize(48)
    .setTextColor(ui::Color{uint8_t{60}, uint8_t{60}, uint8_t{200}, uint8_t{255}})
    .setFontWeight(700);

  auto btnSty = ui::Style()
    .setWidth(200).setHeight(50)
    .setCornerRadius(25)
    .setFontSize(18)
    .setFontWeight(600)
    .setNormalColor(ui::Color{uint8_t{60}, uint8_t{60}, uint8_t{200}, uint8_t{255}})
    .setPressedColor(ui::Color{uint8_t{40}, uint8_t{40}, uint8_t{140}, uint8_t{255}})
    .setTextColor(ui::kWhite);

  // ── Widgets ──
  auto label = std::make_unique<ui::Text>(
      ui::Content{"0"}, countSty, ui::Id{"label"});
  auto* raw = label.get();

  std::vector<std::unique_ptr<ui::Widget>> v;
  v.push_back(std::move(label));
  v.push_back(std::make_unique<ui::Button>(ui::Label{"Increment"}, btnSty, ui::Id{"btn"}));

  auto tree = std::make_unique<ui::Container>(
      ui::Direction{ui::Direction::kColumn},
      ui::Gap{16},
      ui::Width{300}, ui::Height{240},
      ui::Padding{ui::EdgeInsets::All(24)},
      ui::Container::Children{std::move(v)});

  raw->Watch(state->count);

  for (int i = 0; i <= 2; i++) {
    state->count = std::to_string(i);
    state->Flush();
    char p[64];
    std::snprintf(p, sizeof(p), "/tmp/frame_%03d.png", i);
    RenderAndSave(tree.get(), p);
    printf("Generated: %s\n", p);
  }
  return 0;
}
