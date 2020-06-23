#include "process.h"
#include "queue.h"
#include "mem.h"
#include "debug.h"
#include "msg_queue.h"

extern void exit_proc();

proc_table process_table;

void idle(void) {
   
    __asm__ __volatile__ ("sti");
    __asm__ __volatile__ ("hlt");
    __asm__ __volatile__ ("cli");
}

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

        if (curr -> stack != NULL) {

            mem_free(curr -> stack, curr -> size + 4096);
            queue_del(curr, nodes_children);
            
        }
            
        queue_add(curr, &process_table.free_process, process, scheduling, priority);

        } else {

            curr -> state = state;

        }
    }

    //switch context to next proc
    context_switch(&curr -> context , &next -> context);

}


void next_process(uint8_t curr_state) {

    //get first available process
    process * next_proc = queue_top(&process_table.ready_process, process, scheduling);

    if (next_proc != NULL || curr_state != STATE_READY) {

        //if process is null, that means the kernel is in sleep mode
        //get next process and do nothing (idle)
        while (next_proc == NULL) {
            process_table.kernel_state = STATE_KERNEL_SLEEPING;
            next_proc = queue_top(&process_table.ready_process, process, scheduling);
            idle();
        }

        //if we find a not null process, that means the kernel is in run mode
        process_table.kernel_state = STATE_KERNEL_RUNNING;

        //if the priority of the available process is higher than the current one
        //switch processes
        if (next_proc -> priority >= (process_table.current_process) -> priority
            || curr_state != STATE_READY) {
                
                next_proc = queue_out(&process_table.ready_process, process, scheduling);
                switch_proc(next_proc, curr_state);
        }

    }
}


int start(int (*pt_func)(void*), unsigned long ssize, int prio, const char *name, void *arg) {

    if (ssize > INT32_MAX
        || prio < 1
        || prio > MAX_PRIORITY) {

        return -1;
        
    }

    //if the first process of the table is 1
    //that means that the process to be started is not the first
    if ((process_table.table_process[0]).pid == 1) {

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
        new_proc -> stack = mem_alloc(ssize + 4096);
        uint32_t size = (ssize + 4096) / sizeof(uint32_t);
                
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

    } else {
        return first_process(pt_func, ssize, name);
    }

    return -1;
    
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

            //change state of parent to ready
            ((process_table.current_process) -> parent) -> state = STATE_READY;
            //add parent to the ready queue
            queue_add((process_table.current_process) -> parent, &process_table.ready_process, process, scheduling, priority);

        }
        else if ((process_table.current_process) -> pid == (uint32_t) ((process_table.current_process) -> parent) -> wait_pid_val) {

             //change state of parent to ready
            ((process_table.current_process) -> parent) -> state = STATE_READY;
            //add parent to the ready queue
            queue_add((process_table.current_process) -> parent, &process_table.ready_process, process, scheduling, priority);

        }
    }

    //taking care of children
    process * tmp_top = queue_top(&(process_table.current_process) -> head_children, process, nodes_children);
    //choosing this method because the queue for each runs an error and I have 0 motivation to solve it
    while (tmp_top != NULL) {
        //if the process is in zombie mode, add it to the free process list
        //else just wait for its child to finish
        if (tmp_top -> state == STATE_ZOMBIE) {

            //set state of process to free
            tmp_top -> state = STATE_FREE;

            //free stack
            if (tmp_top -> stack != NULL) {

                mem_free(tmp_top -> stack, tmp_top -> size + 4096);
                queue_del(tmp_top, nodes_children);
            
            }
            //add it to the free prcess list
            queue_add(tmp_top, &process_table.free_process, process, scheduling, priority);

        } else {

            waitpid(tmp_top -> pid, NULL);

        }
        //update the top
        tmp_top = queue_top(&(process_table.current_process) -> head_children, process, nodes_children);
    }
   
    next_process(STATE_ZOMBIE);

    while(1);
}

