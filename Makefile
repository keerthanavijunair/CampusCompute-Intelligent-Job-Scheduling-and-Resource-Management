CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pthread

all: campuscompute test_person1 test_person2 shared_memory test_process test_resource

campuscompute: main.c jobs.c scheduling.c bankers.c logger.c shared_memory.c process_manager.c resource_limits.c
	$(CC) $(CFLAGS) -DCAMPUSCOMPUTE_INTEGRATED main.c jobs.c scheduling.c bankers.c logger.c shared_memory.c process_manager.c resource_limits.c -o campuscompute -lrt

test_person1: test_person1.c jobs.c scheduling.c bankers.c
	$(CC) $(CFLAGS) test_person1.c jobs.c scheduling.c bankers.c -o test_person1

test_person2: test_person2.c logger.c logger.h
	$(CC) $(CFLAGS) test_person2.c logger.c -o test_person2

shared_memory: shared_memory.c shared_memory.h
	$(CC) $(CFLAGS) shared_memory.c -o shared_memory -lrt

test_process: test_process.c process_manager.c process_manager.h
	$(CC) $(CFLAGS) test_process.c process_manager.c -o test_process

test_resource: test_resource.c resource_limits.c resource_limits.h
	$(CC) $(CFLAGS) test_resource.c resource_limits.c -o test_resource

clean:
	rm -f campuscompute test_person1 test_person2 shared_memory test_process test_resource logs.txt

.PHONY: all clean
