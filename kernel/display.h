#ifndef _DISPLAY_H_
#define _DISPLAY_H_
    #define MAX_C 80
    #define MAX_L 25

    #ifndef _DISPLAY_ASM_
        #include "../shared/stdint.h"
        #define DISPLAY_ADR 0xB8000
    
        #define DISPLAY_ADR 0xB8000
        #define DISPLAY_TAB_NB 4
        #define BUFFER_SIZE MAX_C * MAX_L

        /*
        * clear_screen : function to clear the console screen
        * @param txt_COLOR : text color
        * @param bg_COLOR : background color
        */
        void clear_screen( uint8_t txt_COLOR, uint8_t bg_COLOR);

        /*
        * console_putbytes : write on console screen
        * @param str : string to display
        * @param size : sizeof string
        */
        void console_putbytes(char *str, int32_t size);

        /*
        * set_cursor : change position of cursor on console
        * @param l: new line
        * @param c: new column
        */
        void set_cursor(uint32_t l, uint32_t c);

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
