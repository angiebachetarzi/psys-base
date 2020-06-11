#include "process.h"
#include "../shared/queue.h"
#include "mem.h"

procTable proc_table;


int start(int (*pt_func)(void*), unsigned long ssize, int prio, const char *name, void *arg) {

    if(prio > MAX_PRIO || prio < 1){

        return ERROR_PRIO;

    }

    //process * new_proc = queue_out(&proc_table.free, process, empty);

    process * new_proc = mem_alloc(sizeof(process));
    new_proc -> name = name;
    new_proc -> size = ssize;
    new_proc -> pt_func = pt_func;
    new_proc -> priority = prio;
    new_proc -> arg = arg;
    new_proc -> ppid = proc_table.current_pid;

    //new process list for the child
    new_proc -> stack = mem_alloc(ssize);
    uint32_t size = ssize / sizeof(uint32_t);
        
    new_proc -> context.esp = (uint32_t) &new_proc -> stack[size - 3];

    new_proc -> stack[size - 1] = (uint32_t) arg;
    new_proc -> stack[size - 3] = (uint32_t) pt_func;

    //if the father process has a lesser priority than his child
    if (proc_table.table_process[new_proc -> ppid - 1].priority < new_proc -> priority) {

        new_proc -> state = STATE_ACTIVE;

        //context switch
        // if(proc_table.current_pid == new_proc -> pid){
        //     return;
        // }
        //queue_add(&proc_table.table_processus[new_proc -> pid],&proc_table.active,processus,active,prio);
        proc_table.table_process[proc_table.current_pid -1].state = STATE_READY;
        uint32_t tmp_pid = proc_table.current_pid;
        proc_table.current_pid = new_proc -> pid;

        context_switch(&proc_table.table_process[ tmp_pid - 1 ].context,
                            &new_proc -> context);

    } else {

        new_proc -> state = STATE_READY;
        //queue_add(process,&proc_table.active,process,active,prio);

    }

    return new_proc -> pid;
}
