#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define FILE_SIZE 256    // 메시지 버퍼 크기 정의
#define NICKNAME_LEN 32     // 닉네임 최대 길이 정의

int main() {
    int sock;                       // 클라이언트 소켓
    struct sockaddr_in server_addr; // 서버 주소 구조체
    char buffer[FILE_SIZE];      // 메시지 입력 버퍼
    int port = 50001;
    char ip[100] = "127.0.0.1";


    // 소켓 생성
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        return -1;
    }

    // 서버 주소 설정
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);   // 서버의 IP 주소
    server_addr.sin_port = htons(port);                     // 서버의 포트 번호

    // 서버에 연결
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        return -1;
    } else{
        printf("접속되었습니다.\n");
    }


    FILE* file;
    size_t fsize;
    int success; // 파일전송 성공 여부
    int isnull = 0;
    char filename[FILE_SIZE];
    int nbyte;
    // 사용자가 메시지 입력 및 서버로 전송
    while (1) {
        printf("명령어를 입력해주세요(/upload, /download):");
        scanf("%s",buffer);
        send(sock, buffer, strlen(buffer), 0);          // 메시지 서버로 전송

        if(strcmp(buffer, "/upload") == 0){
            
            printf("업로드할 파일 이름: ");
            scanf("%s",filename);

            file = fopen(filename, "rb");  //읽기 전용, 이진 모드로 파일 열기
            int isnull = 0;
            
            if(file == NULL){
                perror("file");
                isnull = 1;
            }

            send(sock, &isnull, sizeof(isnull), 0);  //파일 존재 여부 전송
            send(sock, filename, FILE_SIZE, 0);  //파일 이름 전송
            
            /*파일 크기 계산*/
            fseek(file, 0, SEEK_END);  //파일 포인터를 끝으로 이동
            fsize = ftell(file);  //파일 크기 계산
            fseek(file, 0, SEEK_SET);  //파일 포인터를 다시 처음으로 이동
            
            size_t size = htonl(fsize);
            send(sock, &size, sizeof(fsize), 0);  //파일 크기 전송
            
            int nsize =0;
            /*파일 전송*/
            while(nsize != fsize){  //256씩 전송
                int fpsize = fread(buffer, 1, FILE_SIZE, file);
                nsize += fpsize;
                send(sock, buffer, fpsize, 0);
            }
            fclose(file);
            recv(sock, &success, sizeof(int), 0);  //업로드 성공 여부 수신
            
            if (success) printf("업로드가 완료되었습니다.\n");
            else printf("업로드를 실패했습니다.\n");	

        } else if(strcmp(buffer, "/download") == 0){
            printf("다운로드할 파일 이름을 입력하세요: ");
            scanf("%s",filename);
            send(sock, filename, FILE_SIZE, 0); // 파일명 전송

            recv(sock, &isnull, sizeof(isnull), 0); // 파일이 있는지 없는지 확인
            if(isnull ==1){
                printf("파일이 존재하지 않습니다.\n");
                continue;
            }

            file = fopen(filename, "wb");  //쓰기 전용, 이진모드로 파일 열기
            recv(sock, &fsize, sizeof(fsize), 0);  //파일 크기 수신

            nbyte = FILE_SIZE;
            while(nbyte >= FILE_SIZE){
                nbyte = recv(sock, buffer, FILE_SIZE, 0);  //256씩 수신
                success = fwrite(buffer, sizeof(char), nbyte, file);  //파일 쓰기
                if(nbyte < FILE_SIZE) success =1;
            }
            send(sock, &success, sizeof(int), 0);  //성공 여부 전송
            fclose(file);		

            if (success){
                printf("다운로드가 완료되었습니다.\n");
            } 
            else{ 
                printf("다운로드에 실패했습니다.\n");
            }
        }
    }
    close(sock);    // 소켓 닫기
    return 0;
}
