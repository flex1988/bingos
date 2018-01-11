CC := gcc

target := initrd

bin_dir := ../build/bin

all: $(target)

initrd:
	$(CC) initrd.c -o initrd 
	./initrd $(bin_dir)/init init $(bin_dir)/sh sh $(bin_dir)/helloworld helloworld $(bin_dir)/ls ls $(bin_dir)/date date
	rm initrd

font:
	$(CC)  generate_font.c -o font
