#include <stdio.h>

#include <ctype.h>

#include <stdlib.h>

#include <string.h>

#include <sys/socket.h>

#include <arpa/inet.h>

#include <unistd.h>

#define PORT 7070

#define BUFFER_SIZE 1024

#define MAX_USERS 10

struct User
{
    char username[BUFFER_SIZE];
    char password[BUFFER_SIZE];
    int sockfd; // Store the corresponding socket descriptor
};

struct User users[MAX_USERS];
int num_users = 0;

int main()
{

    int serverSocket, newSocket, valRead;
    struct sockaddr_in serverAddress, clientAddress;
    int opt = 1;
    int addressLength = sizeof(serverAddress);
    char buffer[BUFFER_SIZE] = {0};
    char message[BUFFER_SIZE] = {0};
    pid_t childpid;
    // char upperMessage[BUFFER_SIZE] = {0};
    //  Create server socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket successfully created.\n");

    // Set socket options
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
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

            int authenticated = 0;
            int user_index = -1;
            while (!authenticated)
            {
                // Read username and password from client
                recv(newSocket, buffer, BUFFER_SIZE, 0);
                int choice = atoi(buffer);

                switch (choice) {
                    // Validate user credentials
                    case 1:  // Login
                        // Validate user credentials
                        recv(newSocket, buffer, BUFFER_SIZE, 0);
                        char username[BUFFER_SIZE], password[BUFFER_SIZE];
                        sscanf(buffer, "%s %s", username, password);
                        for (int i = 0; i < num_users; i++)
                        {
                            if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0)
                            {
                                user_index = i;
                                break;
                            }
                        }

                        if (user_index != -1)
                        {
                            authenticated = 1;
                            users[user_index].sockfd = newSocket;
                            printf("User %s authenticated.\n", users[user_index].username);
                        }
                        else
                        {
                            printf("Authentication failed for user %s.\n", username);
                            send(newSocket, "Authentication failed. Try again.\n", strlen("Authentication failed. Try again.\n"), 0);
                        }

                        break;

                    case 2:  // Sign Up
                        // Read new username and password from the client
                        recv(newSocket, buffer, BUFFER_SIZE, 0);
                        printf("zzz %s", buffer);
                        char newUsername[BUFFER_SIZE];
                        strcpy(newUsername, buffer);

                        recv(newSocket, buffer, BUFFER_SIZE, 0);
                        char newPassword[BUFFER_SIZE];
                        strcpy(newPassword, buffer);

                        // Check if the username is available
                        int userExists = 0;
                        for (int i = 0; i < num_users; i++) {
                            if (strcmp(users[i].username, newUsername) == 0) {
                                userExists = 1;
                                break;
                            }
                        }

                        if (userExists) {
                            send(newSocket, "Username already exists. Try again.\n", strlen("Username already exists. Try again.\n"), 0);
                        } else {
                            // Add the new user to the list
                            strcpy(users[num_users].username, newUsername);
                            strcpy(users[num_users].password, newPassword);
                            users[num_users].sockfd = newSocket;
                            num_users++;

                            authenticated = 1;

                            printf("User %s signed up and authenticated.\n", newUsername);
                            send(newSocket, "Sign-up successful.\n", strlen("Sign-up successful.\n"), 0);
                        }

                        break;

                    case 3:  // Exit
                        close(newSocket);
                        exit(0);

                    default:
                        printf("Invalid choice. Please try again.\n");
                        send(newSocket, "Invalid choice. Please try again.\n", strlen("Invalid choice. Please try again.\n"), 0);
                }
            }

            while (1)
            {
                // Read message from client
                recv(newSocket, buffer, BUFFER_SIZE, 0);

                // Check for client exit command
                if (strcmp(buffer, "logout") == 0)
                {
                    printf("User %s logged out.\n", users[user_index].username);
                    break;
                }

                // strcpy(message, buffer);
                for (int i = 0; i < BUFFER_SIZE; i++)
                {
                    message[i] = toupper(buffer[i]);
                }
                // Send server message to client
                send(newSocket, message, strlen(message), 0);

                // Check for server exit command
                if (strcmp(message, "exit\n") == 0)
                {
                    break;
                }
                memset(buffer, 0, BUFFER_SIZE);
                memset(message, 0, BUFFER_SIZE);
            }
        }
    }

    // Close sockets
    close(newSocket);
    close(serverSocket);

    return 0;
}
