# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Executable names
OSS_EXEC = oss
USER_EXEC = user

# Default target to build both executables
all: $(OSS_EXEC) $(USER_EXEC)

# Compile OSS executable
$(OSS_EXEC): oss.c
	$(CC) $(CFLAGS) -o $(OSS_EXEC) oss.c

# Compile user executable
$(USER_EXEC): user.c
	$(CC) $(CFLAGS) -o $(USER_EXEC) user.c

# Clean up executables
clean:
	rm -f $(OSS_EXEC) $(USER_EXEC)

# Run OSS
run: $(OSS_EXEC)
	./$(OSS_EXEC)

# Remove shared memory and message queue resources (replace SHMKEY if necessary)
clean_ipc:
	ipcrm -M 9876
	ipcrm -Q $(msgkey)  # Replace $(msgkey) with the actual message queue ID if known
