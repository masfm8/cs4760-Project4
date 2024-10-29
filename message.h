#ifndef MESSAGE_H
#define MESSAGE_H

#define MSG_WORKER_READY 1      // Message type for a ready (still running) process
#define MSG_WORKER_BLOCKED 2    // Message type for a blocked process
#define MSG_WORKER_TERMINATED 3 // Message type for a terminated process

// Message buffer structure for message passing between OSS and user processes
struct msgbuf {
    long mtype; // Message type (PID of the receiving process)
    int msg;    // Message content (e.g., time quantum or status message)
};

#endif
