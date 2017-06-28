obj_dir := ../../build/objs

target := $(obj_dir)/main.o

all: $(target)

$(obj_dir)/%.o: %.c
	$(CC) -I../include -c -o $@ $<

