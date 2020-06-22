#include "cpu.h"
#include "display.h"
#include "process.h"
#include "stdio.h"
#include "clock.h"

extern int test_run(int n);

int tests(){
	printf("TESTS\n");
	printf("TEST 1\n");
	test_run(1);
	printf("TEST 2\n");
	test_run(2);
	printf("TEST 3\n");
	test_run(3);
	// printf("TEST 4\n");
	// test_run(4);
	printf("TEST 5\n");
	test_run(5);
	printf("TEST 6\n");
	test_run(6);
	// printf("TEST 7\n");
	// test_run(7);
	// printf("TEST 8\n");
	// test_run(8); //not work
	// printf("TEST 9\n");
	// test_run(9); //not work
	printf("TEST 10\n");
	test_run(10);
	printf("TEST 11\n");
	test_run(11);
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


