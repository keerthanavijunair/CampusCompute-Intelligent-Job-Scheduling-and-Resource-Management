#include "resource_limits.h"

#include <stdio.h>

int main(void)
{
    printf("====================================\n");
    printf("   CampusCompute Resource Manager\n");
    printf("====================================\n");

    if (demonstrate_resource_limit() != 0) {
        printf("Resource limit demonstration failed.\n");
        return 1;
    }

    printf("\nResource limit test completed successfully.\n");

    return 0;
}