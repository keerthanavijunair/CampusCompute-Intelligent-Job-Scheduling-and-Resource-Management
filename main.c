#include "jobs.h"
#include "scheduling.h"
#include "bankers.h"
#include "logger.h"
#include "shared_memory.h"
#include "process_manager.h"
#include "resource_limits.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>

#define PRODUCER_COUNT 3
#define MESSAGES_PER_PRODUCER 5

typedef struct {
    Logger *logger;
    int producer_id;
} ProducerArgs;

/* Read and validate integer input. */
static int read_int(const char *prompt, int *value)
{
    char line[128];
    char *end;
    long number;

    printf("%s", prompt);

    if (fgets(line, sizeof(line), stdin) == NULL)
        return -1;

    errno = 0;
    number = strtol(line, &end, 10);

    while (isspace((unsigned char)*end))
        end++;

    if (end == line || *end != '\0' || errno != 0 ||
        number < -2147483647L - 1 || number > 2147483647L)
        return 0;

    *value = (int)number;
    return 1;
}

static int ask_int(const char *prompt)
{
    int value;
    int result;

    while ((result = read_int(prompt, &value)) == 0)
        printf("Invalid input. Please enter an integer.\n");

    return result == -1 ? -1 : value;
}

/* FR1: Job management and lifecycle. */
static void manage_jobs(JobManager *manager)
{
    int choice;

    printf("\n=== Job Management ===\n");
    printf("1. Add job\n");
    printf("2. Display jobs\n");
    printf("3. Update job status\n");

    choice = ask_int("Enter choice: ");
    if (choice == -1)
        return;

    if (choice == 1) {
        int arrival = ask_int("Enter arrival time: ");
        if (arrival == -1) return;

        int burst = ask_int("Enter CPU burst time: ");
        if (burst == -1) return;

        int priority = ask_int("Enter priority: ");
        if (priority == -1) return;

        add_job(manager, arrival, burst, priority);
    }
    else if (choice == 2) {
        display_jobs(manager);
    }
    else if (choice == 3) {
        int id = ask_int("Enter job ID: ");
        if (id == -1) return;

        printf("1. READY\n2. RUNNING\n3. COMPLETED\n");
        int status = ask_int("Select new status: ");
        if (status == -1) return;

        if (status < 1 || status > 3) {
            printf("Invalid status.\n");
            return;
        }

        if (update_job_status(manager, id, (JobStatus)(status - 1)) == 0)
            printf("Job status updated successfully.\n");
        else
            printf("Unable to update job status.\n");
    }
    else {
        printf("Invalid job management option.\n");
    }
}

/* FR3: Producer threads submitting messages to the logger. */
static void *producer_function(void *arg)
{
    ProducerArgs *data = (ProducerArgs *)arg;
    char message[MESSAGE_SIZE];

    for (int i = 1; i <= MESSAGES_PER_PRODUCER; i++) {
        snprintf(message, sizeof(message),
                 "Producer %d generated message %d",
                 data->producer_id, i);

        if (logger_log(data->logger, message) != 0) {
            printf("Producer %d could not submit a message.\n",
                   data->producer_id);
            break;
        }

        printf("Producer %d submitted message %d\n",
               data->producer_id, i);
    }

    return NULL;
}

static void run_logger(void)
{
    Logger logger;
    pthread_t producers[PRODUCER_COUNT];
    ProducerArgs args[PRODUCER_COUNT];
    int created = 0;

    printf("\n=== Producer-Consumer Logger ===\n");

    if (logger_init(&logger, "logs.txt") != 0) {
        printf("Logger initialization failed.\n");
        return;
    }

    for (int i = 0; i < PRODUCER_COUNT; i++) {
        args[i].logger = &logger;
        args[i].producer_id = i + 1;

        if (pthread_create(&producers[i], NULL,
                           producer_function, &args[i]) != 0) {
            printf("Failed to create producer thread.\n");
            break;
        }

        created++;
    }

    for (int i = 0; i < created; i++)
        pthread_join(producers[i], NULL);

    logger_destroy(&logger);

    printf("Producer threads and consumer have terminated.\n");
    printf("Messages saved in logs.txt\n");
}

