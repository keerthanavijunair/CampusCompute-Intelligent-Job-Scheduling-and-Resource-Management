#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#define SHM_NAME "/campuscompute_shm"
#define SEM_NAME "/campuscompute_sem"

#define MESSAGE_SIZE 256

typedef struct {
    char message[MESSAGE_SIZE];
    int data_ready;
} SharedData;

int writer(void);
int reader(void);

#endif
