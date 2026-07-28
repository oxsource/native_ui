load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def _skia():
    http_archive(
        name = "skia",
        sha256 = "d9d5558db7006a0bf0ab61e9428c467e47932f6d260eaaba058686a17b1a203e",
        urls = ["https://github.com/google/skia/archive/abc1234567.tar.gz"],
        strip_prefix = "skia-abc1234567",
        build_file = "//third_party/skia:BUILD.bazel",
    )

def _caflex():
    http_archive(
        name = "caflex",
        urls = ["https://github.com/caiof/caflex/archive/refs/tags/v0.1.0.tar.gz"],
        sha256 = "0000000000000000000000000000000000000000000000000000000000000000",
        strip_prefix = "caflex-<commit>",
        build_file = "//third_party/caflex:BUILD.bazel",
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
    if not native.existing_rule("caflex"):
        _caflex()
    if not native.existing_rule("com_google_googletest"):
        _googletest()
