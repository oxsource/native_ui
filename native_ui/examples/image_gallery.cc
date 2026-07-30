#include <chrono>
#include <cstdio>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "canvas.h"
#include "container.h"
#include "edge_insets.h"
#include "glide.h"
#include "image_widget.h"
#include "paint.h"
#include "png_writer.h"
#include "style.h"
#include "surface.h"
#include "text.h"

namespace ui = native::ui;

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

static std::unique_ptr<ui::Container> MakeCard(
    const std::string& title,
    const std::string& img_path,
    ui::ScaleMode scale_type) {
  auto img = std::make_unique<ui::ImageWidget>(
      ui::ImageURI{img_path},
      ui::Width{120}, ui::Height{320},
      scale_type,
      ui::Background{ui::Color{240, 240, 245}},
      ui::CornerRadius{8});
  auto label = std::make_unique<ui::Text>(
      ui::Content{title},
      ui::FontSize{12},
      ui::TextAlign{ui::TextAlign::kCenter},
      ui::Background{ui::kTransparent},
      ui::TextColor{ui::Color{60, 60, 70}},
      ui::Width{140},
      ui::Height{24});
  std::vector<std::unique_ptr<ui::Widget>> v;
  v.push_back(std::move(img));
  v.push_back(std::move(label));
  return std::make_unique<ui::Container>(
      ui::Direction{ui::Direction::kColumn},
      ui::Gap{8},
      ui::AlignItems{ui::AlignItems::kCenter},
      ui::Background{ui::Color{255, 255, 255}},
      ui::Padding{ui::EdgeInsets::All(12)},
      ui::Container::Children{std::move(v)});
}

int main() {
  ui::Glide::SetDefault(new ui::DefaultGlide());

  std::string base = "assets/photo/";
  std::string svg = base + "superdog.svg";
  std::string png = base + "police.png";
  std::vector<std::unique_ptr<ui::Widget>> cards;
  cards.push_back(MakeCard("SVG kCenter",      svg, ui::ScaleMode::kCenter));
  cards.push_back(MakeCard("SVG kCenterInside", svg, ui::ScaleMode::kCenterInside));
  cards.push_back(MakeCard("PNG kCenterCrop",   png, ui::ScaleMode::kCenterCrop));
  cards.push_back(MakeCard("PNG kCenterInside", png, ui::ScaleMode::kCenterInside));
  cards.push_back(MakeCard("PNG kFillXY",       png, ui::ScaleMode::kFillXY));

  auto tree = std::make_unique<ui::Container>(
      ui::Direction{ui::Direction::kRow},
      ui::Gap{16},
      ui::Width{800}, ui::Height{280},
      ui::Background{ui::Color{215, 215, 230}},
      ui::Padding{ui::EdgeInsets::All(20)},
      ui::Container::Children{std::move(cards)});

  // Wait for async Glide loads to complete, then drain callbacks to main thread
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  auto* glide = ui::Glide::Default();
  if (glide) glide->DrainPendingCallbacks();

  RenderAndSave(tree.get(), "/tmp/image_gallery.png");
  std::printf("Generated: /tmp/image_gallery.png\n");
  return 0;
}
