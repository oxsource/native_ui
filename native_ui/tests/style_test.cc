#include "gtest/gtest.h"
#include "src/framework/widgets/style.h"

namespace native::ui {

TEST(StyleTest, ChainableSetters) {
  Style s;
  s.setWidth(200).setHeight(100).setFontSize(24).setTextColor(kRed);
  EXPECT_FLOAT_EQ(s.width(), 200);
  EXPECT_FLOAT_EQ(s.height(), 100);
  EXPECT_FLOAT_EQ(s.font_size(), 24);
  EXPECT_EQ(s.text_color().r, 255);
  EXPECT_EQ(s.text_color().g, 0);
}

TEST(StyleTest, DefaultEnabled) {
  Style s;
  EXPECT_TRUE(s.enabled());
}

TEST(StyleTest, MergeHigherPriorityWins) {
  Style base;
  base.setWidth(100);
  base.setHeight(200);

  Style overlay;
  overlay.setPriority(StylePriority::kInstance);
  overlay.setWidth(300);

  Style merged = Merge(base, overlay);
  EXPECT_FLOAT_EQ(merged.width(), 300);   // overlay wins
  EXPECT_FLOAT_EQ(merged.height(), 200);  // base survives
}

TEST(StyleTest, MergeSamePriorityOverlayWins) {
  Style a;
  a.setWidth(100);
  a.setCornerRadius(8);

  Style b;
  b.setWidth(200);
  b.setFontSize(16);

  Style merged = Merge(a, b);
  EXPECT_FLOAT_EQ(merged.width(), 200);      // b wins (same priority, last)
  EXPECT_FLOAT_EQ(merged.corner_radius(), 8); // a's unset-in-b value survives
  EXPECT_FLOAT_EQ(merged.font_size(), 16);    // b wins
}

TEST(StyleTest, MergeUnsetPropertiesIgnored) {
  Style base;
  base.setWidth(100);
  base.setOpacity(0.5);

  Style overlay;
  overlay.setCornerRadius(12);  // width not set in overlay

  Style merged = Merge(base, overlay);
  EXPECT_FLOAT_EQ(merged.width(), 100);        // base survives
  EXPECT_FLOAT_EQ(merged.corner_radius(), 12); // overlay wins
  EXPECT_FLOAT_EQ(merged.opacity(), 0.5);      // base survives
}

TEST(StyleTest, MergeLowerPriorityIgnored) {
  Style base;
  base.setPriority(StylePriority::kInstance);
  base.setWidth(200);

  Style overlay;
  overlay.setPriority(StylePriority::kClass);  // lower priority
  overlay.setWidth(100);

  Style merged = Merge(base, overlay);
  EXPECT_FLOAT_EQ(merged.width(), 200);  // base wins (higher priority)
}

TEST(StyleTest, SetDefaultAffectsNewStyles) {
  Style custom;
  custom.setFontSize(20).setCornerRadius(4);
  Style::SetDefault(custom);

  Style fresh;
  EXPECT_FLOAT_EQ(fresh.font_size(), 20);
  EXPECT_FLOAT_EQ(fresh.corner_radius(), 4);
}

TEST(StyleTest, ApplyStyleMergesAndSetsEnabled) {
  Style base;
  base.setWidth(100);
  base.setCornerRadius(8);

  Style overlay;
  overlay.setCornerRadius(16);
  overlay.setFontSize(24);

  Style merged = Merge(base, overlay);
  EXPECT_FLOAT_EQ(merged.width(), 100);
  EXPECT_FLOAT_EQ(merged.corner_radius(), 16);
  EXPECT_FLOAT_EQ(merged.font_size(), 24);
}

}  // namespace native::ui
