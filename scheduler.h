#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "pcb.h"

// Function to find the next process to schedule based on priority ratio
int findNextProcessToSchedule(PCB *processTable, int numProcesses);

// Function to calculate the priority ratio (e.g., totalCPUTime / totalSystemTime)
float calculatePriorityRatio(PCB *pcb);

// Function to handle moving processes between ready and blocked queues
void handleProcessQueues(PCB *processTable, int numProcesses);

#endif
