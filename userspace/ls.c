#include <stdio.h>

int main(int argc, char **argv) {
    println("argc %d",argc);
    for (int i = 0; i < argc; i++) println(argv[i]);
    return 0;
}
