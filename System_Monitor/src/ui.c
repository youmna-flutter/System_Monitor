#include "../include/sysmonitor.h"

// Draw circular gauge
gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    AppData *app = user_data;
    gboolean is_cpu = (widget == app->cpu_drawing);
    double percent = is_cpu ? app->cpu_percent : app->mem_percent;
    const char *label = is_cpu ? "CPU" : "Memory";
    
    GtkAllocation alloc;
    gtk_widget_get_allocation(widget, &alloc);
    int width = alloc.width;
    int height = alloc.height;
    int radius = MIN(width, height) * 0.4;
    int center_x = width / 2;
    int center_y = height / 2;
    
    // Set dark background
    cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
    cairo_paint(cr);
    
    // Draw outer ring (background)
    cairo_set_line_width(cr, 10);
    cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 0.5);
    cairo_arc(cr, center_x, center_y, radius, 0, 2 * M_PI);
    cairo_stroke(cr);
    
    // Draw progress ring
    cairo_set_line_width(cr, 10);
    if (is_cpu) {
        cairo_set_source_rgb(cr, 0.0, 0.8, 0.0); // Green for CPU
    } else {
        cairo_set_source_rgb(cr, 0.8, 0.5, 0.0); // Orange for Memory
    }
    cairo_arc(cr, center_x, center_y, radius, 
              -M_PI/2, -M_PI/2 + 2 * M_PI * percent / 100);
    cairo_stroke(cr);
    
    // Draw center label
    char text[32];
    snprintf(text, sizeof(text), "%s\n%.1f%%", label, percent);
    
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 14);
    
    cairo_text_extents_t extents;
    cairo_text_extents(cr, text, &extents);
    
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_move_to(cr, center_x - extents.width/2, center_y + extents.height/2);
    cairo_show_text(cr, text);
    
    return FALSE;
}

void on_sort_clicked(GtkButton *button, gpointer user_data) {
    AppData *app = user_data;
    const char *label = gtk_button_get_label(button);
    
    if (strcmp(label, "Sort by PID") == 0) {
        app->current_sort = SORT_PID;
    } else if (strcmp(label, "Sort by CPU") == 0) {
        app->current_sort = SORT_CPU;
    } else if (strcmp(label, "Sort by Memory") == 0) {
        app->current_sort = SORT_MEM;
    }
    
    update_ui(app);
}

void init_ui(AppData *app) {
    // Create main window with dark theme
    app->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app->window), "System Monitor");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 800, 600);
    g_signal_connect(app->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    // Apply dark theme
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, 
        "button { color: #000000; }", -1, NULL);
	gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    // Main container
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(app->window), vbox);
    
    // Gauges container
    GtkWidget *gauges_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_box_pack_start(GTK_BOX(vbox), gauges_box, FALSE, FALSE, 10);
    
    // CPU Gauge
    GtkWidget *cpu_frame = gtk_frame_new("CPU Usage");
    app->cpu_drawing = gtk_drawing_area_new();
    gtk_widget_set_size_request(app->cpu_drawing, 200, 200);
    g_signal_connect(app->cpu_drawing, "draw", G_CALLBACK(on_draw), app);
    gtk_container_add(GTK_CONTAINER(cpu_frame), app->cpu_drawing);
    gtk_box_pack_start(GTK_BOX(gauges_box), cpu_frame, TRUE, TRUE, 0);
    
    // Memory Gauge
    GtkWidget *mem_frame = gtk_frame_new("Memory Usage");
    app->mem_drawing = gtk_drawing_area_new();
    gtk_widget_set_size_request(app->mem_drawing, 200, 200);
    g_signal_connect(app->mem_drawing, "draw", G_CALLBACK(on_draw), app);
    gtk_container_add(GTK_CONTAINER(mem_frame), app->mem_drawing);
    gtk_box_pack_start(GTK_BOX(gauges_box), mem_frame, TRUE, TRUE, 0);
    
    // Sort buttons
    GtkWidget *button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(button_box), GTK_BUTTONBOX_CENTER);
    gtk_box_pack_start(GTK_BOX(vbox), button_box, FALSE, FALSE, 5);
    
    GtkWidget *pid_button = gtk_button_new_with_label("Sort by PID");
    GtkWidget *cpu_button = gtk_button_new_with_label("Sort by CPU");
    GtkWidget *mem_button = gtk_button_new_with_label("Sort by Memory");
    
    g_signal_connect(pid_button, "clicked", G_CALLBACK(on_sort_clicked), app);
    g_signal_connect(cpu_button, "clicked", G_CALLBACK(on_sort_clicked), app);
    g_signal_connect(mem_button, "clicked", G_CALLBACK(on_sort_clicked), app);
    
    gtk_container_add(GTK_CONTAINER(button_box), pid_button);
    gtk_container_add(GTK_CONTAINER(button_box), cpu_button);
    gtk_container_add(GTK_CONTAINER(button_box), mem_button);
    
    // Process list
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);
    
    app->store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_STRING, 
                                   G_TYPE_STRING, G_TYPE_STRING);
    app->treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(app->store));
    
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    
    // PID column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("PID", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app->treeview), column);
    
    // Name column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app->treeview), column);
    
    // CPU column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("CPU %", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app->treeview), column);
    
    // Memory column
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Memory (MB)", renderer, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app->treeview), column);
    
    gtk_container_add(GTK_CONTAINER(scrolled), app->treeview);
    
    gtk_widget_show_all(app->window);
}

void update_ui(AppData *app) {
    // Get system stats
    double mem_total;
    get_cpu_usage(&app->cpu_percent);
    get_memory_usage(&mem_total, &app->mem_percent);
    app->mem_percent = (app->mem_percent / mem_total) * 100;
    
    // Redraw gauges
    gtk_widget_queue_draw(app->cpu_drawing);
    gtk_widget_queue_draw(app->mem_drawing);
    
    // Get and sort processes
    ProcessInfo processes[100];
    int process_count = get_process_list(processes, 100);
    sort_processes(processes, process_count, app->current_sort);
    
    // Update process list
    gtk_list_store_clear(app->store);
    for (int i = 0; i < process_count && i < 50; i++) {
        gtk_list_store_insert_with_values(app->store, NULL, -1,
            0, g_strdup_printf("%d", processes[i].pid),
            1, processes[i].name,
            2, g_strdup_printf("%.1f", processes[i].cpu_usage),
            3, g_strdup_printf("%.1f", processes[i].mem_usage),
            -1);
    }
}

gboolean update_display(gpointer user_data) {
    AppData *app = user_data;
    update_ui(app);
    return TRUE;
}
