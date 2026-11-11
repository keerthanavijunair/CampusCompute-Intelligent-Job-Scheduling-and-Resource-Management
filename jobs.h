#ifndef JOBS_H
#define JOBS_H

#define MAX_JOBS 20

typedef enum {
    READY,
    RUNNING,
    COMPLETED
} JobStatus;

typedef struct {
    int job_id;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int priority;

    int completion_time;
    int turnaround_time;
    int waiting_time;

    JobStatus status;
} Job;

typedef struct {
    Job jobs[MAX_JOBS];
    int count;
    int next_job_id;
} JobManager;

void init_job_manager(JobManager *manager);

int add_job(JobManager *manager, int arrival_time,
            int burst_time, int priority);

int update_job_status(JobManager *manager, int job_id,
                      JobStatus status);

void display_jobs(const JobManager *manager);

#endif
