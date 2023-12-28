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
  if (listen(serverSocket, 3) < 0)
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

    if ((childpid = fork()) == 0)
    {
      close(serverSocket);
      // Chat loop after authentication or sign-up
      while (1)
      {

        // Check for client exit command
        int valread;
        if ((valread = (newSocket, buffer, BUFFER_SIZE, 0)) < 0)
        {
          printf("recv failed");
          return -1;
        }
        printf("Received: %s\n", buffer);
        memset(buffer, 0, BUFFER_SIZE);
      }
      close(newSocket);
      exit(0);
    }
  }

  // Close server socket
  close(serverSocket);

  return 0;
}