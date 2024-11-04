#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>

#define SHMKEY 9876
#define MAX_PROCESSES 10  // Set to avoid excessive resource usage
#define TERMINATION_LIMIT 3  // 3 seconds as the termination threshold

struct msgbuf {
    long mtype;
    int msg;
};

// Define message types
#define MSG_WORKER_READY 1
#define MSG_WORKER_BLOCKED 2
#define MSG_WORKER_TERMINATED 3

int shmid, msgid;
int *shmClock;
pid_t pids[MAX_PROCESSES];
time_t startTime;

// Function to clean up shared memory and message queue
void cleanup() {
    shmdt(shmClock);
    shmctl(shmid, IPC_RMID, NULL);
    msgctl(msgid, IPC_RMID, NULL);
    printf("OSS: Cleaned up shared memory and message queue.\n");
}

// Function to check termination condition
int shouldTerminate() {
    return (time(NULL) - startTime) >= TERMINATION_LIMIT;
}

int main() {
    // Set up shared memory
    shmid = shmget(SHMKEY, 2 * sizeof(int), IPC_CREAT | 0666);
    shmClock = (int *)shmat(shmid, NULL, 0);
    shmClock[0] = 0;  // seconds
    shmClock[1] = 0;  // nanoseconds

    // Set up message queue
    key_t msgkey = ftok("oss", 65);
    msgid = msgget(msgkey, IPC_CREAT | 0666);

    // Start timing
    startTime = time(NULL);

    int numProcesses = 0;

    // Launch and manage user processes
    while (!shouldTerminate() && numProcesses < MAX_PROCESSES) {
        // Launch user process
        pid_t pid = fork();
        if (pid == 0) {
            execl("./user", "user", NULL);
            perror("oss: execl failed");
            exit(1);
        } else if (pid > 0) {
            pids[numProcesses++] = pid;
            printf("OSS: Launched user process PID %d\n", pid);

            // Send an initial message to the process with quantum information
            struct msgbuf msg;
            msg.mtype = pid;
            msg.msg = 50;  // example quantum in nanoseconds

            if (msgsnd(msgid, &msg, sizeof(msg.msg), 0) == -1) {
                perror("oss: msgsnd failed");
            }
        } else {
            perror("oss: fork failed");
        }

        // Receive messages from user processes
        struct msgbuf rcvMsg;
        if (msgrcv(msgid, &rcvMsg, sizeof(rcvMsg.msg), 0, IPC_NOWAIT) != -1) {
            printf("OSS: Received time used %d ns from PID %ld\n", rcvMsg.msg, rcvMsg.mtype);
            shmClock[1] += rcvMsg.msg;

            // Update shared clock and check for nanoseconds overflow
            if (shmClock[1] >= 1000000000) {
                shmClock[0]++;
                shmClock[1] -= 1000000000;
            }
            printf("OSS: Updated clock: %ds %dns\n", shmClock[0], shmClock[1]);
        }

        usleep(100 * 1000);  // interval between child launches
    }

    // Send termination signal to all remaining user processes
    for (int i = 0; i < numProcesses; i++) {
        struct msgbuf msg;
        msg.mtype = pids[i];
        msg.msg = MSG_WORKER_TERMINATED;  // Send termination signal

        if (msgsnd(msgid, &msg, sizeof(msg.msg), 0) == -1) {
            perror("oss: msgsnd termination signal failed");
        }
    }

    // Wait for all user processes to terminate
    for (int i = 0; i < numProcesses; i++) {
        waitpid(pids[i], NULL, 0);
        printf("OSS: User process PID %d has terminated.\n", pids[i]);
    }

    // Cleanup resources
    cleanup();
    return 0;
}
