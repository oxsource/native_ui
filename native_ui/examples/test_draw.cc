#include <cstdio>
#include "src/framework/render/canvas.h"
#include "src/framework/render/paint.h"
#include "src/framework/surface/surface.h"

int main() {
  auto surface = native::ui::Surface::Create(100, 100);
  if (!surface) { fprintf(stderr, "surface fail\n"); return 1; }

  {
    native::ui::Canvas canvas(*surface);
    native::ui::Paint p;
    p.SetColor(native::ui::Color{255, 0, 0});
    canvas.DrawRect(native::ui::Rect{10, 10, 50, 50}, p);
  }

  surface->Flush();

  if (!surface->Dump("/tmp/test_draw.png")) {
    fprintf(stderr, "Dump FAIL\n");
    return 1;
  }
  fprintf(stderr, "wrote /tmp/test_draw.png\n");
  return 0;
}
