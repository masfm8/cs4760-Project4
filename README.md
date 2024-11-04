# Project 4 - Operating System Simulator

This project simulates an operating system scheduler using inter-process communication (IPC) with shared memory and message queues. It manages multiple user processes, scheduling them based on specified time quanta, and tracks their execution in a simulated clock.

## Repository
GitHub: [cs4760-Project4](https://github.com/masfm8/cs4760-Project4)

## Files
- `oss.c`: The main file for the simulated operating system scheduler. Manages process creation, scheduling, message handling, and cleanup.
- `user.c`: Represents a user process that interacts with the scheduler, utilizing assigned time quanta and sending messages back to `oss`.
- `Makefile`: Automates the compilation of `oss` and `user` executables.
- `README.md`: Documentation on how to compile and run the project.

## Compilation
To compile the project, use the provided `Makefile`:
```bash
make


This will generate two executables:

oss: The scheduler.
user: The user process, which is forked by oss.
Running the Project
After compilation, run the scheduler:
./oss
The scheduler will create and manage user processes up to a specified maximum (e.g., 20). It will terminate after approximately 3 real-time seconds or when all processes have finished.

Simulation Output
The program prints logs for:

Launching new user processes
Process time usage reports
Updates to the simulated system clock
Termination signals and cleanup messages

Example output:
OSS: Launched user process PID 12345
OSS: Received time used 50 ns from PID 12345
OSS: Updated clock: 0s 50ns
...
OSS: User process PID 12345 has terminated.
OSS: Cleaned up shared memory and message queue.

This `README.md` should help users understand the project’s purpose, 
how to compile and run it, and the key configuration and cleanup steps. 
Let me know if you need further adjustments or additional sections!
