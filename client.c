#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main()
{
  int sock = 0, valread;
  struct sockaddr_in serv_addr;
  char buffer[1024] = {0};

  // Tạo socket
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n Socket creation error \n");
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  // Kết nối tới server
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("\nConnection Failed \n");
    return -1;
  }

  // Gửi dữ liệu tới server
  send(sock, hello, strlen(hello), 0);
  printf("Hello message sent\n");

  // Nhận dữ liệu từ server
  if ((valread = recv(sock, buffer, 1024, 0)) < 0)
  {
    printf("recv failed");
    return -1;
  }
  printf("Received: %s\n", buffer);

  return 0;
}
