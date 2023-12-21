#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 7070
#define BUFFER_SIZE 1024

void printMenu()
{
    printf("Choose an option:\n");
    printf("1. Login\n");
    printf("2. Sign Up\n");
    printf("3. Exit\n");
}

int main(int argc, char *argv[])
{
    int clientSocket;
    struct sockaddr_in serverAddress;
    char buffer[BUFFER_SIZE] = {0};
    char message[BUFFER_SIZE] = {0};

    // Create client socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket successfully created.\n");

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);

    if (inet_pton(AF_INET, argv[1], &(serverAddress.sin_addr)) <= 0)
    {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
    printf("Connected to the server.\n");

    int authenticated = 0;

    while (!authenticated)
    {

        // Clear the buffer and message
        memset(buffer, 0, BUFFER_SIZE);
        memset(message, 0, BUFFER_SIZE);

        printMenu();
        // Get user choice
        printf("Enter your choice: ");
        fgets(message, BUFFER_SIZE, stdin);

        // Send user choice to the server
        send(clientSocket, message, strlen(message), 0);
        // Process user choice
        switch (atoi(message))
        {
        case 1: // Login
            // Get username and password from the user
            printf("Enter your username: ");
            fgets(message, BUFFER_SIZE, stdin);
            message[strcspn(message, "\n")] = 0;
            send(clientSocket, message, strlen(message), 0);

            printf("Enter your password: ");
            fgets(message, BUFFER_SIZE, stdin);
            message[strcspn(message, "\n")] = 0;
            send(clientSocket, message, strlen(message), 0);

            // Receive and print the server's response
            recv(clientSocket, buffer, BUFFER_SIZE, 0);
            printf("Server: %s", buffer);

            recv(clientSocket, buffer, BUFFER_SIZE, 0);
            authenticated = atoi(buffer);
            break;
        case 2: // Sign Up
            // Get username and password from the user
            printf("Enter your new username: ");
            fgets(message, BUFFER_SIZE, stdin);
            message[strcspn(message, "\n")] = 0;
            send(clientSocket, message, strlen(message), 0);

            printf("Enter your new password: ");
            fgets(message, BUFFER_SIZE, stdin);
            message[strcspn(message, "\n")] = 0;
            send(clientSocket, message, strlen(message), 0);

            // Receive and print the server's response
            recv(clientSocket, buffer, BUFFER_SIZE, 0);
            printf("Server: %s", buffer);

            authenticated = 1;
            break;

        case 3: // Exit
            close(clientSocket);
            exit(0);

        default:
            printf("Invalid choice from client. Please try again.\n");
            recv(clientSocket, buffer, BUFFER_SIZE, 0);
            break;
        }

        // Break out of the loop if the user successfully logs in or signs up
        if (authenticated)
        {
            break;
        }
    }

    while (1)
    {
        // Clear the buffer and message
        memset(buffer, 0, BUFFER_SIZE);
        memset(message, 0, BUFFER_SIZE);

        // Get client message
        printf("Client: ");
        fgets(message, BUFFER_SIZE, stdin);

        // Send client message to the server
        send(clientSocket, message, strlen(message), 0);

        // Check for client exit command
        if (strcmp(message, "logout\n") == 0)
        {
            break;
        }

        // Read message from server
        recv(clientSocket, buffer, BUFFER_SIZE, 0);
        printf("Server: %s\n", buffer);
    }

    // Close socket
    close(clientSocket);

    return 0;
}
