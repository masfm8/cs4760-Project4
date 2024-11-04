#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdbool.h>

#define SHMKEY 9876

int msgid;
int shmid;
int *shmClock;

struct msgbuf {
    long mtype;
    int msg;
};

// Define message types
#define MSG_WORKER_READY 1
#define MSG_WORKER_BLOCKED 2
#define MSG_WORKER_TERMINATED 3

void cleanup() {
    shmdt(shmClock);
}

void sig_handler(int signo) {
    if (signo == SIGINT) {
        cleanup();
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    signal(SIGINT, sig_handler);

    // Attach to shared memory
    shmid = shmget(SHMKEY, 2 * sizeof(int), 0666);
    shmClock = (int *)shmat(shmid, NULL, 0);

    // Attach to message queue
    key_t msgkey = ftok("oss", 65);
    msgid = msgget(msgkey, 0666);

    struct msgbuf rcvMsg, sndMsg;

    while (true) {
        if (msgrcv(msgid, &rcvMsg, sizeof(rcvMsg.msg), getpid(), 0) == -1) {
            perror("user: msgrcv failed");
            cleanup();
            exit(1);
        }

        if (rcvMsg.msg == MSG_WORKER_TERMINATED) {
            printf("USER PID: %d received termination signal\n", getpid());
            cleanup();
            exit(0);
        }

        int quantum = rcvMsg.msg;
        int percentage = (rand() % 99) + 1;
        int timeUsed = (quantum * percentage) / 100;

        sndMsg.mtype = getppid();
        sndMsg.msg = timeUsed;

        if (msgsnd(msgid, &sndMsg, sizeof(sndMsg.msg), 0) == -1) {
            perror("user: msgsnd failed");
            cleanup();
            exit(1);
        }

        printf("USER PID: %d used %d ns of quantum %d ns\n", getpid(), timeUsed, quantum);
    }

    cleanup();
    return 0;
}
