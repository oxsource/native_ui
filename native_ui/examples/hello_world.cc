#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include "button.h"
#include "canvas.h"
#include "container.h"
#include "edge_insets.h"
#include "paint.h"

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
  surface->Dump(path);
}

int main() {
  auto state = std::make_shared<CounterState>();

  auto label = std::make_unique<ui::Text>(
      ui::Content{"0"},
      ui::Style().setFontSize(64).setFontWeight(700)
         .setTextColor(ui::Color{50, 60, 80}),
      ui::TextAlign{ui::TextAlign::kCenter},
      ui::Id{"label"});
  auto* raw = label.get();

  auto btn = std::make_unique<ui::Button>(
      ui::Label{"Increment"},
      ui::Style().setCornerRadius(22)
         .setFontSize(16).setFontWeight(600)
         .setNormalColor(ui::Color{50, 130, 210})
         .setPressedColor(ui::Color{30, 90, 170})
         .setTextColor(ui::kWhite),
      ui::Id{"btn"});

  std::vector<std::unique_ptr<ui::Widget>> v;
  v.push_back(std::move(label));
  v.push_back(std::move(btn));

  auto tree = std::make_unique<ui::Container>(
      ui::Direction{ui::Direction::kColumn},
      ui::Gap{16},
      ui::Width{320}, ui::Height{260},
      ui::Background{ui::Color{250, 250, 255}},
      ui::Padding{ui::EdgeInsets::All(32)},
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
