#include "cpu.h"
#include "display.h"
#include "process.h"
#include "stdio.h"
#include "clock.h"

extern int test_run(int n);

int tests(){
	printf("TESTS\n");
	// test_run(1);
	// test_run(2);
	// test_run(3);
	// test_run(4);
	// test_run(5);
	// test_run(6);
	// test_run(7);
	// test_run(8);
	// test_run(9);
	test_run(10);
	test_run(11);
	test_run(12);
	test_run(13);
	test_run(14);
	test_run(15);
	test_run(16);
	test_run(17);
	while(1) {
		hlt();
	}
	return 0;
}

void kernel_start(void)
{
	
	set_freq();
	init_traitant_IT(traitant_IT_32, 32);
	demasq_irq(0);
	
	clear_screen(COLOR_WHITE, COLOR_BLACK);
	
	start(tests, 4096, 128, "tests", NULL);
}


