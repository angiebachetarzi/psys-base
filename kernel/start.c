#include "cpu.h"
#include "display.h"
#include "process.h"
#include "stdio.h"
#include "clock.h"

extern int test_run(int n);

int tests(){
	printf("TESTS\n");
	for(int i = 1; i <=20; i ++) {
		test_run(i);
	}
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

	init_display();
	
	start(tests, 4096, 128, "tests", NULL);
}


