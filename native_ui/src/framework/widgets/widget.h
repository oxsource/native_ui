#pragma once

#include <functional>
#include <string>
#include <vector>

#include "edge_insets.h"
#include "gradient.h"
#include "point.h"
#include "property.h"
#include "rect.h"
#include "size.h"
#include "style.h"

namespace native::ui {

struct Id {
  std::string value;
};

struct MouseEvent;
struct KeyEvent;

// ── Widget base style tags ──

struct Width { float value; };
struct Height { float value; };
struct MinWidth { float value; };
struct MaxWidth { float value; };
struct Padding { EdgeInsets value; };
struct Background { Color value; };
struct BackgroundGradient { Gradient value; };
struct Enabled { bool value; };
struct Visible { bool value; };
struct Opacity { float value; };
struct CornerRadius { float value; };
struct BorderWidth { float value; };
struct BorderColor { Color value; };
struct ShadowOffset { Point value; };
struct ShadowRadius { float value; };
struct ShadowColor { Color value; };

class Widget {
public:
  Widget() { style_.setEnabled(true); }
  virtual ~Widget() = default;

  void SetId(std::string id) { id_ = std::move(id); }
  const std::string& GetId() const { return id_; }

  void SetBounds(Rect b) { bounds_ = b; }
  const Rect& bounds() const { return bounds_; }

  virtual Widget* ChildAt(int index) { return nullptr; }
  virtual int ChildCount() const { return 0; }
  virtual int IndexOf(Widget* child) const { return -1; }

  Widget* FindById(const std::string& id);

  void RequestLayout();
  virtual void RequestRedraw();

  template<typename T>
  void Watch(Property<T>& prop);

  void UnwatchAll();

  // ── Style system ──
  const Style& style() const { return style_; }
  void ApplyStyle(const Style& s) {
    style_ = Merge(style_, s);
    needs_draw_ = true;
  }



  // ── ProcessArg: style tags → Style::setXxx ──
  void ProcessArg(Width tag)     { style_.setWidth(tag.value); }
  void ProcessArg(Height tag)    { style_.setHeight(tag.value); }
  void ProcessArg(MinWidth tag)  { style_.setMinWidth(tag.value); }
  void ProcessArg(MaxWidth tag)  { style_.setMaxWidth(tag.value); }
  void ProcessArg(Padding tag)   { style_.setPadding(tag.value); }
  void ProcessArg(Background tag) { style_.setBackground(tag.value); }
  void ProcessArg(BackgroundGradient tag) { style_.setBackgroundGradient(tag.value); }
  void ProcessArg(Enabled tag)   { style_.setEnabled(tag.value); }
  void ProcessArg(Visible tag)   { style_.setVisible(tag.value); }
  void ProcessArg(Opacity tag)   { style_.setOpacity(tag.value); }
  void ProcessArg(CornerRadius tag) { style_.setCornerRadius(tag.value); }
  void ProcessArg(BorderWidth tag)  { style_.setBorderWidth(tag.value); }
  void ProcessArg(BorderColor tag)  { style_.setBorderColor(tag.value); }
  void ProcessArg(ShadowOffset tag) { style_.setShadowOffset(tag.value); }
  void ProcessArg(ShadowRadius tag) { style_.setShadowRadius(tag.value); }
  void ProcessArg(ShadowColor tag)  { style_.setShadowColor(tag.value); }

  virtual void OnMount() {}
  virtual void OnUnmount() {}
  virtual void Draw(class Canvas&) = 0;

  virtual bool OnMouseEvent(const struct MouseEvent& event);
  virtual bool OnKeyEvent(const struct KeyEvent& event);

  bool needs_layout() const { return needs_layout_; }
  bool needs_draw() const { return needs_draw_; }

protected:
  Style style_;

private:
  std::string id_;
  Rect bounds_;
  bool needs_layout_ = false;
  bool needs_draw_ = false;
  State* watched_state_ = nullptr;
};

}  // namespace native::ui

#include "widget_inl.h"
