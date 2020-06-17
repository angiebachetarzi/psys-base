#include "cpu.h"
#include "display.h"
#include "process.h"
#include "stdio.h"
#include "clock.h"

extern int test_run(int n);

int test3(void * arg){
	printf("Watashi wa test3 desu. Soshite, whatashi no arg wa %d!\n",(int) arg);
	return 5;
}

int test2(void * arg){
	if((int) arg == 6){
		printf("Watashi wa test2 desu\n");
		start(test3,4096,130,"test3",(void *) 5);
	}else{
		printf("Moo ii yo\n");
	}
	return 666;
}

int test1(){
	printf("Watashi wa test1 desu\n");
	test_run(1);
	int pid = start(test2,4096,135,"test2",(void *) 6);
	printf("Test1 no PID wa %d \n",pid);
	while(1) {
		hlt();
	}
	return 0;
}

void kernel_start(void)
{
	unsigned long quartz;
	unsigned long ticks;

	clock_settings(&quartz,&ticks);
	demasquage_IRQ();
	init_traitant_IT32(&traitant_IT_32);

	clear_screen(COLOR_WHITE, COLOR_BLACK);

	first_process(test1, 4096, "test1");
	
	sti();
}


