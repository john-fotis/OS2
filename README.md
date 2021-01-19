# Memory Management Simulation

## C project

Details: <br/>
Produce statistics based on the behavior of page replacement algorithms LRU and Second Chance <br/>
Two Threads read 2 different trace files alternately and load them into main memory. <br/>
Main memory is implemented by a queue-array. <br/>
The virtual memory is implemented by a hash table for each thread. <br/> <br/>
Statistics include: <br/>
Queue size (memory frames number), <br/>
Total iterations, Total P1 (gcc) iterations, Total P2 (bzip) iterations, <br/>
Total pagefaults, Total hits, Total updates, <br/>
Total reads from disk, Total writes on disk <br/>

### What you need:

### - A Linux Distribution OS

### - In terminal:

`sudo apt install gcc -y` <br/>

`sudo apt install make -y`

# In the project folder, open a terminal

## Compile with: `make all`

## Execution: `./main.ex [algorithm (0/1)] [frames number] [q] (max entries)`

### ~ Page replacement algorithm options are 0 for LRU and 1 for Second Chance

### ~ frames number stands for memory capacity

### ~ q stands for continuous trace entries a thread will refer before switching to the other thread

### ~ max entries is the total entries (from both threads) that the simulation will stop at


# Example executions:

## `./main.ex 0 96 10 400`

## `./main.ex 0 2048 11 100000`

## `./main.ex 1 2 1 500000`

## `./main.ex 1 5120 17 2000000`
