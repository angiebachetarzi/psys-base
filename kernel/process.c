#include "process.h"
#include "queue.h"
#include "mem.h"
#include "debug.h"

extern void exit_proc();

proc_table process_table;


int start(int (*pt_func)(void*), unsigned long ssize, int prio, const char *name, void *arg) {

    process * new_proc = queue_out(&process_table.free_process, process, scheduling);

    new_proc -> name = name;
    new_proc -> size = ssize;
    new_proc -> pt_func = pt_func;
    new_proc -> priority = prio;
    new_proc -> arg = arg;
    new_proc -> parent = process_table.current_process;
    INIT_LIST_HEAD(&new_proc -> head_children);
    //add new process to children list of parent
    queue_add(new_proc, &(new_proc -> parent) -> head_children, process, nodes_children, priority);

    //new process list for the child
    new_proc -> stack = mem_alloc(ssize);
    uint32_t size = ssize / sizeof(uint32_t);
            
    new_proc -> context.esp = (uint32_t) &new_proc -> stack[size - 3];

    new_proc -> stack[size - 1] = (uint32_t) arg;
    new_proc -> stack[size - 2] = (uint32_t) exit_proc;
    new_proc -> stack[size - 3] = (uint32_t) pt_func;

    //if the parent process has a lesser priority than it's child
    if ((new_proc -> parent) -> priority < new_proc -> priority) {
        new_proc -> state = STATE_ACTIVE;

        //change state of parent to ready
        (new_proc -> parent) -> state = STATE_READY;
        //add the parent to the list of ready processes
        queue_add(new_proc -> parent,&process_table.ready_process,process,scheduling,priority);
        
        //current process becomes new process
        process_table.current_process = new_proc;

        //switch context from parent to child
        process * old = new_proc -> parent;
        context_switch(&old -> context ,
                        &new_proc -> context);

    } else {

        new_proc -> state = STATE_READY;
        queue_add(new_proc,&process_table.ready_process,process,scheduling,priority);

    }

    return new_proc -> pid;
}

void exit(int retval) {
    //assign return value to current process
    (process_table.current_process) -> return_value = retval;

    //if the parent had to wait for the child (child_wait_block state)
    if (((process_table.current_process) -> parent) -> state == STATE_CHILD_WAIT_BLOCK) {
    
        //if return value of wait_pid is < 0 for the parent
        if (((process_table.current_process) -> parent) -> wait_pid_val < 0) {

            //update waitpid value of the parent with pid of child
            ((process_table.current_process) -> parent) -> wait_pid_val = (process_table.current_process) -> pid;

        }

        //change state of parent to ready
        ((process_table.current_process) -> parent) -> state = STATE_READY;
        //add parent to the ready queue
        queue_add((process_table.current_process) -> parent, &process_table.ready_process, process, scheduling, priority);
    }

    //taking care of children
    process * tmp;
    queue_for_each(tmp, &(process_table.current_process) -> head_children, process, nodes_children) {
        //if the process is in zombie mode, add it to the free process list
        //else just wait for its child to finish
        if (tmp -> state == STATE_ZOMBIE) {

            //set state of process to free
            tmp -> state = STATE_FREE;
            //add it to the free prcess list
            queue_add(tmp, &process_table.free_process, process, scheduling, priority);

        } else {

            waitpid(tmp -> pid, NULL);

        }
    }

    //if the parent of the current process is the first ever process
    if (((process_table.current_process) -> parent) -> pid == 1) {

        //get next free process
        process * next_proc = queue_out(&process_table.ready_process, process, scheduling);

        //update state of next process
        next_proc -> state = STATE_ACTIVE;
        //update state of current
        (process_table.current_process) -> state = STATE_ZOMBIE;
        //update current process
        process_table.current_process = next_proc;

        //switch context to next process
        process * old = next_proc -> parent;
        context_switch(&old -> context ,
                        &next_proc -> context);

    } else {
        //state free
        //get next free process
        process * next_proc = queue_out(&process_table.ready_process, process, scheduling);

        //update state of next process
        next_proc -> state = STATE_ACTIVE;
        //update current process
        process_table.current_process = next_proc;
        //add parent to free process list
        queue_add(next_proc -> parent, &process_table.free_process, process, scheduling, priority);

        //switch context to next proc
        process * old = next_proc -> parent;
        context_switch(&old -> context ,
                        &next_proc -> context);

    }
    while(1);
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
    first -> priority = DEFAULT_PRIORITY;
    first -> arg = NULL;
    first -> parent = first;
    INIT_LIST_HEAD(&first -> head_children);

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

int waitpid(int pid, int *retvalp) {
    //temp in order for exit to work
    *retvalp = pid;
    return pid;
}