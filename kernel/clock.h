#ifndef CLOCK_H
#define CLOCK_H

#define CLOCKFREQ 50
#define BASE_FREQ 0x1234DD 

void tic_PIT(void);

void clock_settings(unsigned long * quartz, unsigned long *ticks);

uint32_t current_clock();

void wait_clock(uint32_t wakeup);

void init_traitant_IT32(void (*traitant) (void));

void demasquage_IRQ();

#endif