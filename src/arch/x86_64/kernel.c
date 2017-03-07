static int vga_width = 80;
static int vga_height = 25;

void kernel_main() {
    uint16_t* buffer = (uint16_t*)0xb8000;
    int i,j;
    for(i<vga_width;i>=0;i--){
        for(j<vga_height;j>=0;j--){
            buffer[i][j] = 4<<12|123;
        }
    }

    while (1)
        ;
}
