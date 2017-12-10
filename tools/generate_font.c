#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    FILE *file = fopen(argv[1], "r");
    int height = atoi(argv[2]);
    int width = atoi(argv[3]);
    char *line = NULL;

    size_t size = 0;
    int nread;

    size_t offset = 0;
    char v = 0;

    size_t wbuf_size = 4096;

    char *wbuf = malloc(wbuf_size);

    printf("typedef struct { uint8_t data[%d] } font_mono_t;\n", height * width / 8);
    printf("font_mono_t fonts_data[] = {\n");

    while ((nread = getline(&line, &size, file)) != -1) {
        if (offset + width > wbuf_size) {
            wbuf_size += 4096;
            wbuf = realloc(wbuf, wbuf_size);
        }

        if (line[0] == 'h')
            continue;
        if (line[0] == 'g')
            continue;
        if (line[0] == '\r')
            continue;
        if (line[0] == '\n')
            continue;

        memcpy(wbuf + offset, line, width);

        offset += width;
    }

    int i = 0;
    while (i < offset) {
        uint8_t ret = 0;
        int j;
        for (j = 0; j < 8; j++) {
            if (wbuf[i++] == '*')
                ret |= 1 << j;
        }

        if (i == offset)
            printf("0x%x", ret);
        else
            printf("0x%x, ", ret);
    }

    printf("};");

    return 0;
}
