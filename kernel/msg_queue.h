#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

    #include "queue.h"
    #include "process.h"
    #include "stdint.h"
    #include "mem.h"

    #define N_QUEUE 32

    /*
    * Structure of a message queue
    * fid: id of the queue
    * size: size of the queue
    * nb_msg: number of msg in the queue
    * msg_link: link for messages
    * waiting_process_link: list of waiting processes in queue
    */
   typedef struct _msg_queue {
       int32_t fid;
       uint32_t size;
       uint32_t nb_msg;
       link msg_link;
       link waiting_process_link;
   } msg_queue;

   /*
   * Structure of message
   * value: value of message
   * priority: prio of message
   * msg_link: link for message
   */
  typedef struct _msg {
      int value;
      int priority;
      link msg_link;
  } msg;

  /**
   * Global message queue
   */
   msg_queue * table_queue[N_QUEUE];

   /**
    * create a queue
    * count: size of queue
    */
   int pcreate(int count);

   /**
    * delete a queue
    * fid: id of queue to delete
    */
   int pdelete(int fid);

   /**
    * send a message to a queue
    * fid: id of the queue
    * mess: message to send
    */
   int psend(int fid, int msg);

   /**
    * receive a message from a queue
    * fid: id of queue
    * mess: message to be received
    */
   int preceive(int fid, int * msg);

   /**
    * reset a queue
    * fid: id of the queue
    */
   int preset(int fid);

   /**
    * returns number of processes blocked on empty queue (negative value)
    * or number of processes blocked on full queue and number of messages
    * fid: id of queue
    * count: result of count
    */
   int pcount(int fid, int * count);

   /**
    * returns the fid of first available queue
    */
   int32_t available_queue();
  
#endif 