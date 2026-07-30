load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def _skia():
    http_archive(
        name = "skia",
        urls = ["https://github.com/google/skia/archive/fdbe14582b177785e0e77c938277ef332d45636f.tar.gz"],
        sha256 = "801f902c7d867783950fd2c12f7416b55f06a8129e2247b245389b6d3c3b158a",
        strip_prefix = "skia-fdbe14582b177785e0e77c938277ef332d45636f",
        build_file = "//third_party/skia:BUILD.bazel",
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
        build_file = "//third_party/yoga:BUILD.bazel",
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
        ],
        sha256 = "13a99ad430e930907f5611325ec384168a958bf7610e63e60e2fd8e7b7379610",
        strip_prefix = "stb-b42009b3b9d4ca35bc703f5310eedc74f584be58",
        url = "https://github.com/nothings/stb/archive/b42009b3b9d4ca35bc703f5310eedc74f584be58.tar.gz",
    )

def _nanosvg():
    http_archive(
        name = "nanosvg",
        urls = ["https://github.com/oxsource/nanovg/archive/refs/tags/v1.0.0.tar.gz"],
        sha256 = "91882cb9ea0f6cb75dfbe3a0d272292b640d237e8892d7252b08e38feb930e6f",
        strip_prefix = "nanovg-1.0.0",
        build_file = "//third_party/nanosvg:BUILD.bazel",
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
