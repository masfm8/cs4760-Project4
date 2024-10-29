# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Executable names
OSS_EXEC = oss
USER_EXEC = user

# Object files
OSS_OBJ = oss.o
USER_OBJ = user.o

# Header files
HEADERS = pcb.h clock.h message.h scheduler.h

# Default target
all: $(OSS_EXEC) $(USER_EXEC)

# Compile OSS
$(OSS_EXEC): $(OSS_OBJ)
	$(CC) $(CFLAGS) -o $(OSS_EXEC) $(OSS_OBJ)

# Compile user process
$(USER_EXEC): $(USER_OBJ)
	$(CC) $(CFLAGS) -o $(USER_EXEC) $(USER_OBJ)

# Compile OSS object
oss.o: oss.c $(HEADERS)
	$(CC) $(CFLAGS) -c oss.c

# Compile user process object
user.o: user.c $(HEADERS)
	$(CC) $(CFLAGS) -c user.c

# Clean up object files and executables
clean:
	rm -f $(OSS_EXEC) $(USER_EXEC) *.o

# Run OSS
run: $(OSS_EXEC)
	./$(OSS_EXEC)

# Remove shared memory and message queue (replace SHMKEY and msgkey as needed)
clean_ipc:
	ipcrm -M 9876
	ipcrm -Q $(msgkey)  # Ensure $(msgkey) is replaced with actual value if needed
