#include <gtk/gtk.h>

void on_file_selected(GtkFileChooserButton *filechooserbutton, gpointer user_data) {
    const gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filechooserbutton));
    char *filename_utf8 = g_strdup(filename);
    g_print("선택된 파일: %s\n", filename_utf8);

    // 메모리 누수를 피하기 위해 복사본을 사용한 후에는 메모리를 해제합니다.
    g_free(filename_utf8);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *file_chooser_button = gtk_file_chooser_button_new("파일 선택", GTK_FILE_CHOOSER_ACTION_OPEN);
    g_signal_connect(file_chooser_button, "file-set", G_CALLBACK(on_file_selected), NULL);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(box), file_chooser_button, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(window), box);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
