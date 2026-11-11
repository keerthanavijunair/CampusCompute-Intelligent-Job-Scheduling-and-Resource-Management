
#ifndef SCHEDULING_H
#define SCHEDULING_H

#include "jobs.h"

void run_fcfs(const JobManager *manager);

void run_round_robin(const JobManager *manager, int quantum);

void run_priority(const JobManager *manager);

void compare_scheduling(const JobManager *manager, int quantum);

#endif
