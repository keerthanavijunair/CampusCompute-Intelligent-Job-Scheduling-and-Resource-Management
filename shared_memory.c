
#include "shared_memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <errno.h>

int writer(void)
{
    int shm_fd;
    SharedData *data;
    sem_t *semaphore;

    // Create a new shared memory object.
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_EXCL | O_RDWR, 0600);

    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }

    // Allocate sufficient space for the SharedData structure.
    if (ftruncate(shm_fd, sizeof(SharedData)) == -1) {
        perror("ftruncate");
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return 1;
    }

    // Map shared memory into the writer's address space.
    data = mmap(NULL, sizeof(SharedData),
                PROT_READ | PROT_WRITE, MAP_SHARED,
                shm_fd, 0);

    if (data == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return 1;
    }

    // Create a named semaphore with an initial value of 0.
    semaphore = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0600, 0);

    if (semaphore == SEM_FAILED) {
        perror("sem_open");
        munmap(data, sizeof(SharedData));
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return 1;
    }

    // Write the message into shared memory.
    snprintf(data->message, MESSAGE_SIZE,
             "Hello from the CampusCompute writer process!");

    data->data_ready = 1;

    printf("Writer: Message written to shared memory.\n");
    printf("Writer: %s\n", data->message);

    // Signal the semaphore to notify the reader.
    if (sem_post(semaphore) == -1) {
        perror("sem_post");
        sem_close(semaphore);
        sem_unlink(SEM_NAME);
        munmap(data, sizeof(SharedData));
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return 1;
    }

    printf("Writer: Semaphore signal sent.\n");

    // Release local resources. The named IPC objects remain
    // available for the reader until it completes communication.
    sem_close(semaphore);
    munmap(data, sizeof(SharedData));
    close(shm_fd);

    return 0;
}

int reader(void)
{
    int shm_fd;
    SharedData *data;
    sem_t *semaphore;
    int status = 0;

    // Open the shared memory object created by the writer.
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0600);

    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }

    // Map the shared memory into the reader's address space.
    data = mmap(NULL, sizeof(SharedData),
                PROT_READ | PROT_WRITE, MAP_SHARED,
                shm_fd, 0);

    if (data == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        return 1;
    }

    // Open the existing named semaphore.
    semaphore = sem_open(SEM_NAME, 0);

    if (semaphore == SEM_FAILED) {
        perror("sem_open");
        munmap(data, sizeof(SharedData));
        close(shm_fd);
        return 1;
    }

    printf("Reader: Waiting for writer...\n");

    // Wait until the writer signals that the message is ready.
    // Retry if the wait is interrupted by a signal.
    int wait_result;

    do {
        wait_result = sem_wait(semaphore);
    } while (wait_result == -1 && errno == EINTR);

    if (wait_result == -1) {
        perror("sem_wait");
        status = 1;
    } else {
        printf("Reader: Semaphore signal received.\n");

        // Check whether the writer has placed valid data in memory.
        if (data->data_ready == 1) {
            printf("Reader: Message received from shared memory:\n");
            printf("%s\n", data->message);
        } else {
            fprintf(stderr, "Reader: Shared memory contains no valid message.\n");
            status = 1;
        }
    }

    // Release local resources before removing the IPC objects.
    if (sem_close(semaphore) == -1) {
        perror("sem_close");
        status = 1;
    }

    if (munmap(data, sizeof(SharedData)) == -1) {
        perror("munmap");
        status = 1;
    }

    if (close(shm_fd) == -1) {
        perror("close");
        status = 1;
    }

    // Remove only the IPC objects belonging to this communication.
    // Cleanup happens only after successful communication.
    if (status == 0) {
        if (sem_unlink(SEM_NAME) == -1) {
            perror("sem_unlink");
            status = 1;
        }

        if (shm_unlink(SHM_NAME) == -1) {
            perror("shm_unlink");
            status = 1;
        }
    }

    if (status == 0) {
        printf("Reader: Shared memory and semaphore cleaned up.\n");
    }

    return status;
}

#ifndef CAMPUSCOMPUTE_INTEGRATED
int main(int argc, char *argv[])
{
    // Check that the user supplied exactly one argument.
    if (argc != 2) {
        printf("Usage: %s writer|reader\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "writer") == 0) {
        return writer();
    }

    if (strcmp(argv[1], "reader") == 0) {
        return reader();
    }

    printf("Invalid option. Use writer or reader.\n");
    return 1;
}
#endif
