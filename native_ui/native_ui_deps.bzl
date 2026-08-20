load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def _skia():
    http_archive(
        name = "skia",
        urls = ["https://github.com/google/skia/archive/fdbe14582b177785e0e77c938277ef332d45636f.tar.gz"],
        sha256 = "801f902c7d867783950fd2c12f7416b55f06a8129e2247b245389b6d3c3b158a",
        strip_prefix = "skia-fdbe14582b177785e0e77c938277ef332d45636f",
        build_file = "//third_party:skia.BUILD",
        patch_cmds = ["find . -mindepth 2 -name BUILD.bazel -delete"],
    )

def _yoga():
    # Yoga v2.0.0 (2023-06-30) — C++17 compatible, used by React Native 0.73.
    # v3.0+ requires C++20 (std::floating_point concept, abbreviated auto templates).
    # Downgraded from v3.2.1 to keep the project on C++17. Feature-wise v2.0.0
    # covers all flexbox primitives needed: direction, margin, padding, gap,
    # grow/shrink, alignment, wrapping.
    http_archive(
        name = "yoga",
        urls = ["https://github.com/react/yoga/archive/refs/tags/v2.0.0.tar.gz"],
        sha256 = "29eaf05191dd857f76b6db97c77cce66db3c0067c88bd5e052909386ea66b8c5",
        strip_prefix = "yoga-2.0.0",
        build_file = "//third_party:yoga.BUILD",
    )

def _googletest():
    http_archive(
        name = "com_google_googletest",
        sha256 = "8ad598c73ad796e0d8280b082cebd82a630d73e73cd3c70057938a6501bba5d7",
        strip_prefix = "googletest-1.14.0",
        urls = ["https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz"],
    )

def _bazel_skylib():
    http_archive(
        name = "bazel_skylib",
        urls = ["https://github.com/bazelbuild/bazel-skylib/archive/refs/tags/1.6.1.tar.gz"],
        sha256 = "aede1b60709ac12b3461ee0bb3fa097b58a86fbfdb88ef7e9f90424a69043167",
        strip_prefix = "bazel-skylib-1.6.1",
    )

def _stblib():
    http_archive(
        name = "stblib",
        build_file = "//third_party:stblib.BUILD",
        patch_cmds = [
            "echo '#define STB_IMAGE_WRITE_IMPLEMENTATION' > stb_image_write.c",
            "echo '#include \"stb_image_write.h\"' >> stb_image_write.c",
            "echo '#define STB_IMAGE_IMPLEMENTATION' > stb_image_impl.c",
            "echo '#include \"stb_image.h\"' >> stb_image_impl.c",
        ],
        sha256 = "13a99ad430e930907f5611325ec384168a958bf7610e63e60e2fd8e7b7379610",
        strip_prefix = "stb-b42009b3b9d4ca35bc703f5310eedc74f584be58",
        url = "https://github.com/nothings/stb/archive/b42009b3b9d4ca35bc703f5310eedc74f584be58.tar.gz",
    )

def _nanosvg():
    http_archive(
        name = "nanosvg",
        urls = ["https://github.com/memononen/nanosvg/archive/239e102ec2c691f2902e20ace2ed36ee4a35cfe6.tar.gz"],
        sha256 = "2bc68bdb518d7800252042e5cad50a0ab321596f0cbf49ef2a752926329063d2",
        strip_prefix = "nanosvg-239e102ec2c691f2902e20ace2ed36ee4a35cfe6",
        build_file = "//third_party:nanosvg.BUILD",
    )

def _freetype():
    # FreeType 2.13.2 — C font rasterizer required by Skia's FreeType port
    # (SkFontHost_FreeType / SkTypeface_FreeType) and the custom font manager,
    # so that registered font files render real glyphs on Android/Linux.
    http_archive(
        name = "freetype",
        urls = ["https://github.com/freetype/freetype/archive/refs/tags/VER-2-13-2.tar.gz"],
        sha256 = "427201f5d5151670d05c1f5b45bef5dda1f2e7dd971ef54f0feaaa7ffd2ab90c",
        strip_prefix = "freetype-VER-2-13-2",
        build_file = "//third_party:freetype.BUILD",
        # The shipped (non-GNU-make) ftmodule.h registers the SVG renderer, whose
        # class (ft_svg_renderer_class) is only compiled with FT_CONFIG_OPTION_USE_SVG
        # (needs librsvg). We exclude src/svg/*.c, so drop that registration line to
        # keep the FT_Init_FreeType module table consistent with the compiled modules.
        patch_cmds = [
            "sed -i.bak '/ft_svg_renderer_class/d' include/freetype/config/ftmodule.h && rm -f include/freetype/config/ftmodule.h.bak",
        ],
    )

def _rules_android_ndk():
    # External NDK rules that support NDK r25b+ with Bazel 6.5+. Verified working
    # in the atlas project (tools/platform_setup.sh + WORKSPACE). android_ndk_repository
    # below is tolerant of a missing/invalid $ANDROID_NDK_HOME: host builds are unaffected,
    # and only android_arm64 builds fail (with a clear error) when the NDK is unavailable.
    http_archive(
        name = "rules_android_ndk",
        sha256 = "d230a980e0d3a42b85d5fce2cb17ec3ac52b88d2cff5aaf86bae0f05b48adc55",
        strip_prefix = "rules_android_ndk-d5c9d46a471e8fcd80e7ec5521b78bb2df48f4e0",
        urls = ["https://github.com/bazelbuild/rules_android_ndk/archive/d5c9d46a471e8fcd80e7ec5521b78bb2df48f4e0.zip"],
    )

def native_ui_setup():
    if not native.existing_rule("bazel_skylib"):
        _bazel_skylib()
    if not native.existing_rule("skia"):
        _skia()
    if not native.existing_rule("yoga"):
        _yoga()
    if not native.existing_rule("stblib"):
        _stblib()
    if not native.existing_rule("com_google_googletest"):
        _googletest()
    if not native.existing_rule("nanosvg"):
        _nanosvg()
    if not native.existing_rule("freetype"):
        _freetype()
    if not native.existing_rule("rules_android_ndk"):
        _rules_android_ndk()
