
#ifndef BANKERS_H
#define BANKERS_H

#define MAX_PROCESSES 10
#define MAX_RESOURCES 10

int run_bankers_algorithm(
    int process_count,
    int resource_count,
    int allocation[MAX_PROCESSES][MAX_RESOURCES],
    int maximum[MAX_PROCESSES][MAX_RESOURCES],
    int available[MAX_RESOURCES],
    int safe_sequence[MAX_PROCESSES]
);

#endif
