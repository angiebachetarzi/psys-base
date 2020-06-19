#include "display.h"
#include "string.h"
#include "cpu.h"


//background color
uint8_t color_bg = COLOR_BLACK;

//text color
uint8_t txt_color = COLOR_WHITE;

//column cursor
uint32_t ind_x = 0;

//line cursor
uint32_t ind_y = 0;

void set_cursor(uint32_t x, uint32_t y){
    uint16_t pos = y + x * MAX_C;

    outb(0x0F, 0x3D4);
    outb((uint8_t)pos, 0x3D5);
    outb(0x0E, 0x3D4);
    outb((uint8_t)(pos >> 8), 0x3D5);

    ind_x = x;
    ind_y = y;
}

uint16_t *ptr_mem(uint32_t x, uint32_t y){
    return ((uint16_t * )(DISPLAY_ADR + 2 * (x * MAX_C + y)));
}

void write_car(uint32_t x, uint32_t y, char c, uint32_t txt, uint32_t bg){
    uint16_t * adr = ptr_mem(x, y);
    uint16_t value = c | (txt << 8) | (bg << 12);
    *adr = value;
}


void switch_car(char c){
    uint32_t car = (uint32_t) c;
    if (car >= 32 && car <= 126) {
        write_car(ind_x,ind_y,c,txt_color,color_bg);
    } else {
        switch (c) {
            case '\b':
                if (ind_y != 0) {

                ind_y--;
                
                }
                break;
            case '\t':
                if (ind_y < 72) {

                ind_y += 8 - ind_y % 8;

                } else {

                ind_y = 79;

                }
                break;
            case '\n':
                ind_y = 0;
                ind_x++;
                break;
            case '\f':
                clear_screen(COLOR_WHITE, COLOR_BLACK);
                break;
            case '\r':
                ind_y = 0;
                break;
        }
    }
        if (ind_y == MAX_C - 1) {

            ind_y = 0;

            if (ind_x == MAX_L - 1) {

            scroll();

            } else {

            ind_x++;

            }
        } else {

            ind_y++;

        }

        set_cursor(ind_x,ind_y);
}

void scroll(void) {

    memmove(ptr_mem(0, 0), ptr_mem(1, 0), 2 * (MAX_C * (MAX_L - 1)));
    for(uint32_t i = 0; i < MAX_C; i++) {
        write_car((MAX_L - 1), i, 32, txt_color, color_bg);
    }
}

void clear_screen() {

  for(uint8_t x = 0; x < MAX_L; x++){
        for(uint8_t y = 0; y < MAX_C; y++){
            write_car(x, y, ' ', txt_color, color_bg);
        }
    }

}

void console_putbytes(const char *str, int32_t size){

    for(int i = 0 ; i < size; i++){
        switch_car(str[i]);
    }

}

void display_timer(char *str, int32_t size) {
  for (int32_t i = 0; i < size; i++) {
      write_car(0,MAX_C-size+i,*(str + i),txt_color,color_bg);
    }
}


