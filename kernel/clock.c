#include "stdint.h"
#include "display.h"
#include "clock.h"
#include "stdio.h"
#include "cpu.h"
#include "segment.h"


uint32_t tic = 0;
uint32_t counter = 0;


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
  *quartz = QUARTZ;
  *ticks = QUARTZ/CLOCKFREQ;
}

uint32_t current_clock() {
    return tic;
}

void wait_clock(uint32_t wakeup) {
  //temp
    printf("%d\n",wakeup);
}

void init_traitant(void (*traitant)(void), uint8_t n_interrupt) {
  uint32_t * ad = (uint32_t *)(INTRPT_VECT_ADD + 2 * 4 * n_interrupt);
  *ad = ((KERNEL_CS & 0xFFFF) << 16) | (((uint32_t)(traitant) & 0xFFFF));
  *(ad + 1) = INTRPT_CST_LOWER | ((uint32_t)(traitant) & 0xFFFF0000);
}

void demasq_irq(uint32_t n_irq) {
  uint8_t o;
  uint8_t mask;
  uint16_t port;
  if (n_irq < 8) {
    mask = 0xFF ^ (0x01 << n_irq);
    port = IRQ_ADD_MASTER;
  } else {
    mask = 0xFF ^ (0x01 << (n_irq - 8));
    port = IRQ_ADD_SLAVE;
  }
  o = inb(port) & mask;
  o = o & mask;
  outb(o, port);
}

void set_freq() {
  uint16_t tab = inb(0x21);
  tab = tab&0b11111110;
  outb(tab,0x21);
}

