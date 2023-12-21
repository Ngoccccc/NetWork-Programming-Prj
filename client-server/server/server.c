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
};

struct User users[MAX_USERS];
int num_users = 0;

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

            // Authentication and Sign-up loop
            while (!authenticated)
            {
                // Read user choice from the client
                recv(newSocket, buffer, BUFFER_SIZE, 0);
                int choice = atoi(buffer);
                printf("choice %d\n", choice);

                switch (choice){
                case 1: // Login
                    // Validate user credentials
                    recv(newSocket, buffer, BUFFER_SIZE, 0);
                    char username[BUFFER_SIZE];
                    strcpy(username, buffer);

                    recv(newSocket, buffer, BUFFER_SIZE, 0);
                    char password[BUFFER_SIZE];
                    strcpy(password, buffer);
                    
                    for (int i = 0; i < num_users; i++){
                        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0){
                            user_index = i;
                            authenticated = 1;
                            printf("User %s authenticated.\n", users[user_index].username);
                            send(newSocket, "Login successful.\n", strlen("Login successful.\n"), 0);
                            break;
                        }
                    }

                    if (!authenticated){
                        printf("Authentication failed for user %s.\n", username);
                        send(newSocket, "Authentication failed. Try again.\n", BUFFER_SIZE, 0);
                        send(newSocket, "0", BUFFER_SIZE, 0);
                    }else{
                        send(newSocket, "1", BUFFER_SIZE, 0);
                    }
                    break;

                case 2: // Sign Up
                    // Read new username and password from the client
                    recv(newSocket, buffer, BUFFER_SIZE, 0);
                    char newUsername[BUFFER_SIZE];
                    strcpy(newUsername, buffer);

                    recv(newSocket, buffer, BUFFER_SIZE, 0);
                    char newPassword[BUFFER_SIZE];
                    strcpy(newPassword, buffer);

                    // Check if the username is available
                    int userExists = 0;
                    for (int i = 0; i < num_users; i++)
                    {
                        if (strcmp(users[i].username, newUsername) == 0)
                        {
                            userExists = 1;
                            break;
                        }
                    }

                    if (userExists)
                    {
                        printf("Username %s already exists. Try again.\n", newUsername);
                        send(newSocket, "Username already exists. Try again.\n", strlen("Username already exists. Try again.\n"), 0);
                    }
                    else
                    {
                        user_index = num_users;
                        // Add the new user to the list
                        strcpy(users[num_users].username, newUsername);
                        strcpy(users[num_users].password, newPassword);
                        num_users += 1;
                        authenticated = 1;
                        printf("User %s signed up and authenticated.\n", newUsername);
                        send(newSocket, "Sign-up successful.\n", strlen("Sign-up successful.\n"), 0);
                    }

                    printf("%d\n", num_users);
                    break;

                case 3: // Exit
                    close(newSocket);
                    exit(0);
                    break;

                default:
                    printf("Invalid choice. Please try again.\n");
                    send(newSocket, "Invalid choice. Please try again.\n", strlen("Invalid choice. Please try again.\n"), 0);
                    break;
                }
            }

            // Chat loop after authentication or sign-up
            while (1)
            {
                recv(newSocket, buffer, BUFFER_SIZE, 0);

                // Check for client exit command
                if (strcmp(buffer, "logout\n") == 0)
                {
                    printf("User %s logged out.\n", users[user_index].username);
                    break;
                }

                // Process client message (e.g., convert to uppercase)
                // ...

                // Send server message to client
                send(newSocket, buffer, strlen(buffer), 0);

                // Check for server exit command
                if (strcmp(buffer, "exit\n") == 0)
                {
                    break;
                }

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
