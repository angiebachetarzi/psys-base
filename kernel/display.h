#ifndef DISPLAY_H
#define DISPLAY_H
    #define MAX_C 80
    #define MAX_L 25

    #ifndef DISPLAY_ASM
        #include "stdint.h"
        #define DISPLAY_ADR 0xB8000

        /*
        * clear_screen : function to clear the console screen
        */
        void clear_screen();

        /*
        * console_putbytes : write on console screen
        * str : string to display
        * size : sizeof string
        */
        void console_putbytes(const char *str, int32_t size);

        /*
        * display_timer: display the timer up and to the left
        * string: the actual timer
        * size size of string
        */
        void display_timer(char * string, int32_t size);

        /*
        * set_cursor : change position of cursor on console
        * l: new line
        * c: new column
        */
        void set_cursor(uint32_t l, uint32_t c);

        /*
        * switch_car: take care of display of each car
        * special cars included
        * c: car to display
        */
        void switch_car(char c);

        /*
        * scroll: Well it's for scrolling, duh
        */
        void scroll(void);

        /*
        * ptr_mem: returns a ptr to memory corresponding to x and y
        * x: line
        * y: column
        */
        uint16_t *ptr_mem(uint32_t x, uint32_t y);

        /*
        * write_car: writes car on screen
        * x: line
        * y: column
        * c: car
        * txt: text color
        * bg: background color
        */
        void write_car(uint32_t x, uint32_t y, char c, uint32_t txt, uint32_t bg);

        /*
        * set_cursor: sets cursor at position x and y
        * x: line
        * y: column
        */
        void set_cursor(uint32_t x, uint32_t y);

        enum display_color {
            COLOR_BLACK,
            COLOR_BLUE,
            COLOR_GREEN,
            COLOR_CYAN,
            COLOR_RED,
            COLOR_MAGENTA,
            COLOR_BROWN,
            COLOR_GREY,
            COLOR_DARK_GREY,
            COLOR_BRIGHT_BLUE,
            COLOR_BRIGHT_GREEN,
            COLOR_BRIGHT_CYAN,
            COLOR_BRIGHT_RED,
            COLOR_BRIGHT_MAGENTA,
            COLOR_YELLOW,
            COLOR_WHITE,
        };
    #endif
#endif
