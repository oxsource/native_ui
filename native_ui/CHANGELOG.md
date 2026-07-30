# Changelog

## [0.1.0] - 2026-07-30

### Added

- **Core Types**: Rect, Point, Size, Color, EdgeInsets for geometric primitives
- **Widget System**: Widget base class with ID, bounds, layout invalidation, data binding via State/Property\<T\>
- **Container**: Flexbox container with tagged-parameter construction, child add/remove, layout invalidation
- **Layout Engine**: FlexLayout wrapper over Yoga with Measure/Arrange pipeline
- **Render Wrappers**: Canvas (RAII with save/restore), Paint (chainable builder), Path (vector path builder)
- **Surface**: Backing store for Skia rendering with platform buffer support (IOSurface, AHardwareBuffer, DMA-BUF)
- **Image Support**: Image decode from file, encoded data, and hardware buffers
- **State Management**: State base class with Property\<T\>, Watch mechanism, thread-safe dirty queue with Flush
- **Basic Widgets**: Text (with data binding), Button (with hit detection and callback), ImageWidget (file-based), ExternalImage (hardware buffer), Stack (z-order layering)
- **Layout Pipeline**: Container layout with FlexLayout integration, positioned child drawing via canvas Translate/ClipRect
- **Event System**: EventHub with Push for MouseEvent/KeyEvent, HitTester with DFS z-order aware hit testing, filter chain with AddFilter
- **DebugOverlay**: Toggleable diagnostic overlay with layout borders, FPS counter, widget tree breadcrumb
- **Hello World Example**: End-to-end demo with widget tree, data binding, layout, rendering, and PNG output
- **CI Pipeline**: GitHub Actions workflows for CI, PR gate, and release
- **Shared Library**: Build target for distributable shared library artifact
