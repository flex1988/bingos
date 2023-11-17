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

cc_library(
  name = "c",
  srcs = glob([
      "libc/*.c"
  ]),
  hdrs = glob([
      "include/libc/*.h",
      "include/hal/*.h"
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
  deps = [ ":common" ] 
)

cc_library(
  name = "kernel",
  srcs = glob([
      "hal/*.c",
      "fs/*.c",
      "kernel/*.c",
      "net/*.c",
      "drivers/*.c",
  ]),
  hdrs = glob([
      "include/*.h",
      "include/*/*.h"
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
      "-Iinclude/fs",
      "-Iinclude/kernel",
      "-Iinclude/libc",
  ],
)