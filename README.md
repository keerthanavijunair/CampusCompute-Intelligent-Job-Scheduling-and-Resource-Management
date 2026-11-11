# CampusCompute
## Intelligent Job Scheduling and Resource Management

CampusCompute is a command-line prototype demonstrating operating system concepts using C on Linux.

## Requirements
- Ubuntu/Linux
- GCC compiler
- GNU Make
- POSIX threads, shared memory and semaphores

## Build
Run `make` inside the project directory.

## Run
Start the integrated application using `./campuscompute`.

Select an option from the menu to access:
1. Job management and process lifecycle
2. FCFS scheduling
3. Round Robin scheduling
4. Non-preemptive Priority scheduling
5. Scheduling algorithm comparison
6. Producer-consumer logger using POSIX threads
7. Shared memory and semaphore communication
8. Banker's safety algorithm
9. Process management using fork() and waitpid()
10. Resource limit demonstration using getrlimit() and setrlimit()

## Independent Tests
- `./test_person1`
- `./test_person2`
- `./test_process`
- `./test_resource`
- `./shared_memory writer`
- `./shared_memory reader`

Run the shared memory writer before the reader.

## Cleanup
Use `make clean` to remove generated executables and logs.

Exit the application using menu option 0. The application cleans up its created threads, child processes, shared-memory objects and named semaphores during normal execution.

## Generated Log
The producer-consumer logger writes its output to `logs.txt`.

## Team Contributions

### Khushee Kundra (PES1UG25CS256)
Responsible for Person 1 modules:
- **FR1:** Job Management and Process Lifecycle.
- **FR2:** CPU Scheduling Algorithms (FCFS, Round Robin and Non-preemptive Priority Scheduling).
- **FR5:** Banker's Algorithm for Deadlock Avoidance.

### Keerthana Viju Nair (PES1UG25CS249)
Responsible for Person 2 modules:
- **FR3:** Producer-Consumer Synchronization using POSIX Threads.
- **FR4:** Inter-Process Communication using Shared Memory and Semaphores.
- **FR6:** Process Management using `fork()` and `waitpid()`.
- **FR7:** Resource Management using `setrlimit()` and `getrlimit()`.

### Joint Contributions
Both team members contributed to:
- Integration of individual modules into the complete CampusCompute application.
- Testing and verification of functional requirements.
- Debugging and resolving integration issues.
- Overall project development and final validation.
