
#include "scheduling.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    float average_waiting;
    float average_turnaround;
} ScheduleSummary;

/* Print completion, turnaround and waiting times. */
static ScheduleSummary print_results(Job jobs[], int count)
{
    ScheduleSummary summary = {0, 0};

    printf("\n%-8s %-12s %-12s %-12s\n",
           "Job ID", "Completion", "Turnaround", "Waiting");

    for (int i = 0; i < count; i++) {
        printf("J%-7d %-12d %-12d %-12d\n",
               jobs[i].job_id,
               jobs[i].completion_time,
               jobs[i].turnaround_time,
               jobs[i].waiting_time);

        summary.average_waiting += jobs[i].waiting_time;
        summary.average_turnaround += jobs[i].turnaround_time;
    }

    if (count > 0) {
        summary.average_waiting /= count;
        summary.average_turnaround /= count;
    }

    printf("\nAverage Waiting Time: %.2f\n",
           summary.average_waiting);

    printf("Average Turnaround Time: %.2f\n",
           summary.average_turnaround);

    return summary;
}

/* FCFS and non-preemptive Priority Scheduling. */
static ScheduleSummary run_non_preemptive(
    const JobManager *manager, int priority_mode, int display)
{
    Job jobs[MAX_JOBS];
    int completed = 0;
    int time = 0;

    ScheduleSummary summary = {0, 0};

    if (manager == NULL || manager->count <= 0) {
        if (display) {
            printf("No jobs available.\n");
        }
        return summary;
    }

    memcpy(jobs, manager->jobs, manager->count * sizeof(Job));

    if (display) {
        if (priority_mode) {
            printf("\n=== Non-Preemptive Priority Scheduling ===\n");
        } else {
            printf("\n=== FCFS Scheduling ===\n");
        }

        printf("\nGantt Chart:\n");
    }

    while (completed < manager->count) {
        int selected = -1;

        for (int i = 0; i < manager->count; i++) {
            if (jobs[i].remaining_time <= 0 ||
                jobs[i].arrival_time > time) {
                continue;
            }

            if (selected == -1) {
                selected = i;
            } else if (priority_mode) {
                if (jobs[i].priority < jobs[selected].priority ||
                    (jobs[i].priority == jobs[selected].priority &&
                     jobs[i].arrival_time < jobs[selected].arrival_time) ||
                    (jobs[i].priority == jobs[selected].priority &&
                     jobs[i].arrival_time == jobs[selected].arrival_time &&
                     jobs[i].job_id < jobs[selected].job_id)) {
                    selected = i;
                }
            } else {
                if (jobs[i].arrival_time < jobs[selected].arrival_time ||
                    (jobs[i].arrival_time == jobs[selected].arrival_time &&
                     jobs[i].job_id < jobs[selected].job_id)) {
                    selected = i;
                }
            }
        }

        if (selected == -1) {
            int next_arrival = -1;

            for (int i = 0; i < manager->count; i++) {
                if (jobs[i].remaining_time > 0 &&
                    (next_arrival == -1 ||
                     jobs[i].arrival_time < next_arrival)) {
                    next_arrival = jobs[i].arrival_time;
                }
            }

            if (next_arrival == -1) {
                break;
            }

            if (display && next_arrival > time) {
                printf("%d -- IDLE -- %d\n", time, next_arrival);
            }

            time = next_arrival;
            continue;
        }

        Job *job = &jobs[selected];

        if (display) {
            printf("%d -- J%d -- %d\n",
                   time, job->job_id, time + job->remaining_time);
        }

        time += job->remaining_time;
        job->remaining_time = 0;

        job->completion_time = time;
        job->turnaround_time = time - job->arrival_time;
        job->waiting_time =
            job->turnaround_time - job->burst_time;

        completed++;
    }

    if (display) {
        summary = print_results(jobs, manager->count);
    } else {
        for (int i = 0; i < manager->count; i++) {
            summary.average_waiting += jobs[i].waiting_time;
            summary.average_turnaround += jobs[i].turnaround_time;
        }

        summary.average_waiting /= manager->count;
        summary.average_turnaround /= manager->count;
    }

    return summary;
}

void run_fcfs(const JobManager *manager)
{
    run_non_preemptive(manager, 0, 1);
}

void run_priority(const JobManager *manager)
{
    run_non_preemptive(manager, 1, 1);
}

