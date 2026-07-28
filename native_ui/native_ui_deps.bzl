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
    http_archive(
        name = "yoga",
        urls = ["https://github.com/react/yoga/archive/refs/tags/v3.2.1.tar.gz"],
        sha256 = "86b399ac31fd820d8ffa823c3fae31bb690b6fc45301b2a8a966c09b5a088b55",
        strip_prefix = "yoga-3.2.1",
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

def native_ui_setup():
    if not native.existing_rule("bazel_skylib"):
        _bazel_skylib()
    if not native.existing_rule("skia"):
        _skia()
    if not native.existing_rule("yoga"):
        _yoga()
    if not native.existing_rule("com_google_googletest"):
        _googletest()
