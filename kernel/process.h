#ifndef PROCESS_H
#define PROCESS_H

    #include "queue.h"
    #include "context.h"
    #include "stdint.h"
    

    #define STATE_FREE 0x00
    #define STATE_READY 0x01
    #define STATE_ACTIVE 0x02
    //blocked on semaphore
    #define STATE_SM_BLOCK 0x03
    //blocked on IO
    #define STATE_IO_BLOCK 0x04
    //blocked waiting for child
    #define STATE_CHILD_WAIT_BLOCK 0x05
    #define STATE_SLEEP 0x06
    #define STATE_ZOMBIE 0x07


    #define MAX_PRIORITY 256
    #define N_PROC 30
    #define DEFAULT_PRIORITY 128

    /*
    * structure of process
    * pid: number of process
    * name: name of process
    * state: state of process
    * context: context of process
    * stack: stack of addresses
    * size: size of stack
    * pt_func: address of program of process
    * priority: priority of process
    * arg: argument of pt_func
    * parent: parent of process
    * scheduling: head for scheduling
    * return value: retval for exit function
    * wait_pid_val: return value of waitpid function
    * head_children: head of list of children of process
    * nodes_children: nodes of the list
    * NB: nodes_children is only here so i can use queue_for_each function
    */
    typedef struct _process {
        uint32_t pid;
        const char * name;
        uint8_t state;
        context context;
        uint32_t * stack;
        uint32_t size;
        int (*pt_func)(void *);
        uint32_t priority;
        void * arg;
        struct _process * parent;
        link scheduling;
        int32_t return_value;
        int32_t wait_pid_val;
        link head_children;
        link nodes_children;
    } process;

    /*
    * structure of process table
    * current_process: current process
    * table_process: static list of processes
    * free_process: head of list of free processes
    * ready_process: head of list of ready processes
    */
    struct procTable{
        process * current_process;
        process table_process[N_PROC];
        link free_process;
        link ready_process;
        context ctx_kernel;
    } ;
    typedef struct procTable proc_table;

    /*
    * start first process
    * pt_func: address of program of process
    * ssize: used size for process in stack
    * name: name of process to start
    * NB: first process has default prio
    */
    int first_process(int (*pt_func)(void*), unsigned long ssize, const char *name);

    /*
    * start a new process
    * pt_func: address of program of process
    * ssize: used size for process in stack
    * prio: priority of process to start
    * name: name of process to start
    * arg: argument for pt_func
    */
    int start(int (*pt_func)(void*), unsigned long ssize, int prio, const char *name, void *arg);

    /*
    * wait and get value of terminated child
    * pid: pid of process
    * retvalp: return value of child process
    */
    int waitpid(int pid, int *retvalp);

    /*
    * normal exit of process
    * retval: return value of process
    */
    void exit(int retval);

    /*
    * forced exit of process
    * pid: pid of process to kill
    */
    int kill(int pid);

    /*
    * get priority of process
    * pid: pid of process
    */
    int getprio(int pid);

    /*
    * change priority of process
    * pid: pid of process
    * newprio: new priority of process
    */
    int chprio(int pid, int newprio);

    /*
    * get pid of process
    */
    int getpid(void);

#endif