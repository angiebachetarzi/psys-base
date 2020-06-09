#include "debugger.h"
#include "cpu.h"
#include "display.h"
#include "../shared/debug.h" 
#include "test.c"

int fact(int n)
{
	if (n < 2)
		return 1;

	return n * fact(n-1);
}


void kernel_start(void)
{

	clear_screen(COLOR_WHITE, COLOR_BLACK);
	printf("HELP ME!!");

	test_run(1);


	while (1) {
		hlt();
	}
	return;
	
}
