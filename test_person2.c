
#include "logger.h"

#include <stdio.h>
#include <pthread.h>

#define PRODUCER_COUNT 3
#define MESSAGES_PER_PRODUCER 5

typedef struct {
    Logger *logger;
    int producer_id;
} ProducerArgs;

void *producer_function(void *arg)
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

int main(void)
{
    Logger logger;

    pthread_t producers[PRODUCER_COUNT];
    ProducerArgs args[PRODUCER_COUNT];

    printf("=== Producer-Consumer Logger Test ===\n");

    if (logger_init(&logger, "logs.txt") != 0) {
        printf("Failed to initialize logger.\n");
        return 1;
    }

    for (int i = 0; i < PRODUCER_COUNT; i++) {
        args[i].logger = &logger;
        args[i].producer_id = i + 1;

        if (pthread_create(&producers[i], NULL,
                           producer_function, &args[i]) != 0) {
            printf("Failed to create producer thread.\n");

            for (int j = 0; j < i; j++) {
                pthread_join(producers[j], NULL);
            }

            logger_destroy(&logger);
            return 1;
        }
    }

    for (int i = 0; i < PRODUCER_COUNT; i++) {
        pthread_join(producers[i], NULL);
    }

    logger_destroy(&logger);

    printf("\nAll producers finished.\n");
    printf("Consumer finished processing messages.\n");
    printf("Log file saved as logs.txt\n");

    return 0;
}
