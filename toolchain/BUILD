package(default_visibility = ["//visibility:public"])

cc_toolchain_suite(
    name = "clang_suite",
    toolchains = {
        "i686": ":i686_toolchain",
    },
)

filegroup(name = "empty")

cc_toolchain(
    name = "i686_toolchain",
    toolchain_identifier = "i686-toolchain",
    toolchain_config = ":i686_toolchain_config",
    all_files = ":empty",
    compiler_files = ":empty",
    dwp_files = ":empty",
    linker_files = ":empty",
    objcopy_files = ":empty",
    strip_files = ":empty",
    supports_param_files = 0,
)

load(":cc_toolchain_config.bzl", "cc_toolchain_config")
cc_toolchain_config(name = "i686_toolchain_config")
