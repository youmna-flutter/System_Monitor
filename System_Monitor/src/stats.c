#include "../include/sysmonitor.h"

static int compare_processes(const void *a, const void *b, gpointer user_data) {
    const ProcessInfo *pa = a;
    const ProcessInfo *pb = b;
    SortMethod method = GPOINTER_TO_INT(user_data);
    
    switch(method) {
        case SORT_CPU:
            return (pb->cpu_usage > pa->cpu_usage) ? 1 : -1;
        case SORT_MEM:
            return (pb->mem_usage > pa->mem_usage) ? 1 : -1;
        default: // SORT_PID
            return pa->pid - pb->pid;
    }
}

void sort_processes(ProcessInfo *processes, int count, SortMethod method) {
    g_qsort_with_data(processes, count, sizeof(ProcessInfo), 
                     compare_processes, GINT_TO_POINTER(method));
}

void get_cpu_usage(double *usage) {
    static unsigned long long last_total = 0, last_idle = 0;
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) return;

    char line[256];
    fgets(line, sizeof(line), fp);
    
    unsigned long long user, nice, system, idle, iowait, irq, softirq;
    sscanf(line + 5, "%llu %llu %llu %llu %llu %llu %llu",
           &user, &nice, &system, &idle, &iowait, &irq, &softirq);
    
    unsigned long long total = user + nice + system + idle + iowait + irq + softirq;
    unsigned long long total_diff = total - last_total;
    unsigned long long idle_diff = idle - last_idle;
    
    *usage = 100.0 * (total_diff - idle_diff) / total_diff;
    
    last_total = total;
    last_idle = idle;
    fclose(fp);
}

void get_memory_usage(double *total, double *used) {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) return;

    char line[128];
    long mem_total = 0, mem_free = 0, buffers = 0, cached = 0;
    
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "MemTotal: %ld kB", &mem_total) == 1) continue;
        if (sscanf(line, "MemFree: %ld kB", &mem_free) == 1) continue;
        if (sscanf(line, "Buffers: %ld kB", &buffers) == 1) continue;
        if (sscanf(line, "Cached: %ld kB", &cached) == 1) continue;
    }
    
    *total = mem_total / 1024.0;
    *used = (mem_total - mem_free - buffers - cached) / 1024.0;
    fclose(fp);
}
