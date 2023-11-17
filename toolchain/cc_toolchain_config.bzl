load("@bazel_tools//tools/build_defs/cc:action_names.bzl", "ACTION_NAMES")
load(
    "@bazel_tools//tools/cpp:cc_toolchain_config_lib.bzl",
    "feature",
    "flag_group",
    "flag_set",
    "tool_path",
)

all_link_actions = [ # NEW
    ACTION_NAMES.cpp_link_executable,
    ACTION_NAMES.cpp_link_dynamic_library,
    ACTION_NAMES.cpp_link_nodeps_dynamic_library,
]

def _impl(ctx):
    tool_paths = [
        tool_path(
            name = "gcc",
            path = "/root/opt/cross/bin/i686-elf-gcc",
        ),
        tool_path(
            name = "ld",
            path = "/root/opt/cross/bin/i686-elf-ld",
        ),
        tool_path(
            name = "ar",
            path = "/root/opt/cross/bin/i686-elf-ar",
        ),
        tool_path(
            name = "cpp",
            path = "/root/opt/cross/bin/i686-elf-cpp",
        ),
        tool_path(
            name = "gcov",
            path = "/root/opt/cross/bin/i686-elf-gcov",
        ),
        tool_path(
            name = "nm",
            path = "/root/android-toolchain/bin/aarch64-linux-android-nm",
        ),
        tool_path(
            name = "objdump",
            path = "/root/opt/cross/bin/i686-elf-objdump",
        ),
        tool_path(
            name = "strip",
            path = "/root/opt/cross/bin/i686-elf-strip",
        ),
    ]

    features = [ # NEW
        feature(
            name = "default_linker_flags",
            enabled = True,
            flag_sets = [
                flag_set(
                    actions = all_link_actions,
                    flag_groups = ([
                        flag_group(
                            flags = [
                                "-lstdc++",
                            ],
                        ),
                    ]),
                ),
            ],
        ),
    ]

    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        features = features, # NEW
        cxx_builtin_include_directories = [
        ],
        toolchain_identifier = "local",
        host_system_name = "linux",
        target_system_name = "",
        target_cpu = "i686",
        target_libc = "unknown",
        compiler = "gcc",
        abi_version = "unknown",
        abi_libc_version = "unknown",
        tool_paths = tool_paths,
    )

cc_toolchain_config = rule(
    implementation = _impl,
    attrs = {},
    provides = [CcToolchainConfigInfo],
)
