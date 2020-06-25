#include "display.h"
#include "string.h"
#include "cpu.h"
#include "kbd.h"
#include "process.h"
#include "queue.h"


//background color
uint8_t color_bg = COLOR_WHITE;

//text color
uint8_t txt_color = COLOR_RED;

//column cursor
uint32_t ind_x = 0;

//line cursor
uint32_t ind_y = 0;

//display (used for tests)
uint32_t terminal_echo = 1;

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

int check_scroll(char c) {
    return (ind_x >= (MAX_L - 1) && (ind_y >= (MAX_C - 1) || c == 10));
}


void switch_car(char c){
    if(((c >= 0) && (c <= 31)) || (c == 127)){
        switch(c){
            case 8:
                if (ind_y > 0)
                {
                    set_cursor(ind_x, (ind_y - 1));
                }
                break;
            case 9:
                while(1){
                    set_cursor(ind_x, (ind_y + 1));
                    if (ind_y == 0) {
                        if (check_scroll(c))
                        {
                            scroll();
                            set_cursor(ind_x, ind_y);
                        }
                        set_cursor((ind_x + 1), ind_y);
                        break;
                    } else if ((ind_y % 8) == 0) {
                        break;
                    }
                }
                break;
            case 10:
                if (check_scroll(c)){
                    scroll();
                    set_cursor(ind_x, 0);
                } else {
                    set_cursor((ind_x + 1), 0);
                }
                break;
            case 12:
                clear_screen(txt_color, color_bg);
                break;
            case 13:
                set_cursor(ind_x, 0);
                break;
            default:
                break;
        }
    }
    else{
        write_car(ind_x, ind_y, c, txt_color, color_bg);
        if(ind_y < (MAX_C - 1)){
            set_cursor(ind_x, (ind_y + 1));
        }
        else if(ind_x < (MAX_L - 1)){
            set_cursor((ind_x + 1), 0);
        }
        else{
            if (check_scroll(c)) {
                scroll();
            }
            set_cursor((ind_x), 0);
        }
    }
}

void scroll(void) {

    memmove(ptr_mem(0, 0), ptr_mem(1, 0), 2 * MAX_C * MAX_L);
    
    for(uint32_t i = 0; i < MAX_C; i++) {
        write_car(ind_x, i, ' ', txt_color, color_bg);
    }
}

void clear_screen() {

  for(uint8_t x = 0; x < MAX_L; x++){
        for(uint8_t y = 0; y < MAX_C; y++){
            write_car(x, y, ' ', txt_color, color_bg);
        }
    }
    set_cursor(0,0);

}

void console_putbytes(const char *str, int32_t size){

    for(int i = 0 ; i < size; i++){
        switch_car(str[i]);
    }

}

void cons_write(const char *str, unsigned long size){
    console_putbytes(str,size);
}

void cons_echo(int on){
    terminal_echo = on;
}

int cons_read(void) {
    return 0;
}

void init_display() {

    printf("\f");
    printf("...............................................................................\n");
    printf("    .S_SSSs    S.       .S_SSSs     .S   .S_sSSs      sSSs_sSSs      sSSs  \n");
    printf("   .SS~SSSSS   SS.     .SS~SSSSS   .SS  .SS~YSSbb    ddSSP~YSSbb    ddSSP  \n");
    printf("   SSS   SSSS  SSS     SSS   SSSS  SSS  SSS   `Sbb  dSS'     `Sbb  dSS'    \n");
    printf("   SSS    SSS  SSS     SSS    SSS  SSS  SSS    SSS  SSS       SSS  SS|     \n");
    printf("   SSS SSSSSS  S&S     SSS SSSSPP  S&S  SSS    S&S  S&S       S&S  S&S     \n");
    printf("   S&S  SSSSS  S&S     S&S  SSSY   S&S  S&S    S&S  S&S       S&S  Y&Ss    \n");
    printf("   S&S    S&S  S&S     S&S    S&S  S&S  S&S    S&S  S&S       S&S  `S&&S   \n");
    printf("   S&S    S&S  S&S     S&S    S&S  S&S  S&S    S&S  S&S       S&S    `S*S  \n");
    printf("   S*S    S&S  S*b     S*S    S&S  S*S  S*S    S*S  S*b       d*S     l*S  \n");
    printf("   S*S    S*S  S*S.    S*S    S*S  S*S  S*S    S*S  S*S.     .S*S    .S*P  \n");
    printf("   S*S    S*S   SSSbs  S*S SSSSP   S*S  S*S    S*S   SSSbs_sdSSS   sSS*S   \n");
    printf("   SSS    S*S    YSSP  S*S  SSY    S*S  S*S    SSS    YSSP~YSSY    YSS'    \n");
    printf("          SP           SP          SP   SP                                 \n");
    printf("          Y            Y           Y    Y                                  \n");
    printf("...............................................................................\n\n");
}


