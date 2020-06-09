#include "display.h"
#include "string.h"


//background color
uint8_t color_bg = COLOR_BLACK;

//text color
uint8_t txt_color = COLOR_WHITE;

//column cursor
uint32_t ind_x = 0;

//line cursor
uint32_t ind_y = 0;

//display table
uint16_t * buffer = (uint16_t*) DISPLAY_ADR;

//init buffer
uint16_t init_display(unsigned char ch, uint8_t color_text, uint8_t color_bg) 
{
  uint16_t ax = 0;
  uint8_t ah = 0, al = 0;

  ah = color_bg;
  ah = ah << 4;
  ah += color_text;
  ax = ah;
  ax = ah << 8;
  al = ch;
  ax += al;

  return ax;
}

//write a car on screen
void write_car(char c){
    switch (c) {

    case '\n':

        ind_x += 1;
        ind_y = 0;

    break;
    
    case '\t':

        ind_y = ind_y - (ind_y % 4) + 4;

        if (ind_y > MAX_C) {
            ind_y -= MAX_C;
            ind_x += 1;
        }

    break;

    case '\f':

        ind_x = 0;
        ind_y = 0;

    break;

    case '\b':

        if(ind_y != 0) {
            ind_y -= 1;
        } 

    break;
    case '\r':

        ind_y = 0;

    break;
    
    default:

        buffer[ind_x * MAX_C + ind_y] = init_display(c, txt_color, color_bg);
        if (ind_y == MAX_C - 1) {

            ind_x += 1;
            ind_y = 0;

        } else {
            ind_y += 1;
        }

    }

    if (ind_x == MAX_L) {
        memmove((void *)DISPLAY_ADR,(void *)(DISPLAY_ADR+ MAX_C * 2),MAX_C * 2 * (MAX_L -1));
        ind_x = 24;
        uint32_t i;
        for(i = (MAX_L -1) * MAX_C; i < BUFFER_SIZE; i++){
            buffer[i] = init_display(0, txt_color, color_bg);
        }
    }
    set_cursor(ind_x, ind_y);
}

void clear_screen(uint8_t fore_COLOR, uint8_t color_bg)
{
  for(uint32_t i = 0; i < BUFFER_SIZE; i++){
    buffer[i] = init_display(0, fore_COLOR, color_bg);
  }
}

void console_putbytes(char *chaine, int32_t taille){
    for(int i = 0 ; i < taille; i++){
        write_car(chaine[i]);
    }
}
