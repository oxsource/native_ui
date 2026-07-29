#include "SkCanvas.h"
#include "SkSurface.h"
#include "SkPaint.h"
#include "SkFont.h"
#include "SkGraphics.h"
#include "SkColor.h"

#include "yoga/Yoga.h"

#include "png_writer.h"

#include <cstdio>
#include <cstring>

struct BoxConfig {
  float width;
  float height;
  SkColor color;
  float margin;
};

static void DrawRect(SkCanvas* canvas, float x, float y, float w, float h,
                     SkColor color) {
  SkPaint paint;
  paint.setColor(color);
  paint.setAntiAlias(true);
  canvas->drawRect(SkRect::MakeXYWH(x, y, w, h), paint);
}

static void DrawBoundingBox(SkCanvas* canvas, float x, float y, float w,
                            float h) {
  SkPaint paint;
  paint.setColor(SK_ColorBLACK);
  paint.setStyle(SkPaint::kStroke_Style);
  paint.setStrokeWidth(1);
  canvas->drawRect(SkRect::MakeXYWH(x, y, w, h), paint);
}

static void LayoutAndRender(SkCanvas* canvas, float offset_x, float offset_y,
                            YGFlexDirection direction,
                            const BoxConfig boxes[], int box_count,
                            float container_padding) {
  YGNodeRef root = YGNodeNew();
  YGNodeStyleSetFlexDirection(root, direction);
  YGNodeStyleSetPadding(root, YGEdgeAll, container_padding);
  YGNodeStyleSetWidth(root, 300);
  YGNodeStyleSetHeight(root, direction == YGFlexDirectionRow ? 100 : 220);

  std::vector<YGNodeRef> nodes;
  for (int i = 0; i < box_count; ++i) {
    YGNodeRef child = YGNodeNew();
    YGNodeStyleSetWidth(child, boxes[i].width);
    YGNodeStyleSetHeight(child, boxes[i].height);
    YGNodeStyleSetMargin(child, YGEdgeAll, boxes[i].margin);
    YGNodeInsertChild(root, child, i);
    nodes.push_back(child);
  }

  YGNodeCalculateLayout(root, YGUndefined, YGUndefined, YGDirectionLTR);

  float root_x = offset_x;
  float root_y = offset_y;

  for (int i = 0; i < box_count; ++i) {
    YGNodeRef node = nodes[i];
    float x = root_x + YGNodeLayoutGetLeft(node);
    float y = root_y + YGNodeLayoutGetTop(node);
    float w = YGNodeLayoutGetWidth(node);
    float h = YGNodeLayoutGetHeight(node);
    DrawRect(canvas, x, y, w, h, boxes[i].color);
    DrawBoundingBox(canvas, x, y, w, h);
  }

  DrawBoundingBox(canvas, root_x, root_y,
                  YGNodeLayoutGetWidth(root),
                  YGNodeLayoutGetHeight(root));

  YGNodeFreeRecursive(root);
}

int main(int argc, char** argv) {
  const char* path = argc > 1 ? argv[1] : "/tmp/yoga_skia_output.png";

  SkGraphics::Init();

  int canvas_w = 640;
  int canvas_h = 360;
  auto surface = SkSurfaces::Raster(
      SkImageInfo::MakeN32Premul(canvas_w, canvas_h));
  if (!surface) {
    std::fprintf(stderr, "FAIL: SkSurface::Raster returned null\n");
    return 1;
  }

  auto* canvas = surface->getCanvas();
  canvas->clear(SK_ColorWHITE);

  SkFont label_font;
  SkPaint label_paint;
  label_paint.setColor(SK_ColorDKGRAY);
  label_paint.setAntiAlias(true);

  canvas->drawSimpleText("flexDirection: row (margin: 8)",
                         strlen("flexDirection: row (margin: 8)"),
                         SkTextEncoding::kUTF8, 15, 25, label_font,
                         label_paint);

  BoxConfig row_boxes[] = {
    {60, 60, SkColorSetRGB(0xE7, 0x4C, 0x3C), 8},
    {60, 60, SkColorSetRGB(0x2E, 0xCC, 0x71), 8},
    {60, 60, SkColorSetRGB(0x34, 0x98, 0xDB), 8},
  };
  LayoutAndRender(canvas, 15, 40, YGFlexDirectionRow, row_boxes, 3, 10);

  canvas->drawSimpleText("flexDirection: column (margin: 6)",
                         strlen("flexDirection: column (margin: 6)"),
                         SkTextEncoding::kUTF8, 15, 165, label_font,
                         label_paint);

  BoxConfig col_boxes[] = {
    {260, 40, SkColorSetRGB(0xE6, 0x7E, 0x22), 6},
    {260, 40, SkColorSetRGB(0x9B, 0x59, 0xB6), 6},
    {260, 40, SkColorSetRGB(0x1A, 0xBC, 0x9C), 6},
  };
  LayoutAndRender(canvas, 15, 180, YGFlexDirectionColumn, col_boxes, 3, 10);

  if (!WriteSkSurfaceToPNG(surface.get(), path)) {
    return 1;
  }

  std::printf("Yoga+Skia spike passed — wrote %s\n", path);
  return 0;
}
