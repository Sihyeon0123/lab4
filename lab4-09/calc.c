#include <gtk/gtk.h>

float num1 = 0, num2 = 0;
int op = -1;
int flag = 0; // 연산자를 누르면 1로 변경
char output[100] = {0};

void quit(GtkWidget *window, gpointer data) {
    gtk_main_quit();
}

void num_button_clicked(GtkWidget *button, gpointer data) {
    const gchar *button_label = gtk_button_get_label(GTK_BUTTON(button));
    if (flag == 0){
        num1 *= 10;
        num1 += atoi(button_label);
    }
    else{
        num2 *= 10;
        num2 += atoi(button_label);
    }
    // 현재 output의 길이를 계산
    size_t output_len = strlen(output);

    // output에 현재 버튼의 라벨을 추가
    if (output_len < sizeof(output) - 1) {
        output[output_len] = button_label[0];
        output[output_len + 1] = '\0'; // 문자열 끝에 널 문자 추가
    }
    gtk_label_set_text(GTK_LABEL(data), output);
}

void reset_button_clicked(GtkWidget *button, gpointer data) {
    num1 = 0;
    num2 = 0;
    op = -1;
    flag = 0;
    for(int i=0;i<100;i++)
        output[i] = '\0';
    // 라벨을 NULL으로 변경
    gtk_label_set_text(GTK_LABEL(data), "Result");
}

void op_button_clicked(GtkWidget *button, gpointer data) {
    if (flag == 0) {
        const gchar *op_label = gtk_button_get_label(GTK_BUTTON(button));
        if (g_strcmp0(op_label, "+") == 0) {
            op = 1;
        } else if (g_strcmp0(op_label, "-") == 0) {
            op = 2;
        } else if (g_strcmp0(op_label, "*") == 0) {
            op = 3;
        } else if (g_strcmp0(op_label, "/") == 0) {
            op = 4;
        }
        // 연산자를 output에 추가
    
        // 처음 연산자를 추가할 때만 추가
        if (strlen(output) < sizeof(output) - 1) {
            output[strlen(output)] = op_label[0];
            output[strlen(output) + 1] = '\0'; // 문자열 끝에 널 문자 추가
        }
        flag = 1;
        gtk_label_set_text(GTK_LABEL(data), output);
    }
}

void calc_button_clicked(GtkWidget *button, gpointer data) {
    float result = 0;
    char result_str[20];
    if(op == 1){ // +
        result = num1 + num2;
    } else if (op == 2){ // -
        result = num1 - num2;
    } else if (op == 3){ // *
        result = num1 * num2;
    } else if (op == 4){ // /
        if (num1 != 0 && num2 != 0){
            result = num1 / num2;
        } else{
            result = 0;
        }
    }

    sprintf(result_str, "%.2f", result);
    // 라벨을 NULL으로 변경
    gtk_label_set_text(GTK_LABEL(data), result_str);
    num1 = 0;
    num2 = 0;
    op = -1;
    flag = 0;
    for(int i=0;i<100;i++)
        output[i] = '\0';
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *container;
    GtkWidget *grid;
    GtkWidget *result_label;
    GtkWidget *buttons[15];
    const gchar *button_labels[] = {
        "7", "8", "9", "/",
        "4", "5", "6", "*",
        "1", "2", "3", "-",
        "0", "C", "=", "+"
    };

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Calculator");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 180);

    g_signal_connect(window, "destroy", G_CALLBACK(quit), NULL);

    container = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(window), container);

    result_label = gtk_label_new("Result");
    gtk_fixed_put(GTK_FIXED(container), result_label, 0, 15);  // (0, 0) 위치에 라벨 추가

    grid = gtk_grid_new();
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_fixed_put(GTK_FIXED(container), grid, 0, 50);

    int button_index = 0;
    // 버튼 배열 초기화 및 GtkGrid에 패킹
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            buttons[button_index] = gtk_button_new_with_label(button_labels[button_index]);
            gtk_grid_attach(GTK_GRID(grid), buttons[button_index], col, row, 1, 1);

            // 숫자 버튼에 대한 클릭 시그널과 핸들러 연결
            if (button_index == 0 || button_index == 1 || button_index == 2 || button_index == 4 || button_index == 5 || button_index == 6 ||
            button_index == 8 || button_index == 9 || button_index == 10 || button_index == 12) {
                g_signal_connect(buttons[button_index], "clicked", G_CALLBACK(num_button_clicked), result_label);
            } else if(button_index == 13){
                // 지우기 버튼을 누른다면
                g_signal_connect(buttons[button_index], "clicked", G_CALLBACK(reset_button_clicked), result_label);
            } else if(button_index == 3 || button_index == 7 || button_index == 11 || button_index == 15){
                g_signal_connect(buttons[button_index], "clicked", G_CALLBACK(op_button_clicked), result_label);
            } else if(button_index == 14){
                g_signal_connect(buttons[button_index], "clicked", G_CALLBACK(calc_button_clicked), result_label);
            }

            gtk_widget_show(buttons[button_index]);
            ++button_index;
        }
    }

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
