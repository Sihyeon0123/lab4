#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>

#define FILE_SIZE 256    // 메시지 버퍼 크기 정의

int main() {
    int server_sock, client_sock;                   // 서버 및 클라이언트 소켓
    struct sockaddr_in server_addr, client_addr;    // 소켓 주소 구조체         
    socklen_t client_addr_size;                     // 클라이언트 주소 크기
    int port = 50001;

    // 서버 소켓 생성
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("socket");
        return -1;
    }

    // 서버 주소 설정
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    // 소켓에 주소 바인딩
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        return -1;
    }

    // 소켓 리스닝 시작
    if (listen(server_sock, 5) == -1) {
        perror("listen");
        return -1;
    }

    printf("서버가 시작되었습니다.\n");

    client_addr_size = sizeof(client_addr);
    client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_size);
    if (client_sock == -1) {
        perror("accept");
        return -1;
    }

    char buffer[FILE_SIZE];
    char filename[FILE_SIZE];
    int len;
    FILE* file;
    size_t fsize;
    // 클라이언트로부터 메시지 수신
    while ((len = recv(client_sock, buffer, FILE_SIZE, 0)) > 0) {
        buffer[len] = '\0';

        if(strcmp(buffer, "/upload") == 0){
            int isnull = 0, success = 0, nbyte;

			recv(client_sock, &isnull, sizeof(isnull), 0); // 명령어를 전달받은 후 의 처리
			if(isnull ==1){
				continue;
			}
			
			recv(client_sock, filename, FILE_SIZE, 0);  //파일 이름 수신
			
			file = fopen(filename, "wb");  //쓰기 전용, 이진모드로 파일 열기
			recv(client_sock, &fsize, sizeof(fsize), 0);  //파일 크기 수신

			nbyte = FILE_SIZE;
			while(nbyte >= FILE_SIZE){
				nbyte = recv(client_sock, buffer, FILE_SIZE, 0);  //256씩 수신
				success = fwrite(buffer, sizeof(char), nbyte, file);  //파일 쓰기
				if(nbyte < FILE_SIZE) success =1;
			}
			send(client_sock, &success, sizeof(int), 0);  //성공 여부 전송
			fclose(file);		
            if (success){ 
                printf("다운로드가 완료되었습니다.\n");
            }
			else{ 
                printf("다운로드에 실패했습니다.\n");	
            }
        } 
        else if(strcmp(buffer, "/download") == 0){ 
            int success; // 파일전송 성공 여부

            recv(client_sock, filename, FILE_SIZE, 0);  // 파일명 송신
            
            file = fopen(filename, "rb");  //읽기 전용, 이진 모드로 파일 열기

			int isnull =0;
			if(file ==NULL){
				isnull =1;
				send(client_sock, &isnull, sizeof(isnull), 0);
				continue;			
			}	
			send(client_sock, &isnull, sizeof(isnull), 0);  //파일 존재 여부 전송

			/*파일 크기 계산*/
			fseek(file, 0, SEEK_END);  //파일 포인터를 끝으로 이동
			fsize = ftell(file);  //파일 크기 계산
			fseek(file, 0, SEEK_SET);  //파일 포인터를 다시 처음으로 이동
			
			size_t size = htonl(fsize);
			send(client_sock, &size, sizeof(fsize), 0);  //파일 크기 전송
			
			int nsize =0;
			/*파일 전송*/
			while(nsize != fsize){  //256씩 전송
				int fpsize = fread(buffer, 1, FILE_SIZE, file);
				nsize += fpsize;
				send(client_sock, buffer, fpsize, 0);
			}
			fclose(file);
			recv(client_sock, &success, sizeof(int), 0);  //업로드 성공 여부 수신
			
			if (success){
                printf("업로드가 완료되었습니다.\n");
            } 
			else{ 
                printf("업로드를 실패했습니다.\n");	
            }
        }
    }

    return 0;
}
