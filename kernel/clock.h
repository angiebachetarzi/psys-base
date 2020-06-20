#ifndef CLOCK_H
#define CLOCK_H

#define CLOCKFREQ 100
#define QUARTZ 0x1234DD
#define SCHEDFREQ 50

void tic_PIT(void);

void clock_settings(unsigned long * quartz, unsigned long *ticks);

uint32_t current_clock();

void wait_clock(uint32_t wakeup);

void init_traitant_IT(void (*traitant) (void), uint8_t n_interrupt);

void set_freq();

void demasq_irq(uint32_t n_irq);

void traitant_IT_32(void);

#endif