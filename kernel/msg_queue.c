#include "msg_queue.h"

int32_t available_queue() {
    for (int i = 0; i < N_QUEUE; i++) {
        if (table_queue[i] == NULL || table_queue[i] -> fid == -1) {
            return i;
        }
    }
    return -1;
}

int pcreate(int count) {

    //check that count is valid
    if (count <= 0 || count > INT32_MAX / 2) {
        return -1;
    }

    //get first available queue
    int32_t new_fid = available_queue();
    if (new_fid == -1) {
        return -1;
    }

    //create new message queue
    msg_queue * queue;
    queue = mem_alloc(sizeof(msg_queue));
    if (queue == NULL) {
        return -1;
    }

    //update global queue
    table_queue[new_fid] = queue;

    //update structure of newly created queue
    queue -> fid = new_fid;
    queue -> size = count;
    queue -> nb_msg = 0;
    INIT_LIST_HEAD(&(queue -> msg_link));
    INIT_LIST_HEAD(&(queue -> waiting_process_link));

    return new_fid;

}

int pdelete(int fid) {
    
    if (fid < 0
        || fid >= N_QUEUE
        || table_queue[fid] == NULL
        || table_queue[fid] -> fid == -1) {

        return -1;
    }

    //delete all messages of queue
    table_queue[fid] -> nb_msg = 0;
    while(!queue_empty(&(table_queue[fid] -> msg_link))) {
        msg * message = queue_out(&(table_queue[fid] -> msg_link), msg, msg_link);
        mem_free(message, sizeof(msg));
    }

    //update global queue
    table_queue[fid] -> fid = -1;

    //add all processes of queue to ready list
    process * tmp;
    while (!queue_empty(&(table_queue[fid] -> waiting_process_link))) {
        tmp = queue_out(&(table_queue[fid] -> waiting_process_link), process, waiting_msg_link);
        tmp -> state = STATE_READY;
        queue_add(tmp, ready_process_list(), process, scheduling, priority);
        tmp -> fid_waiting = -1;
    }

    return 0;
}

int psend(int fid, int msg_to_send) {

    if (fid < 0
        || fid >= N_QUEUE
        || table_queue[fid] == NULL
        || table_queue[fid] -> fid == -1) {

        return -1;
    }
    
    //if the queue is full
    if (table_queue[fid] -> nb_msg == table_queue[fid] -> size) {

        //get current process
        process * curr = current_process();

        //update msg of process
        curr -> msg_value = msg_to_send;

        //add process to the waiting list
        queue_add(curr, &(table_queue[fid] -> waiting_process_link), process, waiting_msg_link, priority);

        //update fid for process
        curr -> fid_waiting = fid;

        //move on to next process
        next_process(STATE_WAIT_MESSAGE);

        if (curr -> fid_waiting == -1) {
            return -1;
        }
    }
    //if th waiting process list is not empty 
    else if (!queue_empty(&(table_queue[fid] -> waiting_process_link))) {
        //wake up the oldest process
        process * old = queue_out(&(table_queue[fid] -> waiting_process_link), process, waiting_msg_link);
        //give him the message
        old -> msg_value = msg_to_send;
        //add him to the ready list
        old -> state = STATE_READY;
        queue_add(old, ready_process_list(), process, scheduling, priority);

        //move on to next available process
        next_process(STATE_READY);
    }
    else {
        //create new message
        msg * message = mem_alloc(sizeof(msg));
        if (message == NULL) {
            return -1;
        }
        //update structure of message
        message -> value = msg_to_send;
        message -> priority = 1;
        link m_link = {NULL, NULL};
        message -> msg_link = m_link;
        //add message to the message list of global queue
        queue_add(message, &(table_queue[fid] -> msg_link), msg, msg_link, priority);
        //update number of messages for global queue
        table_queue[fid] -> nb_msg++;
    }
    return 0;

}

int preceive(int fid, int * msg) {

//temp
    if (fid < 0
        || fid >= N_QUEUE
        || table_queue[fid] == NULL
        || table_queue[fid] -> fid == -1) {

        return -1;
    }
    *msg = 0;
    return 0;
}

