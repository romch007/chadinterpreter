#include <gtk/gtk.h>

static void activate(GtkApplication* app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new (app);;

    gtk_window_set_title (GTK_WINDOW (window), "PNS interpreter");
    gtk_window_set_default_size (GTK_WINDOW (window), 1280, 720);

    GtkWidget* layout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    // GtkWidget* code = gtk_text_view_new();
    // GtkWidget* output = gtk_text_view_new();
    GtkWidget* code = gtk_button_new_with_label("Test1");
    GtkWidget* output = gtk_button_new_with_label("Test2");

    gtk_box_append(GTK_BOX(layout), code);
    gtk_box_append(GTK_BOX(layout), output);

    gtk_window_set_child(GTK_WINDOW(window), layout);

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