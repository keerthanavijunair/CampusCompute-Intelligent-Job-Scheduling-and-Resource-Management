#include "jobs.h"
#include "scheduling.h"
#include "bankers.h"

#include <stdio.h>

int main(void)
{
    JobManager manager;

    int safe_allocation[MAX_PROCESSES][MAX_RESOURCES] = {
        {0, 1, 0},
        {2, 0, 0},
        {3, 0, 2},
        {2, 1, 1},
        {0, 0, 2}
    };

    int safe_maximum[MAX_PROCESSES][MAX_RESOURCES] = {
        {7, 5, 3},
        {3, 2, 2},
        {9, 0, 2},
        {2, 2, 2},
        {4, 3, 3}
    };

    int safe_available[MAX_RESOURCES] = {3, 3, 2};
    int safe_sequence[MAX_PROCESSES];

    int unsafe_allocation[MAX_PROCESSES][MAX_RESOURCES] = {
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1}
    };

    int unsafe_maximum[MAX_PROCESSES][MAX_RESOURCES] = {
        {2, 1, 0},
        {0, 2, 1},
        {1, 0, 2}
    };

    int unsafe_available[MAX_RESOURCES] = {0, 0, 0};

    printf("====================================\n");
    printf("     CampusCompute Job Manager\n");
    printf("====================================\n");

    init_job_manager(&manager);

    printf("\n=== Adding Jobs ===\n");
    add_job(&manager, 0, 5, 2);
    add_job(&manager, 1, 3, 1);
    add_job(&manager, 2, 8, 3);

    printf("\n=== Current Job Details ===\n");
    display_jobs(&manager);

    printf("\n=== FCFS Scheduling ===\n");
    run_fcfs(&manager);

    printf("\n=== Round Robin Scheduling ===\n");
    run_round_robin(&manager, 2);

    printf("\n=== Priority Scheduling ===\n");
    run_priority(&manager);

    printf("\n=== Scheduling Comparison ===\n");
    compare_scheduling(&manager, 2);

    printf("\n=== Banker's Algorithm: Safe State ===\n");
    run_bankers_algorithm(
        5, 3, safe_allocation, safe_maximum,
        safe_available, safe_sequence
    );

    printf("\n=== Banker's Algorithm: Unsafe State ===\n");
    run_bankers_algorithm(
        3, 3, unsafe_allocation, unsafe_maximum,
        unsafe_available, safe_sequence
    );

    printf("\n=== Testing Invalid Job ===\n");
    add_job(&manager, -1, 5, 2);

    return 0;
}
