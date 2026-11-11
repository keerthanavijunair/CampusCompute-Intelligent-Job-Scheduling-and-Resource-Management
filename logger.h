
#ifndef LOGGER_H
#define LOGGER_H

#include <pthread.h>

#define BUFFER_SIZE 10
#define MESSAGE_SIZE 256

typedef struct {
    char messages[BUFFER_SIZE][MESSAGE_SIZE];

    int front;
    int rear;
    int count;

    int stopping;

    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;

    pthread_t consumer_thread;

    char filename[256];
} Logger;

int logger_init(Logger *logger, const char *filename);

int logger_log(Logger *logger, const char *message);

void logger_destroy(Logger *logger);

#endif
