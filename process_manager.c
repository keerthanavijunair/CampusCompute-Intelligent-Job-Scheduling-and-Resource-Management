
#include "process_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

// Wait for a child process, retrying if interrupted by a signal.
static int wait_for_child(pid_t pid, int *status)
{
    pid_t result;

    do {
        result = waitpid(pid, status, 0);
    } while (result == -1 && errno == EINTR);

    if (result == -1) {
        perror("waitpid");
        return -1;
    }

    return 0;
}

int run_normal_child(void)
{
    pid_t pid;
    int status;

    printf("\n=== Normal Child Process ===\n");

    // Flush output before creating a child process.
    fflush(NULL);

    // Create a child process using fork().
    pid = fork();

    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        // Child process executes its assigned work.
        printf("Child: Process created successfully.\n");
        printf("Child: Performing assigned work...\n");

        sleep(1);

        printf("Child: Work completed successfully.\n");

        // Exit with status 0 to indicate successful execution.
        fflush(stdout);
        _exit(0);
    }

    // Parent process waits for the child to finish.
    printf("Parent: Created child with PID %d.\n", pid);
    printf("Parent: Waiting for child to finish...\n");

    if (wait_for_child(pid, &status) == -1) {
        return -1;
    }

    // Check whether the child terminated normally.
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);

        printf("Parent: Child exited with status %d.\n", exit_code);

        if (exit_code == 0) {
            printf("Parent: Child completed successfully.\n");
        } else {
            printf("Parent: Child failed.\n");
            return 1;
        }
    } else {
        printf("Parent: Child terminated abnormally.\n");
        return 1;
    }

    return 0;
}

int run_failed_child(void)
{
    pid_t pid;
    int status;

    printf("\n=== Simulated Child Process Failure ===\n");

    // Flush output before creating a child process.
    fflush(NULL);

    // Create another child process using fork().
    pid = fork();

    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        // Child process performs work before simulating failure.
        printf("Child: Process created successfully.\n");
        printf("Child: Performing assigned work...\n");

        sleep(1);

        printf("Child: Simulating a worker failure!\n");

        // Exit with status 1 to indicate failure.
        fflush(stdout);
        _exit(1);
    }

    // Parent waits for the child and retrieves its exit status.
    printf("Parent: Created child with PID %d.\n", pid);
    printf("Parent: Waiting for child status...\n");

    if (wait_for_child(pid, &status) == -1) {
        return -1;
    }

    // Detect whether the child exited normally or abnormally.
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);

        printf("Parent: Child exited with status %d.\n", exit_code);

        if (exit_code != 0) {
            printf("Parent: Worker failure detected!\n");
        } else {
            printf("Parent: Child completed successfully.\n");
        }
    } else {
        printf("Parent: Child terminated abnormally.\n");
    }

    return 0;
}
