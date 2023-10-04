#include <gtk/gtk.h>

static void activate(GtkApplication* app, gpointer user_data) {
    GtkWidget *window;

    window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (window), "PNS interpreter");
    gtk_window_set_default_size (GTK_WINDOW (window), 1280, 720);

    GtkWidget* grid;
    grid = gtk_grid_new();

    GtkWidget* code;
    code = gtk_text_view_new();

    GtkWidget* output;
    output = gtk_text_view_new();

    gtk_grid_attach(GTK_GRID(grid), code, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), output, 0, 1, 1, 1);

    gtk_window_set_child(GTK_WINDOW(window), grid);

    gtk_widget_set_visible(window, true);
}

int main(int argc, char** argv) {
    GtkApplication* app;
    int status;

    app = gtk_application_new("net.chardiny.romain.pns-interpreter", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
    status = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);

    return status;
}