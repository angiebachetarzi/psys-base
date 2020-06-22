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

int preceive(int fid, int * msg_to_receive) {

    if (fid < 0
        || fid >= N_QUEUE
        || table_queue[fid] == NULL
        || table_queue[fid] -> fid == -1) {

        return -1;
    }
    
    //if the queue is empty (no messages)
    if (table_queue[fid] -> nb_msg == 0) {

        //get current process
        process * curr = current_process();
        //add it to the list of waiting processes
        queue_add(curr, &(table_queue[fid] -> waiting_process_link), process, waiting_msg_link, priority);
        //update fid for process
        curr -> fid_waiting = fid;
        //move on to next process
        next_process(STATE_WAIT_MESSAGE);

        if (curr -> fid_waiting == -1) {

            return -1;

        } else {

            if (msg_to_receive != NULL) {
                //update message
                *msg_to_receive = curr -> msg_value;
            }

        }
        
    }
    //if there is still a process waiting
    else if (!queue_empty(&(table_queue[fid] -> waiting_process_link))) {

        //get message from the list
        msg * m = queue_out(&(table_queue[fid] -> msg_link), msg, msg_link);
        //update value
        if (msg_to_receive != NULL) {
            *msg_to_receive = m -> value;
        }

        //get next waiting process
        process * p_wait = queue_out(&(table_queue[fid] -> waiting_process_link), process, waiting_msg_link);
        //add it to the ready list
        p_wait -> state = STATE_READY;
        queue_add(p_wait, ready_process_list(), process, scheduling, priority);

        //create new message
        msg * m2 = mem_alloc(sizeof(msg));
        if (m2 == NULL) {
            return -1;
        }
        m2 -> value = p_wait -> msg_value;
        m2 -> priority = 1;
        link m_link = {NULL, NULL};
        m2 -> msg_link = m_link;

        //add msg to the global queue
        queue_add(m2, &(table_queue[fid] -> msg_link), msg, msg_link, priority);
        next_process(STATE_WAIT_MESSAGE);
    }
    //queue full
    else {
        //get last msg from queue
        msg * m = queue_out(&(table_queue[fid] -> msg_link), msg, msg_link);

        //update msg value
        if (msg_to_receive != NULL) {
            *msg_to_receive = m -> value;
        }

        //update number of messages of global queue
        table_queue[fid] -> nb_msg--;
        //free message
        mem_free(m, sizeof(msg));

    }

    return 0;
}

int pcount (int fid, int * count) {

    if (fid < 0
        || fid >= N_QUEUE
        || table_queue[fid] == NULL
        || table_queue[fid] -> fid == -1) {

        return -1;
    }

    if (count != NULL) {

        int counter = 0;
        process * tmp;
        queue_for_each(tmp, &(table_queue[fid] -> waiting_process_link), process, waiting_msg_link) {
            counter++;
        }

        //if global queue has no messages
        if (table_queue[fid] -> nb_msg == 0) {
            *count = -counter;
        } else {
            *count = table_queue[fid] -> nb_msg + counter;
        }
    }

    return 0;
}

int preset (int fid) {

    if (fid < 0
        || fid >= N_QUEUE
        || table_queue[fid] == NULL
        || table_queue[fid] -> fid == -1) {

        return -1;
    }

    //delete all messages
    table_queue[fid] -> nb_msg = 0;
    while(!queue_empty(&(table_queue[fid] -> msg_link))) {
        msg * message = queue_out(&(table_queue[fid] -> msg_link), msg, msg_link);
        mem_free(message, sizeof(msg));
    }

    //wake every process up
    process * tmp;
    while (!queue_empty(&(table_queue[fid] -> waiting_process_link))) {
        tmp = queue_out(&(table_queue[fid] -> waiting_process_link), process, waiting_msg_link);
        tmp -> fid_waiting = -1;
        tmp -> state = STATE_READY;
        queue_add(tmp, ready_process_list(), process, scheduling, priority);
    }

    return 0;
    
}

