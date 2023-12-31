#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_USERS 10

int main()
{
  int serverSocket, newSocket;
  struct sockaddr_in serverAddress, clientAddress;
  int opt = 1;
  int addressLength = sizeof(serverAddress);
  char buffer[BUFFER_SIZE] = {0};
  char message[BUFFER_SIZE] = {0};
  pid_t childpid;
  int clientSockets[10] = {0}; // Mảng lưu trữ các socket của client
  int activeClients = 0;       // Số lượng client hiện tại

  //  Create server socket
  if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
  {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }
  printf("Socket successfully created.\n");

  // Set socket options
  if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
  {
    perror("setsockopt failed");
    exit(EXIT_FAILURE);
  }

  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = INADDR_ANY;
  serverAddress.sin_port = htons(PORT);

  // Bind
  if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
  {
    perror("Bind failed");
    exit(EXIT_FAILURE);
  }
  printf("Socket successfully binded.\n");

  // Listen for connections
  if (listen(serverSocket, 10) < 0)
  {
    perror("Listen failed");
    exit(EXIT_FAILURE);
  }
  printf("Server listening...\n");

  // Start chat
  while (1)
  {
    // Accept incoming connection
    newSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, (socklen_t *)&addressLength);
    if (newSocket < 0)
    {
      perror("Accept failed");
      exit(EXIT_FAILURE);
    }
    printf("Server accepted the client...\n");

    if (activeClients >= 10)
    {
      // Đã đạt đến số lượng kết nối tối đa, từ chối kết nối mới
      printf("Connection limit reached. Rejecting new connection.\n");
      close(newSocket);
      continue;
    }

    // Thêm socket của client mới vào mảng
    for (int i = 0; i < 10; ++i)
    {
      if (clientSockets[i] == 0)
      {
        clientSockets[i] = newSocket;
        activeClients++;
        break;
      }
    }

    if ((childpid = fork()) == 0)
    {
      close(serverSocket); // Đóng socket của server trong quá trình con

      int clientIndex = -1;
      for (int i = 0; i < 10; ++i)
      {
        if (clientSockets[i] == newSocket)
        {
          clientIndex = i;
          break;
        }
      }

      // Chat loop
      while (1)
      {
        int valread;
        if ((valread = recv(clientSockets[clientIndex], buffer, BUFFER_SIZE, 0)) < 0)
        {
          printf("recv failed");
          return -1;
        }
        else if (valread == 0)
        {
          // Client closed connection
          printf("Client closed connection\n");
          break;
        }
        printf("Received: %s\n", buffer);
        int randomNum = rand() * 10000000;
        if (strcmp(buffer, "SEND_RANDOM"))
        {
          char resNum[20];
          sprintf(resNum, "%d", randomNum);
          if ((send(clientSockets[clientIndex], resNum, 20, 0)) < 0)
          {
            printf("send failed");
            return -1;
          }
          printf("Send number %s", resNum);
        }
        memset(buffer, 0, BUFFER_SIZE);
      }

      // Đóng kết nối của client và loại bỏ socket khỏi mảng
      close(clientSockets[clientIndex]);
      clientSockets[clientIndex] = 0;
      activeClients--;
      exit(0);
    }

    close(newSocket); // Đóng socket mới trong quá trình cha
  }

  // Close server socket
  close(serverSocket);

  return 0;
}