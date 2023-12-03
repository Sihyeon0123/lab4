
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
// UI
#include <gtk/gtk.h>

#define BUFFER_SIZE 1024    // 메시지 버퍼 크기 정의
#define NICKNAME_LEN 32     // 닉네임 최대 길이 정의
#define FILE_SIZE 256 // 파일 사이즈

int sock; // 클라이언트 소켓
char nickname[NICKNAME_LEN];    // 사용자 닉네임

GtkWidget *window;              // 메인창
GtkWidget *send_button;         // 보내기 버튼
GtkWidget *text_view;           // 채팅 로그 박스
GtkWidget *chat_entry;          // 메시지 입력창

void *receive_message(void *socket);
void show_nickname_dialog(GtkWidget *parent);
void send_button_clicked(GtkWidget *widget, gpointer data);                 
void append_text_to_textview(GtkTextView *textview, const gchar *text);

int main(int argc, char *argv[]) {
    struct sockaddr_in server_addr; // 서버 주소 구조체
    pthread_t recv_thread;          // 수신 스레드
    char message[BUFFER_SIZE];      // 메시지 입력 버퍼
    int port = 50001;                // 포트번호

    // 소켓 생성
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        return -1;
    }

    // 서버 주소 설정
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");   // 서버의 IP 주소
    server_addr.sin_port = htons(port);                     // 서버의 포트 번호

    // 서버에 연결
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        return -1;
    }

    gtk_init(&argc, &argv);

    // 윈도우 생성
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "chatting");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_widget_set_size_request(window, 520, 520);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // GtkFixed 위젯 생성
    GtkWidget *fixed = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(window), fixed);

    text_view = gtk_text_view_new(); // GtkTextView 위젯 생성

    // 텍스트 뷰의 스크롤 가능 여부 설정 (선택 사항)
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                GTK_POLICY_AUTOMATIC,
                                GTK_POLICY_AUTOMATIC);

    // GtkTextView에 사용자 입력 비활성화 및 커서 숨김 설정
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE); // 입력 비활성화
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE); // 커서 숨김
    // 부모 위젯에 추가 (예시: fixed라는 GtkFixed 위젯을 부모로 추가)
    gtk_fixed_put(GTK_FIXED(fixed), scrolled_window, 10, 0);

    gtk_widget_set_size_request(scrolled_window, 430, 480);


    // 채팅창 텍스트 박스 및 레이블
    GtkWidget *chat_label = gtk_label_new("chat:");
    gtk_fixed_put(GTK_FIXED(fixed), chat_label, 10, 500);

    chat_entry = gtk_entry_new();
    gtk_fixed_put(GTK_FIXED(fixed), chat_entry, 55, 500);
    gtk_entry_set_width_chars(GTK_ENTRY(chat_entry), 8);
    gtk_widget_set_size_request(chat_entry, 300, 30);

    // '보내기' 버튼
    send_button = gtk_button_new_with_label("↵");
    gtk_fixed_put(GTK_FIXED(fixed), send_button, 370, 500);
    gtk_widget_set_size_request(send_button, 70, -1);

    // 클릭 이벤트에 대한 콜백 함수 연결
    g_signal_connect(send_button, "clicked", G_CALLBACK(send_button_clicked), text_view);

    // 수신 스레드 생성
    pthread_create(&recv_thread, NULL, receive_message, (void *)&sock);

    // 화면 표시 및 메인 루프 시작
    gtk_widget_show_all(window);
    // 닉네임
    show_nickname_dialog(window);
    send(sock, nickname, strlen(nickname), 0);  // 닉네임 서버로 전송
    gtk_main();
    return 0;
}

/* 서버로부터 메시지를 수신하는 스레드 함수 */
void *receive_message(void *socket) {
    int sock = *((int *)socket);    // 클라이언트 소켓
    char message[BUFFER_SIZE];      // 메시지 저장 버퍼
    int length;

    // 서버로부터 메시지를 계속 수신
    while ((length = recv(sock, message, BUFFER_SIZE, 0)) > 0) {
        message[length] = '\0';
        append_text_to_textview(GTK_TEXT_VIEW(text_view), message);
    }

    return NULL;
}

/* 채팅 삽입 함수 */
void append_text_to_textview(GtkTextView *textview, const gchar *text) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(textview);
    GtkTextIter iter;

    // 텍스트 버퍼의 끝 위치 얻기
    gtk_text_buffer_get_end_iter(buffer, &iter);

    // 텍스트 끝에 줄바꿈 추가
    gchar *text_with_newline = g_strdup_printf("%s\n", text);

    // 텍스트 삽입
    gtk_text_buffer_insert(buffer, &iter, text_with_newline, -1);

    // 메모리 해제
    g_free(text_with_newline);
}

void show_nickname_dialog(GtkWidget *parent) {
    GtkWidget *dialog, *content_area, *entry, *label;
    const gchar *name;

    // 다이얼로그 생성
    dialog = gtk_dialog_new_with_buttons("닉네임 입력",
                                         GTK_WINDOW(parent),
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         "확인",
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    // 라벨 추가
    label = gtk_label_new("닉네임을 입력하세요:");
    gtk_container_add(GTK_CONTAINER(content_area), label);

    // 엔트리 추가
    entry = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(content_area), entry);

    // 다이얼로그 표시
    gtk_widget_show_all(dialog);

    // 다이얼로그 실행
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_ACCEPT) {
        name = gtk_entry_get_text(GTK_ENTRY(entry));
        // g_print("입력된 닉네임: %s\n", name);
        strcpy(nickname, name);
    }

    // 다이얼로그 파괴
    gtk_widget_destroy(dialog);
}

// '보내기' 버튼 클릭 시 호출되는 콜백 함수
void send_button_clicked(GtkWidget *widget, gpointer data) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(data)); // data는 GtkTextView
    GtkWidget *entry_widget = GTK_WIDGET(g_object_get_data(G_OBJECT(widget), "ENTRY")); // chat_entry

    char message[BUFFER_SIZE]; // 메시지 입력 버퍼
    const gchar *message_text = gtk_entry_get_text(GTK_ENTRY(chat_entry)); // chat_entry에서 텍스트 가져오기

    strcpy(message, message_text);
    send(sock, message, strlen(message), 0); 
    gtk_entry_set_text(GTK_ENTRY(chat_entry), "");
}