/* FR5: Banker's algorithm with safe and unsafe examples. */
static void run_bankers_demo(void)
{
    int choice;
    int sequence[MAX_PROCESSES];

    int safe_allocation[MAX_PROCESSES][MAX_RESOURCES] = {
        {0, 1, 0}, {2, 0, 0}, {3, 0, 2},
        {2, 1, 1}, {0, 0, 2}
    };

    int safe_maximum[MAX_PROCESSES][MAX_RESOURCES] = {
        {7, 5, 3}, {3, 2, 2}, {9, 0, 2},
        {2, 2, 2}, {4, 3, 3}
    };

    int safe_available[MAX_RESOURCES] = {3, 3, 2};

    int unsafe_allocation[MAX_PROCESSES][MAX_RESOURCES] = {
        {1, 0, 0}, {0, 1, 0}, {0, 0, 1}
    };

    int unsafe_maximum[MAX_PROCESSES][MAX_RESOURCES] = {
        {2, 0, 0}, {0, 2, 0}, {0, 0, 2}
    };

    int unsafe_available[MAX_RESOURCES] = {0, 0, 0};

    printf("\n=== Banker's Algorithm ===\n");
    printf("1. Safe state demonstration\n");
    printf("2. Unsafe state demonstration\n");

    choice = ask_int("Enter choice: ");
    if (choice == -1) return;

    if (choice == 1) {
        run_bankers_algorithm(5, 3, safe_allocation,
                              safe_maximum, safe_available, sequence);
    }
    else if (choice == 2) {
        run_bankers_algorithm(3, 3, unsafe_allocation,
                              unsafe_maximum, unsafe_available, sequence);
    }
    else {
        printf("Invalid choice.\n");
    }
}

/* Main menu. */
static void display_menu(void)
{
    printf("\n========================================\n");
    printf("       CAMPUSCOMPUTE JOB MANAGER\n");
    printf("========================================\n");
    printf("1. Job Management (FR1)\n");
    printf("2. FCFS Scheduling (FR2)\n");
    printf("3. Round Robin Scheduling (FR2)\n");
    printf("4. Priority Scheduling (FR2)\n");
    printf("5. Compare Scheduling Algorithms (FR2)\n");
    printf("6. Producer-Consumer Logger (FR3)\n");
    printf("7. Shared Memory and Semaphore (FR4)\n");
    printf("8. Banker's Algorithm (FR5)\n");
    printf("9. Process Management (FR6)\n");
    printf("10. Resource Limit Demonstration (FR7)\n");
    printf("0. Exit\n");
    printf("========================================\n");
}

int main(void)
{
    JobManager manager;
    int choice;

    init_job_manager(&manager);

    printf("CampusCompute integrated application started.\n");

    while (1) {
        display_menu();

        choice = ask_int("Enter your choice: ");

        if (choice == -1) {
            printf("\nInput closed. Exiting CampusCompute.\n");
            break;
        }

        switch (choice) {
        case 1:
            manage_jobs(&manager);
            break;

        case 2:
            run_fcfs(&manager);
            break;

        case 3: {
            int quantum = ask_int("Enter time quantum: ");
            if (quantum != -1)
                run_round_robin(&manager, quantum);
            break;
        }

        case 4:
            run_priority(&manager);
            break;

        case 5: {
            int quantum = ask_int("Enter time quantum for comparison: ");
            if (quantum != -1)
                compare_scheduling(&manager, quantum);
            break;
        }

        case 6:
            run_logger();
            break;

        case 7:
            printf("\n=== Shared Memory Communication ===\n");
            if (writer() == 0)
                reader();
            else
                printf("Shared memory writer failed.\n");
            break;

        case 8:
            run_bankers_demo();
            break;

        case 9:
            printf("\n=== Process Management ===\n");
            run_normal_child();
            run_failed_child();
            break;

        case 10:
            printf("\n=== Resource Limit Demonstration ===\n");
            demonstrate_resource_limit();
            break;

        case 0:
            printf("Exiting CampusCompute. Goodbye!\n");
            return 0;

        default:
            printf("Invalid menu option. Please select a listed option.\n");
        }
    }

    return 0;
}
