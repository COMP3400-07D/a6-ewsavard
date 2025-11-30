#include "parta.h"
#include <stdlib.h>
#include <stdio.h>


/**
 * Initialize an array of PCBs from a raw bursts array.
 * @param bursts Pointer to an array of integers representing CPU bursts.
 * @param blen Length of the bursts array (number of processes).
 * @return Pointer to a heap-allocated array of struct pcb of length blen.
 */
struct pcb* init_procs(int* bursts, int blen) {
    struct pcb* procs = malloc(sizeof(struct pcb) * blen);
    if (!procs) return NULL;
    for (int i = 0; i < blen; ++i) {
        procs[i].pid = i;
        procs[i].burst_left = bursts[i];
        procs[i].wait = 0;
    }
    return procs;
}

/**
 * Print all PCBs (helper / debugging).
 * @param procs Pointer to array of struct pcb.
 * @param plen Number of processes.
 * @return void
 */
void printall(struct pcb* procs, int plen) {
    for (int i = 0; i < plen; ++i) {
        printf("PID %d: burst_left=%d wait=%d original=%d\n",
               procs[i].pid, procs[i].burst_left, procs[i].wait);
    }
}

/**
 * Run the specified process for a given amount of time.
 * Decreases the current process's burst_left by at most amount,
 * and increases the wait of other non-complete processes by the same actual amount.
 * @param procs Pointer to array of struct pcb.
 * @param plen Number of processes in the array.
 * @param current Index of the process to run.
 * @param amount Amount of time to run (time units).
 * @return void
 */
void run_proc(struct pcb* procs, int plen, int current, int amount) {
    if (current < 0 || current >= plen) return;
    if (procs[current].burst_left <= 0) return;
    int actual = amount;
    if (actual > procs[current].burst_left) actual = procs[current].burst_left;
    procs[current].burst_left -= actual;
    for (int i = 0; i < plen; ++i) {
        if (i == current) continue;
        if (procs[i].burst_left > 0) procs[i].wait += actual;
    }
}

/**
 * Return the next process index to run under round-robin order.
 * Starts searching from prev+1 (wrap-around). If prev < 0, start at 0.
 * If all processes are complete, return -1.
 * @param prev The previously run process index (or -1 to start at 0).
 * @param procs Pointer to array of struct pcb.
 * @param plen Number of processes.
 * @return Index of next process to run, or -1 if none remain.
 */
int rr_next(int prev, struct pcb* procs, int plen) {
    if (plen <= 0) return -1;
    int start;
    if (prev < 0) start = 0;
    else start = (prev + 1) % plen;
    for (int offset = 0; offset < plen; ++offset) {
        int idx = (start + offset) % plen;
        if (procs[idx].burst_left > 0) return idx;
    }
    return -1;
}

/**
 * Run processes using First-Come-First-Serve scheduling.
 * Start from PID 0, run each process to completion in order.
 * PCBs are updated (wait times); returns total elapsed time.
 * @param procs Pointer to array of struct pcb.
 * @param plen Number of processes.
 * @return Total time elapsed when all processes complete.
 */
int fcfs_run(struct pcb* procs, int plen) {
    int time = 0;
    for (int i = 0; i < plen; ++i) {
        if (procs[i].burst_left <= 0) continue;
        int run_amount = procs[i].burst_left;
        run_proc(procs, plen, i, run_amount);
        time += run_amount;
    }
    return time;
}

/**
 * Run processes using Round-Robin scheduling with given quantum.
 * Start from PID 0 and cycle through processes that still have burst_left.
 * PCBs are updated (wait times); returns total elapsed time.
 * @param procs Pointer to array of struct pcb.
 * @param plen Number of processes.
 * @param quantum Time quantum for RR.
 * @return Total time elapsed when all processes complete.
 */
int rr_run(struct pcb* procs, int plen, int quantum) {
    int time = 0;
    int prev = -1;
    int next = rr_next(prev, procs, plen);
    while (next != -1) {
        int amount = quantum;
        if (amount > procs[next].burst_left) amount = procs[next].burst_left;
        run_proc(procs, plen, next, amount);
        time += amount;
        prev = next;
        next = rr_next(prev, procs, plen);
    }
    return time;
}

