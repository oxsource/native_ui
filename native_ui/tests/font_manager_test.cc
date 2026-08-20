#include "gtest/gtest.h"

#include <cstdio>
#include <cstdlib>
#include <string>

#include "src/framework/render/canvas.h"
#include "src/framework/render/font_manager.h"
#include "src/framework/surface/surface.h"

namespace native::ui {
namespace {

std::string FontAsset(const char* name) {
  // Bazel runfiles dir; fall back to a raw repo path for standalone runs.
  const char* srcdir = std::getenv("TEST_SRCDIR");
  if (srcdir) {
    return std::string(srcdir) + "/native_ui/tests/assets/fonts/" + name;
  }
  const char* override = std::getenv("NATIVE_UI_FONTS_DIR");
  if (override) {
    return std::string(override) + "/" + name;
  }
  return std::string("tests/assets/fonts/") + name;
}

// Fresh manager state per test so registrations never leak across cases.
class FontManagerTest : public ::testing::Test {
protected:
  void SetUp() override { FontManager::Default().Clear(); }
  void TearDown() override { FontManager::Default().Clear(); }
};

// SC-001: registering a font by path and referencing it must produce
// non-empty text measurement (previously the drawn/measured typeface was
// empty on non-Apple platforms).
TEST_F(FontManagerTest, RegisteredFontMeasuresNonEmpty) {
  FontManager& fm = FontManager::Default();
  ASSERT_TRUE(fm.RegisterFont("demo", FontAsset("roboto_regular.ttf"), 400));

  auto surface = Surface::Create(200, 50);
  ASSERT_NE(surface, nullptr);
  Canvas canvas(*surface);
  Font font;
  font.family = "demo";
  font.weight = 400;
  font.size = 24.0f;
  Rect bounds = canvas.MeasureText("Hello", font);
  EXPECT_GT(bounds.width, 0.0f);
  EXPECT_GT(bounds.height, 0.0f);
}

// FR-013 + SC-006: the first successful registration becomes the default font,
// so text with an unset family uses it and measures non-empty.
TEST_F(FontManagerTest, FirstRegistrationBecomesDefault) {
  FontManager& fm = FontManager::Default();
  ASSERT_FALSE(fm.HasDefaultFont());
  ASSERT_TRUE(fm.RegisterFont("first", FontAsset("roboto_regular.ttf"), 400));
  EXPECT_TRUE(fm.HasDefaultFont());

  auto surface = Surface::Create(200, 50);
  ASSERT_NE(surface, nullptr);
  Canvas canvas(*surface);
  Font unset;  // family empty
  unset.size = 24.0f;
  Rect bounds = canvas.MeasureText("Hello", unset);
  EXPECT_GT(bounds.width, 0.0f);

  // Metrics match the default family's dedicated resolve.
  Font explicit_font;
  explicit_font.family = "first";
  explicit_font.weight = 400;
  explicit_font.size = 24.0f;
  Rect explicit_bounds = canvas.MeasureText("Hello", explicit_font);
  EXPECT_FLOAT_EQ(bounds.width, explicit_bounds.width);
}

// FR-014: SetDefaultFont re-points the default to another registered family;
// invalid family → false + last_error, default unchanged.
TEST_F(FontManagerTest, SetDefaultFontOverride) {
  FontManager& fm = FontManager::Default();
  ASSERT_TRUE(fm.RegisterFont("first", FontAsset("roboto_regular.ttf"), 400));
  ASSERT_TRUE(fm.RegisterFont("second", FontAsset("montserrat_regular.ttf"), 400));
  ASSERT_EQ(fm.HasDefaultFont(), true);
  EXPECT_TRUE(fm.SetDefaultFont("second"));
  EXPECT_TRUE(fm.SetDefaultFont("unregistered") == false);
  EXPECT_FALSE(fm.last_error().empty());

  auto surface = Surface::Create(200, 50);
  ASSERT_NE(surface, nullptr);
  Canvas canvas(*surface);
  Font unset;
  unset.size = 24.0f;
  Rect default_bounds = canvas.MeasureText("Hello", unset);
  Font second;
  second.family = "second";
  second.weight = 400;
  second.size = 24.0f;
  Rect second_bounds = canvas.MeasureText("Hello", second);
  EXPECT_FLOAT_EQ(default_bounds.width, second_bounds.width);
}

// FR-010: re-registering the same (family, weight) with a new path replaces the
// entry; default designation persists.
TEST_F(FontManagerTest, ReRegisterRefreshesEntry) {
  FontManager& fm = FontManager::Default();
  ASSERT_TRUE(fm.RegisterFont("demo", FontAsset("roboto_regular.ttf"), 400));
  ASSERT_TRUE(fm.RegisterFont("demo", FontAsset("roboto_regular.ttf"), 400));
  EXPECT_TRUE(fm.HasDefaultFont());
  EXPECT_EQ(fm.last_error(), "");
}

// FR-006: missing/unreadable/corrupt file → RegisterFont false + last_error,
// default entry untouched (no crash).
TEST_F(FontManagerTest, MissingFileRejected) {
  FontManager& fm = FontManager::Default();
  bool ok = fm.RegisterFont("bad", "/nonexistent/font.ttf", 400);
  EXPECT_FALSE(ok);
  EXPECT_FALSE(fm.last_error().empty());
  EXPECT_FALSE(fm.HasDefaultFont());  // failed registration never sets default
}

// SC-002: a family with regular+bold registered renders wider/bolder metrics at
// weight 700 than at 400 for the same glyphs (FontWeight selects the variant).
TEST_F(FontManagerTest, BoldVariantWiderThanRegular) {
  FontManager& fm = FontManager::Default();
  ASSERT_TRUE(fm.RegisterFont("demo", FontAsset("roboto_regular.ttf"), 400));
  ASSERT_TRUE(fm.RegisterFont("demo", FontAsset("roboto_bold.ttf"), 700));

  auto surface = Surface::Create(300, 50);
  ASSERT_NE(surface, nullptr);
  Canvas canvas(*surface);
  Font regular;
  regular.family = "demo";
  regular.weight = 400;
  regular.size = 24.0f;
  Font bold;
  bold.family = "demo";
  bold.weight = 700;
  bold.size = 24.0f;
  float w400 = canvas.MeasureText("Hello", regular).width;
  float w700 = canvas.MeasureText("Hello", bold).width;
  EXPECT_GT(w400, 0.0f);
  EXPECT_GT(w700, w400);
}

// FR-004: with only weights 400 and 900 registered, a request for 500 picks the
// nearest (400). Single-variant family resolves to that one file for any weight.
TEST_F(FontManagerTest, NearestWeightSelection) {
  FontManager& fm = FontManager::Default();
  ASSERT_TRUE(fm.RegisterFont("gap", FontAsset("roboto_regular.ttf"), 400));
  ASSERT_TRUE(fm.RegisterFont("gap", FontAsset("roboto_bold.ttf"), 900));

  auto surface = Surface::Create(300, 50);
  ASSERT_NE(surface, nullptr);
  Canvas canvas(*surface);
  Font mid;
  mid.family = "gap";
  mid.weight = 500;
  mid.size = 24.0f;
  Font reg;
  reg.family = "gap";
  reg.weight = 400;
  reg.size = 24.0f;
  // Nearest to 500 is 400 (Δ100 < Δ400), so metrics match the 400 file.
  float mid_w = canvas.MeasureText("Hello", mid).width;
  float reg_w = canvas.MeasureText("Hello", reg).width;
  EXPECT_GT(mid_w, 0.0f);
  EXPECT_FLOAT_EQ(mid_w, reg_w);
}

// FR-004 + story-2 edge: single-variant family answers any requested weight with
// its only file (no crash, non-empty measure).
TEST_F(FontManagerTest, SingleVariantAnyWeight) {
  FontManager& fm = FontManager::Default();
  ASSERT_TRUE(fm.RegisterFont("single", FontAsset("roboto_regular.ttf"), 400));

  auto surface = Surface::Create(300, 50);
  ASSERT_NE(surface, nullptr);
  Canvas canvas(*surface);
  Font any;
  any.family = "single";
  any.weight = 700;
  any.size = 24.0f;
  float w = canvas.MeasureText("Hello", any).width;
  EXPECT_GT(w, 0.0f);
}

// SC-003 + FR-007: unknown family + a default registered → falls back to the
// default font's metrics, no crash.
TEST_F(FontManagerTest, UnknownFamilyFallsBackToDefault) {
  FontManager& fm = FontManager::Default();
  ASSERT_TRUE(fm.RegisterFont("first", FontAsset("roboto_regular.ttf"), 400));

  auto surface = Surface::Create(300, 50);
  ASSERT_NE(surface, nullptr);
  Canvas canvas(*surface);
  Font unknown;
  unknown.family = "never_registered";
  unknown.weight = 400;
  unknown.size = 24.0f;
  Font def;
  def.family = "first";
  def.weight = 400;
  def.size = 24.0f;
  float unknown_w = canvas.MeasureText("Hello", unknown).width;
  float default_w = canvas.MeasureText("Hello", def).width;
  EXPECT_GT(unknown_w, 0.0f);
  EXPECT_FLOAT_EQ(unknown_w, default_w);
}

// FR-012/SC-005: with NO registration at all, an empty-family text meads no-op
// (returns zero/empty bounds) but does not crash — platform-default path.
TEST_F(FontManagerTest, NoRegistrationNoCrash) {
  FontManager& fm = FontManager::Default();
  EXPECT_FALSE(fm.HasDefaultFont());

  auto surface = Surface::Create(300, 50);
  ASSERT_NE(surface, nullptr);
  Canvas canvas(*surface);
  Font unset;
  unset.size = 24.0f;
  Rect bounds = canvas.MeasureText("Hello", unset);
  // On macOS the CoreText default yields non-empty measurement (unchanged);
  // the contract only requires no crash and a valid (possibly zero) result.
  EXPECT_GE(bounds.width, 0.0f);
  EXPECT_GE(bounds.height, 0.0f);
}

// FR-006 hardening: corrupt (non-font) file → RegisterFont false + last_error.
TEST_F(FontManagerTest, CorruptFileRejected) {
  // Write a junk file to a temp path, then try to register it.
  const char* tmpl = std::getenv("TMPDIR");
  std::string dir = tmpl ? std::string(tmpl) : "/tmp";
  std::string path = dir + "/native_ui_corrupt_font_test.bin";
  {
    FILE* f = fopen(path.c_str(), "wb");
    ASSERT_NE(f, nullptr);
    const char junk[] = "this is not a font file, definitely not ttf";
    fwrite(junk, 1, sizeof(junk), f);
    fclose(f);
  }
  FontManager& fm = FontManager::Default();
  bool ok = fm.RegisterFont("bad", path, 400);
  EXPECT_FALSE(ok);
  EXPECT_FALSE(fm.last_error().empty());
  EXPECT_FALSE(fm.HasDefaultFont());
  remove(path.c_str());
}

}  // namespace
}  // namespace native::ui