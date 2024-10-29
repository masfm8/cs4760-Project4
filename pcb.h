#ifndef PCB_H
#define PCB_H

#include <sys/types.h>

#define MAX_PROCESSES 20
#define STATUS_READY 0
#define STATUS_BLOCKED 1
#define STATUS_TERMINATED 2

typedef struct {
    int occupied;            // Indicates if this slot in the table is in use
    pid_t pid;               // Process ID of the user process
    int startSeconds;        // Start time (seconds) from the system clock
    int startNano;           // Start time (nanoseconds) from the system clock
    int totalCPUTime;        // Total CPU time used by this process
    int totalSystemTime;     // Total time in the system (from start to current)
    int lastBurstTime;       // Time used in the most recent CPU burst
    int status;              // Process status: READY, BLOCKED, TERMINATED
} PCB;

#endif
