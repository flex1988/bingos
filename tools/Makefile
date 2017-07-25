CC := gcc

target := initrd

all: $(target)

initrd:
	$(CC) initrd.c -o initrd 
	./initrd hello hello
	rm initrd
