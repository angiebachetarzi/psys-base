#include "process.h"
#include "queue.h"
#include "mem.h"
#include "debug.h"

extern void exit_proc();

proc_table process_table;

/*
* Switch to next process
* next: the next process
* state: state of the current process
*/
void switch_proc(process * next, uint8_t state) {

    process * curr = process_table.current_process;
    //next process becomes active
    next -> state = STATE_ACTIVE;
    //update current process
    process_table.current_process = next;

    if (state == STATE_READY) {

        //add old process to the ready list
        curr -> state = STATE_READY;
        queue_add(curr, &process_table.ready_process, process, scheduling, priority);

    } else {
        if (state == STATE_FREE) {

        //add old process to free list
        curr -> state = STATE_FREE;
        queue_add(curr, &process_table.free_process, process, scheduling, priority);

        }
        else {

            curr -> state = state;

        }
    }

    //switch context to next proc
    context_switch(&curr -> context ,
                        &next -> context);
}

/*
* Get next process in ready list and change to that process
* curr_state: state of current process
*/
void next_process(uint8_t curr_state) {

    process * next_proc = queue_out(&process_table.ready_process, process, scheduling);
    switch_proc(next_proc, curr_state);

}


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
    if (process_table.current_process -> priority < new_proc -> priority) {

        switch_proc(new_proc, STATE_READY);

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

    next_process(STATE_ZOMBIE);

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

int getpid() {
    return (process_table.current_process) -> pid;
}

int getprio(int pid) {
    return (process_table.table_process[pid - 1]).priority;
}

int chprio(int pid, int newprio) {
    //get old prio
    int32_t tmp_prio = (process_table.table_process[pid - 1]).priority;
    //update prio
    (process_table.table_process[pid - 1]).priority = newprio;

    return tmp_prio;

}

int waitpid(int pid, int *retvalp) {

    if (pid < 1) {
        return ERROR_PID;
    }

    if (queue_empty(&(process_table.current_process) -> head_children)
        || (process_table.table_process[pid - 1]).parent -> pid != (uint32_t) getpid()) {
        return ERROR_CHILD;
    }

    //if father is zombie and pid is not of the first process
    if (pid > 0 && (process_table.table_process[pid - 1]).state == STATE_ZOMBIE) {
        //check that return value is not null
        if (retvalp != NULL) {
            //update return value with the one of the father
            *retvalp = (process_table.table_process[pid - 1]).return_value;
        }
        return pid;
    }
    
    //update waitpid value
    (process_table.current_process) -> wait_pid_val = pid;

    //move on to next process
    next_process(STATE_CHILD_WAIT_BLOCK);

    if(retvalp != NULL){
        *retvalp = (process_table.table_process[pid - 1]).return_value;
    }

    return (process_table.current_process) -> wait_pid_val;
}