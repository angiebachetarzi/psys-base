#include "stdint.h"
#include "display.h"
#include "clock.h"
#include "stdio.h"
#include "cpu.h"
#include "segment.h"
#include "process.h"
#include "queue.h"


uint32_t tic = 0;


void tic_PIT(void) {
  outb(0x20, 0x20);
  tic++;
  if (current_clock() % (CLOCKFREQ/SCHEDFREQ) == 0) {
    //wakey wakey little sleepy process
    process * tmp;
    queue_for_each(tmp, asleep_process_list(), process, scheduling) {
      //if process is ready to be awaken
      if (current_clock() >= tmp -> sleep_time) {
        queue_del(tmp, scheduling);
        //add the process to the ready list
        tmp -> state = STATE_READY;
        queue_add(tmp, ready_process_list(), process, scheduling, priority);
      }
    } 
    next_process(STATE_READY);     
  }

}

void clock_settings(unsigned long * quartz, unsigned long *ticks) {
  *quartz = QUARTZ;
  *ticks = QUARTZ/CLOCKFREQ;
}

uint32_t current_clock() {
    return tic;
}

void wait_clock(uint32_t wakeup_time) {

  //get current process
  process * curr = current_process();
  //now current goes to sleep until wakeup time
  curr -> state = STATE_ASLEEP;
  curr -> sleep_time = wakeup_time;
  //add current to sleep list
  queue_add(curr, asleep_process_list(), process, scheduling, sleep_time);
  //move on to next process
  next_process(STATE_ASLEEP);

}

void init_traitant_IT(void (*traitant)(void), uint8_t n_interrupt) {
    uint32_t fist_word;
    uint32_t second_word;
    uint32_t * address;

    fist_word = (KERNEL_CS << 16) | ((uint32_t)traitant & 0x0000FFFF);
    if (n_interrupt == 49) {
        second_word = ((uint32_t)traitant & 0xFFFF0000) | (0xEE00);
    } else {
        second_word = ((uint32_t)traitant & 0xFFFF0000) | (0x8E00);
    }
    
    address = (uint32_t*)(0x1000 + 8 * n_interrupt);
    
    (*address) = fist_word;
    address = address + 1;
    (*address) = second_word;
}

void demasq_irq(uint32_t n_irq) {
  unsigned char cur_mask = inb(0x21);
  unsigned char mask = 0xFF ^ (0x01 << n_irq);
  mask = cur_mask & mask;
  outb(mask, 0x21);
}

void set_freq() {
  outb(0x34, 0x43);
  outb((QUARTZ / CLOCKFREQ) % 256, 0x40);
  outb((QUARTZ / CLOCKFREQ) >> 8, 0x40);
}