/* Round Robin Scheduling using a circular ready queue. */
static ScheduleSummary execute_round_robin(
    const JobManager *manager, int quantum, int display)
{
    Job jobs[MAX_JOBS];
    int queue[MAX_JOBS];
    int added[MAX_JOBS] = {0};

    int front = 0;
    int rear = 0;
    int queue_count = 0;

    int completed = 0;
    int time = 0;

    ScheduleSummary summary = {0, 0};

    if (manager == NULL || manager->count <= 0) {
        if (display) {
            printf("No jobs available.\n");
        }
        return summary;
    }

    if (quantum <= 0) {
        if (display) {
            printf("Error: Time quantum must be positive.\n");
        }
        return summary;
    }

    memcpy(jobs, manager->jobs, manager->count * sizeof(Job));

    /* Add each arriving job to the ready queue only once. */
    #define ADD_ARRIVED_JOBS() \
        for (int i = 0; i < manager->count; i++) { \
            if (jobs[i].arrival_time <= time && \
                jobs[i].remaining_time > 0 && !added[i]) { \
                queue[rear] = i; \
                rear = (rear + 1) % MAX_JOBS; \
                queue_count++; \
                added[i] = 1; \
            } \
        }

    if (display) {
        printf("\n=== Round Robin Scheduling ===\n");
        printf("Time Quantum: %d\n", quantum);
        printf("\nGantt Chart:\n");
    }

    while (completed < manager->count) {

        if (queue_count == 0) {
            int next_arrival = -1;

            for (int i = 0; i < manager->count; i++) {
                if (jobs[i].remaining_time > 0 &&
                    (next_arrival == -1 ||
                     jobs[i].arrival_time < next_arrival)) {
                    next_arrival = jobs[i].arrival_time;
                }
            }

            if (next_arrival == -1) {
                break;
            }

            if (display && next_arrival > time) {
                printf("%d -- IDLE -- %d\n", time, next_arrival);
            }

            time = next_arrival;
            ADD_ARRIVED_JOBS();
        }

        int index = queue[front];
        front = (front + 1) % MAX_JOBS;
        queue_count--;

        int execution_time = jobs[index].remaining_time;

        if (execution_time > quantum) {
            execution_time = quantum;
        }

        int start_time = time;

        time += execution_time;
        jobs[index].remaining_time -= execution_time;

        if (display) {
            printf("%d -- J%d -- %d\n",
                   start_time, jobs[index].job_id, time);
        }

        /* Add newly arrived jobs before requeuing the current job. */
        ADD_ARRIVED_JOBS();

        if (jobs[index].remaining_time > 0) {
            queue[rear] = index;
            rear = (rear + 1) % MAX_JOBS;
            queue_count++;
        } else {
            jobs[index].completion_time = time;
            jobs[index].turnaround_time =
                time - jobs[index].arrival_time;
            jobs[index].waiting_time =
                jobs[index].turnaround_time - jobs[index].burst_time;

            completed++;
        }
    }

    #undef ADD_ARRIVED_JOBS

    if (display) {
        summary = print_results(jobs, manager->count);
    } else {
        for (int i = 0; i < manager->count; i++) {
            summary.average_waiting += jobs[i].waiting_time;
            summary.average_turnaround += jobs[i].turnaround_time;
        }

        summary.average_waiting /= manager->count;
        summary.average_turnaround /= manager->count;
    }

    return summary;
}

void run_round_robin(const JobManager *manager, int quantum)
{
    execute_round_robin(manager, quantum, 1);
}

/* Compare all three scheduling algorithms. */
void compare_scheduling(const JobManager *manager, int quantum)
{
    ScheduleSummary fcfs;
    ScheduleSummary rr;
    ScheduleSummary priority;

    if (manager == NULL || manager->count <= 0) {
        printf("No jobs available for comparison.\n");
        return;
    }

    if (quantum <= 0) {
        printf("Error: Time quantum must be positive.\n");
        return;
    }

    fcfs = run_non_preemptive(manager, 0, 0);
    rr = execute_round_robin(manager, quantum, 0);
    priority = run_non_preemptive(manager, 1, 0);

    printf("\n=== Scheduling Algorithm Comparison ===\n");

    printf("\n%-20s %-15s %-15s\n",
           "Algorithm", "Average WT", "Average TAT");

    printf("%-20s %-15.2f %-15.2f\n",
           "FCFS", fcfs.average_waiting,
           fcfs.average_turnaround);

    printf("%-20s %-15.2f %-15.2f\n",
           "Round Robin", rr.average_waiting,
           rr.average_turnaround);

    printf("%-20s %-15.2f %-15.2f\n",
           "Priority", priority.average_waiting,
           priority.average_turnaround);
}
