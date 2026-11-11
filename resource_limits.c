
#include "resource_limits.h"

#include <stdio.h>
#include <sys/resource.h>

int demonstrate_resource_limit(void)
{
    struct rlimit limit;

    printf("\n=== Resource Limit Demonstration ===\n");

    // Retrieve the current file descriptor limits.
    if (getrlimit(RLIMIT_NOFILE, &limit) == -1) {
        perror("getrlimit");
        return -1;
    }

    // Soft limit is the current limit enforced by the OS.
    printf("Current soft file descriptor limit: %ld\n",
           (long)limit.rlim_cur);

    // Hard limit is the maximum soft limit that an unprivileged
    // process can set.
    printf("Current hard file descriptor limit: %ld\n",
           (long)limit.rlim_max);

    // Set the new soft limit without changing the hard limit.
    if (limit.rlim_max != RLIM_INFINITY && limit.rlim_max < 64) {
        fprintf(stderr, "Cannot set soft limit to 64: hard limit is too low.\n");
        return -1;
    }

    limit.rlim_cur = 64;

    // Apply the new resource limit to the current process.
    if (setrlimit(RLIMIT_NOFILE, &limit) == -1) {
        perror("setrlimit");
        return -1;
    }

    printf("Soft file descriptor limit changed to 64.\n");

    // Retrieve the limits again to verify the change.
    if (getrlimit(RLIMIT_NOFILE, &limit) == -1) {
        perror("getrlimit");
        return -1;
    }

    printf("Verified soft limit: %ld\n", (long)limit.rlim_cur);

    if (limit.rlim_cur != 64) {
        fprintf(stderr, "Resource limit verification failed.\n");
        return -1;
    }

    return 0;
}
