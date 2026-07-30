#include <cstdio>
#include "canvas.h"
#include "paint.h"
#include "surface.h"

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

  SkPixmap pm;
  if (!surface->sk_surface()->peekPixels(&pm)) {
    fprintf(stderr, "peekPixels FAIL\n");
    return 1;
  }

  const auto* addr = static_cast<const uint8_t*>(pm.addr());
  int non_black = 0;
  for (int i = 0; i < 100 * 100; i++) {
    if (addr[i*4+0] || addr[i*4+1] || addr[i*4+2]) non_black++;
  }
  fprintf(stderr, "non-black pixels: %d / 10000\n", non_black);
  fprintf(stderr, "pixel at (15,15): %d %d %d %d\n", addr[15*4*100+15*4+0], addr[15*4*100+15*4+1], addr[15*4*100+15*4+2], addr[15*4*100+15*4+3]);
  fprintf(stderr, "pixel at (5,5):   %d %d %d %d\n", addr[5*4*100+5*4+0], addr[5*4*100+5*4+1], addr[5*4*100+5*4+2], addr[5*4*100+5*4+3]);

  return 0;
}
