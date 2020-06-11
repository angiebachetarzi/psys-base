#ifndef PROCESS_H
#define PROCESS_H

    #include "queue.h"
    #include "context.h"
    

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


    #define MAX_PRIO 256
    #define N_PROC 30


    #define ERROR_PRIO -1

    //structure of process
    //pid: number of process
    //name: name of process
    //state: state of process
    //context: context of process
    //stack: stack of addresses
    //size: size of stack
    //pt_func: address of program of process
    //priority: priority of process
    //arg: argument of pt_func
    //ppid: number of parent process
    typedef struct {
        uint32_t pid;
        const char * name;
        uint8_t state;
        context context;
        uint32_t * stack;
        uint32_t size;
        int (*pt_func)(void *);
        uint32_t priority;
        void * arg;
        uint32_t ppid;
        link scheduling;
        link childhead;
        link child;
    } process;

    //process table
    typedef struct {
        uint32_t current_pid;
        process table_process[N_PROC];
        link free;
        link active;
        link zombie;
        context ctx_kernel;
    } procTable;
    extern procTable proc_table;

    //start a new process
    //pt_func: address of program of process
    //ssize: used size for process in stack
    //prio: priority of process to start
    //name: name of process to start
    //arg: argument for pt_func
    int start(int (*pt_func)(void*), unsigned long ssize, int prio, const char *name, void *arg);

    //wait and get value of terminated child
    //pid: pid of process
    //retvalp: return value of child process
    int waitpid(int pid, int *retvalp);

    //normal exit of process
    void exit(int retval);

    //forced exit of process
    int kill(int pid);

    //get priority of process
    int getprio(int pid);

    //change priority of process
    int chprio(int pid, int newprio);

    //get pid of process
    int getpid(void);

#endif