#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>
#include <sys/wait.h>
#include "pcb.h"
#include "clock.h"
#include "message.h"
#include "scheduler.h"

#define SHMKEY 9876
#define MAX_PROCESSES 20

// Shared memory for the system clock
int *shmClock;
int shmid, msgid;
PCB processTable[MAX_PROCESSES];

// Message queue key
key_t msgkey;

// Flag to stop launching new processes after 3 seconds
bool stopLaunchingNewProcesses = false;

void cleanup() {
    shmdt(shmClock);
    shmctl(shmid, IPC_RMID, NULL);
    msgctl(msgid, IPC_RMID, NULL);
    printf("OSS: Cleaned up shared memory and message queue.\n");
}

void sig_handler(int signo) {
    if (signo == SIGINT) {
        cleanup();
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    signal(SIGINT, sig_handler);

    srand(time(NULL));

    // Initialize shared memory and message queue
    shmid = shmget(SHMKEY, 2 * sizeof(int), IPC_CREAT | 0666);
    shmClock = (int *)shmat(shmid, NULL, 0);

    msgkey = ftok("oss", 65);
    msgid = msgget(msgkey, 0666 | IPC_CREAT);

    // Initialize the shared clock
    shmClock[0] = 0;  // Seconds
    shmClock[1] = 0;  // Nanoseconds

    int numProcesses = 0;
    int launchedProcesses = 0;
    time_t startTime = time(NULL);

    while (true) {
        // Stop launching new processes after 3 real-life seconds
        if (difftime(time(NULL), startTime) > 3) {
            stopLaunchingNewProcesses = true;
        }

        // Launch new process if allowed
        if (!stopLaunchingNewProcesses && numProcesses < MAX_PROCESSES && launchedProcesses < MAX_PROCESSES) {
            int slot = findAvailableSlot();
            if (slot != -1) {
                pid_t childPid = fork();
                if (childPid == 0) {  // Child process
                    execl("./worker", "worker", NULL);
                    perror("oss: execl failed");
                    exit(1);
                } else if (childPid > 0) {  // OSS process
                    processTable[slot].occupied = 1;
                    processTable[slot].pid = childPid;
                    processTable[slot].startSeconds = shmClock[0];
                    processTable[slot].startNano = shmClock[1];
                    numProcesses++;
                    launchedProcesses++;
                    printf("OSS: Launched worker process PID %d\n", childPid);
                } else {
                    perror("oss: fork failed");
                }
            }
        }

        // Schedule a process based on priority
        int processIndex = findNextProcessToSchedule(processTable, MAX_PROCESSES);
        if (processIndex != -1) {
            int timeQuantum = 50;  // Maximum quantum of 50ms

            struct msgbuf sndMsg;
            sndMsg.mtype = processTable[processIndex].pid;
            sndMsg.msg = timeQuantum;

            if (msgsnd(msgid, &sndMsg, sizeof(sndMsg.msg), 0) == -1) {
                perror("oss: msgsnd failed");
            }

            struct msgbuf rcvMsg;
            if (msgrcv(msgid, &rcvMsg, sizeof(rcvMsg.msg), processTable[processIndex].pid, 0) == -1) {
                perror("oss: msgrcv failed");
            }

            int timeUsed = rcvMsg.msg;
            incrementClock(shmClock, timeUsed);

            if (rcvMsg.msg == 0) {  // Process has finished
                printf("OSS: Worker PID %d has completed.\n", processTable[processIndex].pid);
                processTable[processIndex].occupied = 0;
                numProcesses--;
            } else if (rcvMsg.msg == 1) {  // Process is blocked
                printf("OSS: Worker PID %d is blocked.\n", processTable[processIndex].pid);
                processTable[processIndex].status = BLOCKED;
            }
        }

        // Check if all processes are done
        if (numProcesses == 0 && stopLaunchingNewProcesses) break;

        usleep(100 * 1000);  // Simulate scheduling overhead
    }

    // Cleanup
    cleanup();
    return 0;
}
