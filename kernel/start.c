#include "debugger.h"
#include "cpu.h"
#include "display.h"
#include "process.h"
#include "../shared/debug.h"
#include "mem.h"


int test2(void * arg){
	if((int) arg == 6){
		printf("Watashi wa test2 desu\n");
	}else{
		printf("Moo ii yo\n");
	}
	return 666;
}

int test1(){
	printf("Watashi wa number one desu\n");
	int pid = start(test2,4096,130,"test",(void *) 6);
	printf("Watashi no PID wa %d \n",pid);
	return 0;
}



void kernel_start(void)
{

	clear_screen(COLOR_WHITE, COLOR_BLACK);
	first_process(test1, 4096, "help");

	while(1) {
		hlt();
	}

	return;
}
