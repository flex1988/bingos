CC := gcc

target := initrd font

bin_dir := ../build/bin

all: $(target)

initrd:
	$(CC) initrd.c -o initrd 
	./initrd $(bin_dir)/init init $(bin_dir)/sh sh
	rm initrd

font:
	$(CC)  generate_font.c -o font
