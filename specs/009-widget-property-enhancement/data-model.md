# Data / Entity Model: Widget Property Enhancement

**Date**: 2026-07-30

## Entity: Style

| Field | Type | Description |
|-------|------|-------------|
| `width_` / `width_set_` | float / bool | Preferred width |
| `height_` / `height_set_` | float / bool | Preferred height |
| `min_width_` / `min_width_set_` | float / bool | Minimum width |
| `max_width_` / `max_width_set_` | float / bool | Maximum width |
| `padding_` / `padding_set_` | EdgeInsets / bool | Inner padding |
| `background_` / `background_set_` | Color / bool | Background fill color |
| `gradient_` / `gradient_set_` | Gradient / bool | Background gradient |
| `enabled_` / `enabled_set_` | bool / bool | Widget enabled |
| `visible_` / `visible_set_` | bool / bool | Widget visible |
| `opacity_` / `opacity_set_` | float / bool | Alpha multiplier |
| `corner_radius_` / `corner_radius_set_` | float / bool | Corner rounding |
| `border_width_` / `border_width_set_` | float / bool | Border stroke width |
| `border_color_` / `border_color_set_` | Color / bool | Border stroke color |
| `shadow_offset_` / `shadow_offset_set_` | Point / bool | Shadow offset |
| `shadow_radius_` / `shadow_radius_set_` | float / bool | Shadow blur radius |
| `shadow_color_` / `shadow_color_set_` | Color / bool | Shadow color |
| `font_size_` / `font_size_set_` | float / bool | Font size |
| `text_color_` / `text_color_set_` | Color / bool | Text color |
| `text_align_` / `text_align_set_` | TextAlign / bool | Text alignment |
| `font_family_` / `font_family_set_` | string / bool | Typeface name |
| `font_weight_` / `font_weight_set_` | int / bool | Font weight (100-900) |
| `line_height_` / `line_height_set_` | float / bool | Line spacing multiplier |
| `max_lines_` / `max_lines_set_` | int / bool | Max text lines |
| `scale_type_` / `scale_type_set_` | ScaleMode / bool | Image scale mode |
| `scale_gravity_` / `scale_gravity_set_` | Gravity / bool | Image crop/anchor |
| `placeholder_` / `placeholder_set_` | string / bool | Loading placeholder path |
| `error_image_` / `error_image_set_` | string / bool | Error fallback path |
| `priority_` | StylePriority | This Style's merge priority |

**Methods**: chainable setters (`setWidth`, `setFontSize`, etc.), `Merge(base, overlay)`

## Entity: Gradient

| Variant | Factory | Params |
|---------|---------|--------|
| Linear | `Gradient::Linear(from, to, stops)` | `from, to`: Point; `stops`: vector of `(float position, Color color)` |
| Radial | `Gradient::Radial(center, radius, stops)` | `center`: Point; `radius`: float |

## Entity: Glide

| Field | Type | Description |
|-------|------|-------------|
| `cache_` | `LRUCache<string, shared_ptr<Image>>` | In-memory decoded image cache |
| `requests_` | `map<uint64_t, Request>` | Pending load requests |
| `next_id_` | uint64_t | Monotonic request ID counter |

**Methods**: `Load(path, callback, options) → uint64_t`, `Cancel(id)`, `ClearCache()`

## Entity: ImageWidget (updated)

| Field | Type | Description |
|-------|------|-------------|
| `style_` | Style (inherited) | ScaleType, ScaleGravity, Placeholder path, ErrorImage path — all stored in style_, delegated via ProcessArg |
| `uri_` | string | Image URI for Glide loading |
| `loaded_image_` | shared_ptr<Image> | Glide-decoded image |
| `state_` | LoadState | kLoading/kLoaded/kError |
| `request_id_` | uint64_t | Active Glide request ID |
| `load_key_` | string | URI copy for stale callback check |

**ProcessArg delegation**: `ProcessArg(ScaleType tag)` → `style_.setScaleType(tag.value)`. `ProcessArg(Placeholder tag)` → `style_.setPlaceholder(tag.value)`. No separate member fields for visual properties.

**Draw reads from style()**: `auto mode = style().scale_type()` to determine transform. Placeholder/ErrorImage paths from `style().placeholder()`/`style().error_image()`.

**State Machine**: Construct → `Load()` → kLoading → show Placeholder → decode → kLoaded → show image; or → kError → show ErrorImage. `Cancel()` on destruction/URI change.

## Validation Rules

| Rule | Entity | Description |
|------|--------|-------------|
| Opacity clamped 0–1 | Widget | Values outside range clamped |
| Enabled(false) blocks events | Widget | EventHub checks before dispatch |
| Width/Height as Yoga constraints | Widget | Preferred, not forced |
| Gradient wins over Color | Widget | Both Background + BackgroundGradient set → Gradient renders |
| ScaleType(kCenter) no scaling | ImageWidget | Natural size, centered, cropped if too large |
| kCenterInside no upscale | ImageWidget | Smaller-than-bounds images stay at natural size |
| Glide callback key check | ImageWidget | Stale callback ignored if `load_key_` changed |
| Style is_set prevents override | Style | Merge only for set properties |
