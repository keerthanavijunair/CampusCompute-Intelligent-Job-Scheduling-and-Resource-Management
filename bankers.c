
#include "bankers.h"

#include <stdio.h>

/* Check whether the system is in a safe state. */
int run_bankers_algorithm(
    int process_count,
    int resource_count,
    int allocation[MAX_PROCESSES][MAX_RESOURCES],
    int maximum[MAX_PROCESSES][MAX_RESOURCES],
    int available[MAX_RESOURCES],
    int safe_sequence[MAX_PROCESSES])
{
    int need[MAX_PROCESSES][MAX_RESOURCES];
    int work[MAX_RESOURCES];
    int finish[MAX_PROCESSES] = {0};

    int completed = 0;

    if (process_count <= 0 || process_count > MAX_PROCESSES ||
        resource_count <= 0 || resource_count > MAX_RESOURCES) {
        printf("Error: Invalid number of processes or resources.\n");
        return 0;
    }

    /* Calculate Need = Maximum - Allocation. */
    printf("\n=== Need Matrix ===\n");

    for (int i = 0; i < process_count; i++) {
        for (int j = 0; j < resource_count; j++) {

            if (allocation[i][j] < 0 ||
                maximum[i][j] < allocation[i][j] ||
                available[j] < 0) {
                printf("Error: Invalid resource allocation.\n");
                return 0;
            }

            need[i][j] = maximum[i][j] - allocation[i][j];

            printf("%d ", need[i][j]);
        }

        printf("\n");
    }

    /* Copy Available resources into Work. */
    for (int j = 0; j < resource_count; j++) {
        work[j] = available[j];
    }

    /* Find a process whose remaining needs can be satisfied. */
    while (completed < process_count) {
        int found = 0;

        for (int i = 0; i < process_count; i++) {

            if (finish[i]) {
                continue;
            }

            int possible = 1;

            for (int j = 0; j < resource_count; j++) {
                if (need[i][j] > work[j]) {
                    possible = 0;
                    break;
                }
            }

            if (possible) {

                /* Process finishes and releases its resources. */
                for (int j = 0; j < resource_count; j++) {
                    work[j] += allocation[i][j];
                }

                safe_sequence[completed] = i;
                finish[i] = 1;
                completed++;
                found = 1;
            }
        }

        if (!found) {
            printf("\nSystem is in an UNSAFE state.\n");
            return 0;
        }
    }

    printf("\nSystem is in a SAFE state.\n");

    printf("Safe Sequence: ");

    for (int i = 0; i < process_count; i++) {
        printf("P%d", safe_sequence[i]);

        if (i < process_count - 1) {
            printf(" -> ");
        }
    }

    printf("\n");

    return 1;
}
