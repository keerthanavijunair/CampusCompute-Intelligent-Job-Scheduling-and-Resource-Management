
#include "process_manager.h"

#include <stdio.h>

int main(void)
{
    printf("====================================\n");
    printf("   CampusCompute Process Manager\n");
    printf("====================================\n");

    if (run_normal_child() != 0) {
        printf("Normal child test failed.\n");
        return 1;
    }

    if (run_failed_child() != 0) {
        printf("Failure detection test encountered an error.\n");
        return 1;
    }

    printf("\nAll process management tests completed.\n");

    return 0;
}
