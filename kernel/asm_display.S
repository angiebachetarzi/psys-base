#define _DISPLAY_ASM_
#include "display.h"

.text
.globl set_cursor

set_cursor:
    enter $0,$0
    // res = 80 * lign + col
    movl $MAX_C,%eax
    mulw 8(%ebp)
    addw 12(%ebp),%ax
    movw %ax, %cx

    // port de commande
    movw $0x3D4, %dx
    movb $0x0f , %al
    outb %al, %dx

    movw $0x3D5, %dx
    movb %cl , %al
    outb %al, %dx

    movw $0x3D4, %dx
    movb $0x0e , %al
    outb %al, %dx

    movw $0x3D5, %dx
    movb %ch , %al
    outb %al, %dx

    leave
    ret
