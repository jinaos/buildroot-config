#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Function prototypes
void toggle_hotspot(GtkSwitch *switch_widget, gpointer user_data);
void clean_cache(GtkButton *button, gpointer user_data);
void change_password(GtkButton *button, gpointer user_data);
void view_logs(GtkButton *button, gpointer user_data);

// Network section
void toggle_hotspot(GtkSwitch *switch_widget, gpointer user_data) {
    if (gtk_switch_get_active(switch_widget)) {
        system("sudo /etc/init.d/S45hostapd start");
    } else {
        system("sudo /etc/init.d/S45hostapd stop");
    }
}

// Storage section
void clean_cache(GtkButton *button, gpointer user_data) {
    system("sudo jpm clean 30");
    GtkWidget *dialog = gtk_message_dialog_new(NULL,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "Cache cleaned successfully");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Security section
void change_password(GtkButton *button, gpointer user_data) {
    system("foot -e passwd");
}

// Developer section
void view_logs(GtkButton *button, gpointer user_data) {
    system("foot -e tail -f /var/log/messages");
}

// Create info row
GtkWidget* create_info_row(const char *label_text, const char *value) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *label = gtk_label_new(label_text);
    GtkWidget *value_label = gtk_label_new(value);
    gtk_widget_set_size_request(label, 150, -1);
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), value_label, FALSE, FALSE, 0);
    return box;
}

// Create switch row
GtkWidget* create_switch_row(const char *label_text, GtkSwitch **switch_ptr) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *label = gtk_label_new(label_text);
    GtkWidget *switch_widget = gtk_switch_new();
    gtk_widget_set_size_request(label, 150, -1);
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), switch_widget, FALSE, FALSE, 0);
    if (switch_ptr) *switch_ptr = GTK_SWITCH(switch_widget);
    return box;
}

// Create button row
GtkWidget* create_button_row(const char *label_text, GCallback callback) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *label = gtk_label_new(label_text);
    GtkWidget *button = gtk_button_new_with_label("Configure");
    gtk_widget_set_size_request(label, 150, -1);
    g_signal_connect(button, "clicked", callback, NULL);
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);
    return box;
}

// Create section frame
GtkWidget* create_section(const char *title, GtkWidget *parent) {
    GtkWidget *frame = gtk_frame_new(title);
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_container_add(GTK_CONTAINER(frame), vbox);
    gtk_box_pack_start(GTK_BOX(parent), frame, FALSE, FALSE, 5);
    return vbox;
}

// Get IP address
char* get_ip_address() {
    FILE *fp = popen("ip -4 addr show wlan0 | grep inet | grep -v 127.0.0.1 | awk '{print $2}' | cut -d/ -f1", "r");
    static char ip[64] = "Not connected";
    if (fp) {
        if (fgets(ip, sizeof(ip), fp)) {
            ip[strcspn(ip, "\n")] = 0;
        }
        pclose(fp);
    }
    return ip;
}

// Get battery level
char* get_battery_level() {
    FILE *fp = fopen("/sys/class/power_supply/battery/capacity", "r");
    static char level[16] = "Unknown";
    if (fp) {
        if (fgets(level, sizeof(level), fp)) {
            level[strcspn(level, "\n")] = 0;
            strcat(level, "%");
        }
        fclose(fp);
    }
    return level;
}

// Get storage free
char* get_storage_free() {
    FILE *fp = popen("df -h / | awk 'NR==2 {print $4}'", "r");
    static char free[64] = "Unknown";
    if (fp) {
        if (fgets(free, sizeof(free), fp)) {
            free[strcspn(free, "\n")] = 0;
        }
        pclose(fp);
    }
    return free;
}

// Get kernel version
char* get_kernel() {
    FILE *fp = popen("uname -r", "r");
    static char kernel[64] = "Unknown";
    if (fp) {
        if (fgets(kernel, sizeof(kernel), fp)) {
            kernel[strcspn(kernel, "\n")] = 0;
        }
        pclose(fp);
    }
    return kernel;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "JinaOS Settings");
    gtk_window_set_default_size(GTK_WINDOW(window), 720, 1280);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(main_vbox), 20);
    gtk_container_add(GTK_CONTAINER(window), main_vbox);
    
    // Header
    GtkWidget *header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header), "<span size='x-large' weight='bold'>JinaOS Settings</span>");
    gtk_box_pack_start(GTK_BOX(main_vbox), header, FALSE, FALSE, 0);
    
    // Scrolled window
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(main_vbox), scrolled, TRUE, TRUE, 0);
    
    GtkWidget *settings_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(scrolled), settings_box);
    
    // Network section
    GtkWidget *network_box = create_section("Network", settings_box);
    GtkSwitch *hotspot_switch;
    gtk_box_pack_start(GTK_BOX(network_box), create_switch_row("WiFi Hotspot", &hotspot_switch), FALSE, FALSE, 0);
    g_signal_connect(hotspot_switch, "notify::active", G_CALLBACK(toggle_hotspot), NULL);
    gtk_box_pack_start(GTK_BOX(network_box), create_info_row("IP Address", get_ip_address()), FALSE, FALSE, 0);
    
    // Storage section
    GtkWidget *storage_box = create_section("Storage", settings_box);
    gtk_box_pack_start(GTK_BOX(storage_box), create_info_row("Cache Size", "Run jpm cache-stats"), FALSE, FALSE, 0);
    GtkWidget *clean_btn = gtk_button_new_with_label("Clean Cache Now");
    g_signal_connect(clean_btn, "clicked", G_CALLBACK(clean_cache), NULL);
    gtk_box_pack_start(GTK_BOX(storage_box), clean_btn, FALSE, FALSE, 5);
    
    // System section
    GtkWidget *system_box = create_section("System", settings_box);
    gtk_box_pack_start(GTK_BOX(system_box), create_info_row("JinaOS Version", "1.0"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(system_box), create_info_row("Kernel", get_kernel()), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(system_box), create_info_row("Storage Free", get_storage_free()), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(system_box), create_info_row("Battery", get_battery_level()), FALSE, FALSE, 0);
    
    // Security section
    GtkWidget *security_box = create_section("Security", settings_box);
    gtk_box_pack_start(GTK_BOX(security_box), create_button_row("Change Password", G_CALLBACK(change_password)), FALSE, FALSE, 0);
    
    // Developer section
    GtkWidget *dev_box = create_section("Developer", settings_box);
    gtk_box_pack_start(GTK_BOX(dev_box), create_button_row("View Logs", G_CALLBACK(view_logs)), FALSE, FALSE, 0);
    
    // Close button
    GtkWidget *close_btn = gtk_button_new_with_label("Close");
    g_signal_connect(close_btn, "clicked", G_CALLBACK(gtk_main_quit), NULL);
    gtk_box_pack_start(GTK_BOX(main_vbox), close_btn, FALSE, FALSE, 10);
    
    gtk_widget_show_all(window);
    gtk_main();
    
    return 0;
}