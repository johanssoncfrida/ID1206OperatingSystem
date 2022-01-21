#define COLUMNS 80
#define ROWS 25

char *name = "Frida Johansson";

typedef struct vga_char{
    char character;
    char colors;
}vga_char;

void myos(void){
    vga_char *vga = (vga_char*)0xb8000; //the VGA terminal mapped memory

    for(int i = 0; i < COLUMNS*ROWS; i++){
        vga[i].character = ' ';
        vga[i].colors = 0x0f;
    }

    for(int i = 0; name[i] != '\0'; i++){
        vga[600+i].character = name[i];
        vga[600+i].colors = 0x0f;
    }
    return;
}