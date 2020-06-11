#ifndef CTX_SWT_H
#define CTX_SWT_H

    #include "../shared/stdint.h"

    typedef struct {
        uint32_t ebx;
        uint32_t esp;
        uint32_t ebp;
        uint32_t esi;
        uint32_t edi;
    } context;

    //switching context
    void context_switch(context * previous_context, context * next_context);

#endif