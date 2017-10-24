CC := gcc

bin_dir := ../build/bin

target := $(bin_dir)/init

all: $(target)

$(bin_dir)/%: %.c
	$(CC) $< -o $@
