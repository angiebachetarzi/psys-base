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
	printf("TEST 8\n");
	test_run(8);
	printf("TEST 9\n");
	test_run(9);
	printf("TEST 10\n");
	test_run(10);
	printf("TEST 11\n");
	test_run(11);
	printf("TEST 12\n");
	test_run(12);
	// printf("TEST 13\n");
	// test_run(13); //not work
	// printf("TEST 14\n");
	// test_run(14); //not work
	// printf("TEST 15\n");
	// test_run(15); //not work
	// printf("TEST 16\n");
	// test_run(16); //not work
	// printf("TEST 17\n");
	// test_run(17); //not work
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


