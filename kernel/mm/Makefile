obj_dir := ../../build/objs

include ../../Makefile.config

target := $(obj_dir)/mmu.o $(obj_dir)/frame.o

all: $(target)

$(obj_dir)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< 
