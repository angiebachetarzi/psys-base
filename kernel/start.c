#include "debugger.h"
#include "cpu.h"
#include "display.h"
#include "process.h"
#include "../shared/debug.h"
#include "mem.h"


int test(void * arg){
	if((int) arg == 5){
		printf("BITCH YOU DID THAT\n");
	}else{
		printf("Je veux rejoindre papa Johnny\n");
	}
	return 5;
}

int help(){
	printf("I'm dying\n");
	int pid = start(test,4096,130,"test",(void *) 5);
	printf("PID : %d \n",pid);

    while(1)
	  hlt();
	return 0;
}



void kernel_start(void)
{

	clear_screen(COLOR_WHITE, COLOR_BLACK);
	char * test = mem_alloc(8);
	printf("%s", test);
	sti();
	
}
