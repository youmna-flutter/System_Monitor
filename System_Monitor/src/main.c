#include "../include/sysmonitor.h"

int main(int argc, char *argv[]) {
    AppData app = {0};
    app.current_sort = SORT_PID;
    
    gtk_init(&argc, &argv);
    init_ui(&app);
    
    // Set up timer for periodic updates (500ms)
    g_timeout_add(500, update_display, &app);
    
    gtk_main();
    return 0;
}
