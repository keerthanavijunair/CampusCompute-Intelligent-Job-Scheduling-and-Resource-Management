#include "jobs.h"

#include <stdio.h>
#include <string.h>

void init_job_manager(JobManager *manager)
{
    if (manager == NULL) {
        return;
    }

    memset(manager, 0, sizeof(JobManager));
    manager->next_job_id = 1;
}

int add_job(JobManager *manager, int arrival_time,
            int burst_time, int priority)
{
    Job *job;

    if (manager == NULL) {
        return -1;
    }

    if (manager->count >= MAX_JOBS) {
        printf("Error: Maximum job limit reached.\n");
        return -1;
    }

    if (arrival_time < 0 || burst_time <= 0 || priority <= 0) {
        printf("Error: Invalid job details.\n");
        return -1;
    }

    job = &manager->jobs[manager->count];

    job->job_id = manager->next_job_id++;
    job->arrival_time = arrival_time;
    job->burst_time = burst_time;
    job->remaining_time = burst_time;
    job->priority = priority;

    job->completion_time = 0;
    job->turnaround_time = 0;
    job->waiting_time = 0;

    job->status = READY;

    manager->count++;

    printf("Job J%d added successfully.\n", job->job_id);

    return job->job_id;
}

int update_job_status(JobManager *manager, int job_id,
                      JobStatus status)
{
    if (manager == NULL) {
        return -1;
    }

    if (status < READY || status > COMPLETED) {
        return -1;
    }

    for (int i = 0; i < manager->count; i++) {
        if (manager->jobs[i].job_id == job_id) {

            if (manager->jobs[i].status == COMPLETED) {
                printf("Error: Job is already completed.\n");
                return -1;
            }

            manager->jobs[i].status = status;
            return 0;
        }
    }

    printf("Error: Job not found.\n");
    return -1;
}

void display_jobs(const JobManager *manager)
{
    if (manager == NULL) {
        return;
    }

    if (manager->count == 0) {
        printf("No jobs available.\n");
        return;
    }

    printf("\n========== JOB DETAILS ==========\n");

    printf("%-8s %-10s %-10s %-10s %-12s\n",
           "Job ID", "Arrival", "Burst", "Priority", "Status");

    for (int i = 0; i < manager->count; i++) {
        const Job *job = &manager->jobs[i];

        const char *status;

        switch (job->status) {
            case READY:
                status = "READY";
                break;

            case RUNNING:
                status = "RUNNING";
                break;

            case COMPLETED:
                status = "COMPLETED";
                break;

            default:
                status = "UNKNOWN";
        }

        printf("J%-7d %-10d %-10d %-10d %-12s\n",
               job->job_id,
               job->arrival_time,
               job->burst_time,
               job->priority,
               status);
    }
}
