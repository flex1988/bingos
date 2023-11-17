cc_library(
  name = "common",
  srcs = glob([
      "common/*.c",
  ]),
  hdrs = glob([
      "include/lib/*.h",
      "include/kernel/*.h",
      "include/*.h"
  ]),
  copts = [
      "-m32",
      "-std=c99",
      "-nostdlib",
      "-nostdinc",
      "-fno-builtin",
      "-fno-stack-protector",
      "-nostartfiles",
      "-nodefaultlibs",
      "-Iinclude",
      "-Iinclude/libc",
  ],
  visibility = ["//visibility:public"]
)

