#include "../include/sysmonitor.h"

void get_process_stats(ProcessInfo *p) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/stat", p->pid);

    FILE *fp = fopen(path, "r");
    if (fp) {
        unsigned long utime, stime;
        long rss;
        
        fscanf(fp, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu",
               &utime, &stime);
        fseek(fp, 0, SEEK_SET);
        
        // Read RSS (page count)
        for (int i = 0; i < 23; i++) fscanf(fp, "%*s");
        fscanf(fp, "%ld", &rss);
        
        p->cpu_usage = (utime + stime) / (double)sysconf(_SC_CLK_TCK);
        p->mem_usage = (rss * sysconf(_SC_PAGESIZE)) / (1024.0 * 1024.0);
        fclose(fp);
    }

    snprintf(path, sizeof(path), "/proc/%d/comm", p->pid);
    FILE *name_fp = fopen(path, "r");
    if (name_fp) {
        fgets(p->name, sizeof(p->name), name_fp);
        p->name[strcspn(p->name, "\n")] = '\0';
        fclose(name_fp);
    } else {
        strncpy(p->name, "Unknown", sizeof(p->name));
    }
}

int get_process_list(ProcessInfo *processes, int max) {
    DIR *dir = opendir("/proc");
    struct dirent *entry;
    int count = 0;

    while ((entry = readdir(dir)) != NULL && count < max) {
        if (entry->d_type == DT_DIR && atoi(entry->d_name) != 0) {
            processes[count].pid = atoi(entry->d_name);
            get_process_stats(&processes[count]);
            count++;
        }
    }

    closedir(dir);
    return count;
}
