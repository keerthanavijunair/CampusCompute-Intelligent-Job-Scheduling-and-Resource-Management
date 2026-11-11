
#include "logger.h"

#include <stdio.h>
#include <string.h>

static void *consumer_function(void *arg)
{
    Logger *logger = (Logger *)arg;
    char message[MESSAGE_SIZE];

    while (1) {
        if (pthread_mutex_lock(&logger->mutex) != 0) {
            perror("Failed to lock mutex");
            break;
        }

        // Wait while the buffer is empty and the logger is active.
        while (logger->count == 0 && !logger->stopping) {
            if (pthread_cond_wait(&logger->not_empty,
                                  &logger->mutex) != 0) {
                fprintf(stderr, "Consumer: Condition wait failed.\n");
                pthread_mutex_unlock(&logger->mutex);
                return NULL;
            }
        }

        // Stop only after all queued messages have been processed.
        if (logger->count == 0 && logger->stopping) {
            pthread_mutex_unlock(&logger->mutex);
            break;
        }

        // Remove the oldest message from the circular buffer.
        snprintf(message, MESSAGE_SIZE, "%s",
                 logger->messages[logger->front]);

        logger->front = (logger->front + 1) % BUFFER_SIZE;
        logger->count--;

        // Notify producers that a buffer slot is available.
        pthread_cond_signal(&logger->not_full);

        pthread_mutex_unlock(&logger->mutex);

        printf("Consumer processed: %s\n", message);

        // Append the processed message to the log file.
        FILE *file = fopen(logger->filename, "a");

        if (file == NULL) {
            perror("Failed to open log file");
        } else {
            if (fprintf(file, "%s\n", message) < 0) {
                perror("Failed to write to log file");
            }

            if (fclose(file) != 0) {
                perror("Failed to close log file");
            }
        }
    }

    return NULL;
}

int logger_init(Logger *logger, const char *filename)
{
    if (logger == NULL || filename == NULL || filename[0] == '\0') {
        fprintf(stderr, "Invalid logger or filename.\n");
        return -1;
    }

    memset(logger, 0, sizeof(Logger));

    // Store the log filename safely.
    int length = snprintf(logger->filename, sizeof(logger->filename),
                          "%s", filename);

    if (length < 0 || (size_t)length >= sizeof(logger->filename)) {
        fprintf(stderr, "Log filename is too long.\n");
        return -1;
    }

    // Initialize the mutex used to protect the shared buffer.
    if (pthread_mutex_init(&logger->mutex, NULL) != 0) {
        fprintf(stderr, "Failed to initialize mutex.\n");
        return -1;
    }

    // Initialize condition variables for buffer synchronization.
    if (pthread_cond_init(&logger->not_empty, NULL) != 0) {
        fprintf(stderr, "Failed to initialize not_empty condition.\n");
        pthread_mutex_destroy(&logger->mutex);
        return -1;
    }

    if (pthread_cond_init(&logger->not_full, NULL) != 0) {
        fprintf(stderr, "Failed to initialize not_full condition.\n");
        pthread_cond_destroy(&logger->not_empty);
        pthread_mutex_destroy(&logger->mutex);
        return -1;
    }

    // Create or clear the log file before starting the consumer.
    FILE *file = fopen(logger->filename, "w");

    if (file == NULL) {
        perror("Failed to create log file");
        pthread_cond_destroy(&logger->not_full);
        pthread_cond_destroy(&logger->not_empty);
        pthread_mutex_destroy(&logger->mutex);
        return -1;
    }

    if (fclose(file) != 0) {
        perror("Failed to close log file");
        pthread_cond_destroy(&logger->not_full);
        pthread_cond_destroy(&logger->not_empty);
        pthread_mutex_destroy(&logger->mutex);
        return -1;
    }

    // Start the consumer thread.
    if (pthread_create(&logger->consumer_thread, NULL,
                       consumer_function, logger) != 0) {
        fprintf(stderr, "Failed to create consumer thread.\n");
        pthread_cond_destroy(&logger->not_full);
        pthread_cond_destroy(&logger->not_empty);
        pthread_mutex_destroy(&logger->mutex);
        return -1;
    }

    return 0;
}

int logger_log(Logger *logger, const char *message)
{
    if (logger == NULL || message == NULL) {
        fprintf(stderr, "Invalid logger or message.\n");
        return -1;
    }

    if (pthread_mutex_lock(&logger->mutex) != 0) {
        fprintf(stderr, "Producer: Failed to lock mutex.\n");
        return -1;
    }

    // Wait until the consumer frees a slot in the buffer.
    while (logger->count == BUFFER_SIZE && !logger->stopping) {
        if (pthread_cond_wait(&logger->not_full,
                              &logger->mutex) != 0) {
            fprintf(stderr, "Producer: Condition wait failed.\n");
            pthread_mutex_unlock(&logger->mutex);
            return -1;
        }
    }

    if (logger->stopping) {
        pthread_mutex_unlock(&logger->mutex);
        return -1;
    }

    // Insert the new message at the rear of the circular buffer.
    snprintf(logger->messages[logger->rear],
             MESSAGE_SIZE, "%s", message);

    logger->rear = (logger->rear + 1) % BUFFER_SIZE;
    logger->count++;

    // Notify the consumer that a message is available.
    pthread_cond_signal(&logger->not_empty);

    pthread_mutex_unlock(&logger->mutex);

    return 0;
}

void logger_destroy(Logger *logger)
{
    if (logger == NULL) {
        return;
    }

    // Signal the consumer to stop after processing remaining messages.
    if (pthread_mutex_lock(&logger->mutex) != 0) {
        fprintf(stderr, "Failed to lock mutex during cleanup.\n");
        return;
    }

    logger->stopping = 1;

    pthread_cond_broadcast(&logger->not_empty);
    pthread_cond_broadcast(&logger->not_full);

    pthread_mutex_unlock(&logger->mutex);

    // Wait for the consumer thread to finish before destroying resources.
    if (pthread_join(logger->consumer_thread, NULL) != 0) {
        fprintf(stderr, "Failed to join consumer thread.\n");
    }

    pthread_cond_destroy(&logger->not_full);
    pthread_cond_destroy(&logger->not_empty);
    pthread_mutex_destroy(&logger->mutex);
}
