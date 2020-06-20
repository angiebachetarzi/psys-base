#ifndef CLOCK_H
#define CLOCK_H

#define CLOCKFREQ 100
#define QUARTZ 0x1234DD
#define IRQ_ADD_MASTER 0x21
#define IRQ_ADD_SLAVE 0xA1
#define INTRPT_VECT_ADD 0x1000
#define INTRPT_CST_LOWER 0x8E00

void tic_PIT(void);

void clock_settings(unsigned long * quartz, unsigned long *ticks);

uint32_t current_clock();

void wait_clock(uint32_t wakeup);

void init_traitant(void (*traitant) (void), uint8_t n_interrupt);

void set_freq();

void demasq_irq(uint32_t n_irq);

void traitant_IT_32(void);

#endif