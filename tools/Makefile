CC := gcc

target := initrd

bin_dir := ../build/bin

all: $(target)

initrd:
	$(CC) initrd.c -o initrd 
	./initrd $(bin_dir)/init init
	rm initrd
