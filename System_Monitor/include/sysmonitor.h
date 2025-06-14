#ifndef SYSMONITOR_H
#define SYSMONITOR_H

#include <gtk/gtk.h>
#include <cairo.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

typedef enum {
    SORT_PID,
    SORT_CPU,
    SORT_MEM
} SortMethod;

typedef struct {
    int pid;
    char name[32];
    double cpu_usage;
    double mem_usage;
} ProcessInfo;

typedef struct {
    GtkWidget *window;
    GtkWidget *cpu_drawing;
    GtkWidget *mem_drawing;
    GtkWidget *treeview;
    GtkListStore *store;
    SortMethod current_sort;
    double cpu_percent;
    double mem_percent;
} AppData;

// Function declarations
void get_cpu_usage(double *usage);
void get_memory_usage(double *total, double *used);
void init_ui(AppData *app);
void update_ui(AppData *app);
void sort_processes(ProcessInfo *processes, int count, SortMethod method);
int get_process_list(ProcessInfo *processes, int max);
gboolean update_display(gpointer user_data);
gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);

#endif
