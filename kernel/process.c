#include "process.h"
#include "../shared/queue.h"
#include "mem.h"

proc_table process_table;


int start(int (*pt_func)(void*), unsigned long ssize, int prio, const char *name, void *arg) {

    process * new_proc = queue_out(&process_table.free_process, process, scheduling);

    new_proc -> name = name;
    new_proc -> size = ssize;
    new_proc -> pt_func = pt_func;
    new_proc -> priority = prio;
    new_proc -> arg = arg;
    new_proc -> parent = process_table.current_process;

    //new process list for the child
    new_proc -> stack = mem_alloc(ssize);
    uint32_t size = ssize / sizeof(uint32_t);
            
    new_proc -> context.esp = (uint32_t) &new_proc -> stack[size - 3];

    new_proc -> stack[size - 1] = (uint32_t) arg;
    new_proc -> stack[size - 3] = (uint32_t) pt_func;

    //if the parent process has a lesser priority than it's child
    if ((new_proc -> parent) -> priority < new_proc -> priority) {

        new_proc -> state = STATE_ACTIVE;

        //change state of parent to ready
        (new_proc -> parent) -> state = STATE_READY;
        //add the parent to the list of ready processes
        queue_add(new_proc -> parent,&process_table.ready_process,process,scheduling,priority);

        //switch context from parent to child
        context_switch(&(new_proc -> parent) -> context ,
                        &new_proc -> context);

    } else {

        new_proc -> state = STATE_READY;
        queue_add(new_proc,&process_table.ready_process,process,scheduling,priority);

    }
    return new_proc -> pid;
}

int first_process(int (*pt_func)(void*), unsigned long ssize, const char *name) {

    //init the free and ready lists of processes
    INIT_LIST_HEAD(&process_table.free_process);
    INIT_LIST_HEAD(&process_table.ready_process);

    //init static table of processes
    //setting their pids and their states at free
    //NB: starting at 1 because the first one is for the first process, duh!
    for (int i = 1; i < N_PROC; i++) {
        (process_table.table_process[i]).pid = i + 1;
        (process_table.table_process[i]).state = STATE_FREE;
        queue_add(&(process_table.table_process[i]), &process_table.free_process, process, scheduling, priority);
    }

    //taking care of the first process
    process * first = &process_table.table_process[0];

    first -> pid = 1;
    first -> name = name;
    first -> size = ssize;
    first -> pt_func = pt_func;
    first -> priority = 128;
    first -> arg = NULL;
    first -> parent = first;

    //init stack of first process
    first -> stack = mem_alloc(ssize);
    uint32_t size = ssize / sizeof(uint32_t);
            
    first -> context.esp = (uint32_t) &first -> stack[size - 3];

    first -> stack[size - 1] = (uint32_t) NULL;
    first -> stack[size - 3] = (uint32_t) pt_func;

    //init current process of process table
    process_table.current_process = first;

    //state of first process is active
    first -> state = STATE_ACTIVE;
    //switching context from random one to first
    context random_ctx;
    context_switch(&random_ctx, &first -> context);

    return 0;
}