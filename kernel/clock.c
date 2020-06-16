#include "stdint.h"
#include "display.h"
#include "clock.h"
#include "stdio.h"
#include "cpu.h"
#include "segment.h"


uint32_t tic = 0;
uint32_t counter = 0;
uint32_t timing = BASE_FREQ / CLOCKFREQ;


void tic_PIT(void) {
  outb(0x20, 0x20);
  if (tic % CLOCKFREQ == 0) {
    uint32_t h, m, s;
    char r[MAX_C];

    tic = 0;
    s = counter % 60;
    m = ((counter - s) % 3600) / 60;
    h = ((counter - m)) / 3600;
    uint32_t size = sprintf(r, "UPTIME : %02d:%02d:%02d\n", h, m, s);
    display_timer(r, size-1);
    counter++;
  }
  tic++;
}

void clock_settings(unsigned long * quartz, unsigned long *ticks) {
    outb(0x34,0x43);
    outb(timing,0x40);
    outb(timing/256,0x40);
    *quartz = BASE_FREQ;
    *ticks = timing;
}

uint32_t current_clock() {
    return tic;
}

void wait_clock(uint32_t wakeup) {
    printf("%d\n",wakeup);
}

void init_traitant_IT32(void (*traitant)(void))
{
    uint32_t *ptr = (uint32_t*) (0x1000 + 32 * 4 * 2);
    uint32_t low = (uint32_t)traitant&0x0000FFFF;
    uint32_t high = (uint32_t)traitant&0xFFFF0000;
    *ptr = KERNEL_CS << 16 | low; 
    *(ptr+1) = high | 0x8E00;
}

void demasquage_IRQ() {

   uint16_t tab = inb(0x21);
   tab = tab&0b11111110;
   outb(tab,0x21);
}

