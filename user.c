#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdbool.h>
#include "message.h"  // Include message structures and constants

#define SHMKEY 9876

int msgid;           // Message queue ID
int shmid;           // Shared memory ID
int *shmClock;       // Pointer to shared memory clock

// Structure to define messages
struct msgbuf {
    long mtype;
    int msg;
};

void cleanup() {
    shmdt(shmClock);  // Detach from shared memory
}

void sig_handler(int signo) {
    if (signo == SIGINT) {
        cleanup();
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    signal(SIGINT, sig_handler);

    // Connect to shared memory for system clock
    shmid = shmget(SHMKEY, 2 * sizeof(int), 0666);
    if (shmid == -1) {
        perror("user: shmget failed");
        exit(1);
    }

    shmClock = (int *)shmat(shmid, NULL, 0);
    if (shmClock == (int *)-1) {
        perror("user: shmat failed");
        exit(1);
    }

    // Connect to message queue
    key_t msgkey = ftok("oss", 65);  // Generate key for message queue
    msgid = msgget(msgkey, 0666);
    if (msgid == -1) {
        perror("user: msgget failed");
        exit(1);
    }

    struct msgbuf rcvMsg, sndMsg;

    // Main loop for receiving quantum and responding to OSS
    while (true) {
        // Receive time quantum from OSS
        if (msgrcv(msgid, &rcvMsg, sizeof(rcvMsg.msg), getpid(), 0) == -1) {
            perror("user: msgrcv failed");
            cleanup();
            exit(1);
        }

        int quantum = rcvMsg.msg;  // Time quantum received
        int percentage = (rand() % 99) + 1;  // Random percentage of quantum to use (1-99%)
        int timeUsed = (quantum * percentage) / 100;  // Actual time used by process

        // Determine if process will block, complete, or request more time
        int status;
        if (percentage < 80) {  // 80% chance to block before using full quantum
            status = MSG_WORKER_BLOCKED;
        } else if (percentage >= 80 && percentage < 95) {  // 15% chance to complete
            status = MSG_WORKER_TERMINATED;
        } else {
            status = MSG_WORKER_READY;  // 5% chance to use entire quantum and request more time
        }

        printf("USER PID: %d used %d ns of quantum %d ns; Status: %d\n", getpid(), timeUsed, quantum, status);

        // Send message back to OSS with actual time used and process status
        sndMsg.mtype = getppid();  // Send to OSS
        sndMsg.msg = timeUsed;

        if (msgsnd(msgid, &sndMsg, sizeof(sndMsg.msg), 0) == -1) {
            perror("user: msgsnd failed");
            cleanup();
            exit(1);
        }

        // Exit if process has completed
        if (status == MSG_WORKER_TERMINATED) {
            printf("USER PID: %d is terminating\n", getpid());
            cleanup();
            exit(0);
        }

        // Wait or go to blocked state
        if (status == MSG_WORKER_BLOCKED) {
            usleep(100 * 1000);  // Simulate wait time before becoming ready again
        }
    }

    cleanup();
    return 0;
}
