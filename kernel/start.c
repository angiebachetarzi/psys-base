#include "debugger.h"
#include "cpu.h"
#include "display.h"
#include "process.h"
#include "../shared/debug.h"


int fact(int n)
{
	if (n < 2)
		return 1;

	return n * fact(n-1);
}

int print_proc(){
	printf("Hello world !\n");
	printf("Hello world !\n");
	return 0;
}


void kernel_start(void)
{

	clear_screen(COLOR_WHITE, COLOR_BLACK);
	start(print_proc, 4096 ,1,"print_proc",NULL);
	while (1) {
		hlt();
	}
	return;
	
}