int first_process(int (*pt_func)(void*), unsigned long ssize, const char *name) {

    //init the free and ready lists of processes
    INIT_LIST_HEAD(&process_table.free_process);
    INIT_LIST_HEAD(&process_table.ready_process);
    INIT_LIST_HEAD(&process_table.asleep_process);

    //default state of kernel is run mode
    process_table.kernel_state = STATE_KERNEL_RUNNING;

    //init static table of processes
    //setting their pids and their states at free
    //NB: starting at 1 because the first one is for the first process, duh!
    for (int i = 1; i < N_PROC; i++) {
        (process_table.table_process[i]).pid = i + 1;
        //free the process
        (process_table.table_process[i]).state = STATE_FREE;
        if ((process_table.table_process[i]).stack != NULL) {

            mem_free((process_table.table_process[i]).stack, (process_table.table_process[i]).size + 4096);
            queue_del(&(process_table.table_process[i]), nodes_children);
        
        }
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

    if (pid < 1 || pid > N_PROC) {

        return -1;

    }

    return (process_table.table_process[pid - 1]).priority;
}

process * current_process() {
    return process_table.current_process;
}

link * asleep_process_list() {
    return &process_table.asleep_process;
}

link * ready_process_list() {
    return &process_table.ready_process;
}

uint8_t state_kernel() {
    return process_table.kernel_state;
}

int chprio(int pid, int newprio) {
    
    if(newprio < 1 || newprio > MAX_PRIORITY
        || pid < 1 || pid > N_PROC
        || (process_table.table_process[pid - 1]).state == STATE_ZOMBIE
        || (process_table.table_process[pid - 1]).state == STATE_FREE){

        return -1;
    }

    //get old prio
    int32_t tmp_prio = (process_table.table_process[pid - 1]).priority;
    //update prio
    (process_table.table_process[pid - 1]).priority = newprio;

    //case where the process is in STATE WAITING MESSAGE block
    //add it to the waiting msg list
    if ((process_table.table_process[pid - 1]).state == STATE_WAIT_MESSAGE) {

        queue_del(&(process_table.table_process[pid - 1]), scheduling);
        queue_add(&(process_table.table_process[pid - 1]), (process_table.table_process[pid - 1]).waiting_msg_link, process, scheduling, priority);

    }

    //if pid is of current process and we have at least one process available
    //switch processes
    if ((process_table.table_process[pid - 1]).pid == (process_table.current_process) -> pid
    && !queue_empty(&process_table.ready_process)) {

            next_process(STATE_READY);

    }
    //if the concerned process is ready
    //add it to the list of available processes
    else if ((process_table.table_process[pid - 1]).state == STATE_READY) {

        queue_del(&(process_table.table_process[pid - 1]), scheduling);

        (process_table.table_process[pid - 1]).state = STATE_READY;

        queue_add(&(process_table.table_process[pid - 1]), &process_table.ready_process, process, scheduling, priority);
    }

    return tmp_prio;

}

int waitpid(int pid, int *retvalp) {

    if(pid == 0 || pid > N_PROC
        || (process_table.current_process) -> pid == (uint32_t) pid
        || queue_empty(&(process_table.current_process) -> head_children)){

        return -1;
    }

    //if father is zombie and pid is not of the first process
    if (pid > 0 && (process_table.table_process[pid - 1]).state == STATE_ZOMBIE) {
        //check that return value is not null
        if (retvalp != NULL) {
            //update return value with the one of the father
            *retvalp = (process_table.table_process[pid - 1]).return_value;
        }
        (process_table.table_process[pid - 1]).state =  STATE_FREE;
        //free the stack of the process
        if ((process_table.table_process[pid - 1]).stack != NULL) {

            mem_free((process_table.table_process[pid - 1]).stack, (process_table.table_process[pid - 1]).size + 4096);
            queue_del(&(process_table.table_process[pid - 1]), nodes_children);

        }
        queue_add(&(process_table.table_process[pid - 1]), &process_table.free_process, process, scheduling, priority);
        
        return pid;

    } else if (pid < 0) {
        process * tmp;

        queue_for_each(tmp, &(process_table.current_process) -> head_children, process, nodes_children){

            if(tmp -> state == STATE_ZOMBIE){
                if(retvalp != NULL){

                    *retvalp = (process_table.table_process[ tmp->pid - 1]).return_value;

                }
                //free the stack of the process
                if (tmp -> stack != NULL) {

                    mem_free(tmp -> stack, tmp -> size + 4096);
                    queue_del(tmp, nodes_children);

                }
                queue_add(tmp, &process_table.free_process, process, scheduling, priority);

                return tmp -> pid;
            }
        }
    }
    
    //update waitpid value
    (process_table.current_process) -> wait_pid_val = pid;

    //move on to next process
    next_process(STATE_CHILD_WAIT_BLOCK);

    int32_t current_wait_pid = (process_table.current_process) -> wait_pid_val;

    if(retvalp != NULL){
        *retvalp = (process_table.table_process[current_wait_pid - 1]).return_value;
    }

    (process_table.table_process[current_wait_pid - 1]).state =  STATE_FREE;
        //free the stack of the process
        if ((process_table.table_process[current_wait_pid - 1]).stack != NULL) {

            mem_free((process_table.table_process[current_wait_pid - 1]).stack, (process_table.table_process[current_wait_pid - 1]).size + 4096);
            queue_del(&(process_table.table_process[current_wait_pid - 1]), nodes_children);

        }
        queue_add(&(process_table.table_process[current_wait_pid - 1]), &process_table.free_process, process, scheduling, priority);

    return (process_table.current_process) -> wait_pid_val;
}

int kill(int pid) {

    if (pid < 1 || pid > N_PROC) {
        return -1;
    }

    //get the process to kill
    process * to_kill = &process_table.table_process[pid - 1];

    if(to_kill -> state == STATE_ZOMBIE
        || to_kill -> state == STATE_FREE){
        return -1;
    }

    //if the parent was blocked by the process to kill
    //aka state of child wait block
    if ((to_kill -> parent) -> state == STATE_CHILD_WAIT_BLOCK) {

        //if value of wait pid is < 0
        if ((to_kill -> parent) -> wait_pid_val < 0) {

            //update wait pid value
            (to_kill -> parent) -> wait_pid_val = to_kill -> pid;
        }

        //add parent to ready list
        (to_kill -> parent) -> state = STATE_READY;
        queue_add(to_kill -> parent, &process_table.ready_process, process, scheduling, priority);

    }

    //kill all children of process to be killed (#sosad)
    process * tmp;
    
    queue_for_each(tmp, &to_kill -> head_children, process, nodes_children) {

        if (tmp != NULL) {

            kill(tmp -> pid);
            //updating the process to take into account potential children
            //(some processes have a big family tree)
            tmp = queue_out(&to_kill -> head_children, process, nodes_children);
            //setting the parent to null because it already has been taken care of
            tmp -> parent = NULL;

            //add process to free list
            tmp -> state =  STATE_FREE;
            //free the stack of the process
            if (tmp -> stack != NULL) {
                mem_free(tmp -> stack, tmp -> size + 4096);
                queue_del(tmp, nodes_children);
            }

            queue_add(tmp, &process_table.free_process, process, scheduling, priority);

        }

    }

    //update return value
    to_kill -> return_value = 0;

    //actually kill process
    //but only if he was available, sleeping or waiting for message
    if (to_kill -> state == STATE_ASLEEP 
    || to_kill -> state == STATE_READY
    || to_kill -> state == STATE_WAIT_MESSAGE) {
        queue_del(to_kill, scheduling);
    }
    
    //if we are killing the current process
    //we need to switch to the next available one
    if ((process_table.current_process) -> pid == (uint32_t) pid) {
        next_process(STATE_ZOMBIE);
    } else {
        to_kill -> state = STATE_ZOMBIE;
    }

    return 0;
}