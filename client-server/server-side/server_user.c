#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include "../message.h"
#include "../../user/user.h"
#include "server_user.h"

extern UserNode *users;

UserNode *login(char **msg, int client_send_sock, int client_recv_sock, int client_game_sock)
{
	char buff[BUFFSIZE];
	UserNode *node = searchUser(users, msg[1]);
	if (node == NULL)
	{
		send(client_recv_sock, "LOGIN-FAILED-NONEXIST", SEND_RECV_LEN, 0); // message
		return NULL;
	}
	if (node->status != OFFLINE)
	{
		send(client_recv_sock, "LOGIN-FAILED-ACTIVE", SEND_RECV_LEN, 0); // message
		return NULL;
	}
	printf("3\n");
	if (strcmp(node->password, msg[2]) != 0)
	{
		send(client_recv_sock, "LOGIN-FAILED-WRONGPASS", SEND_RECV_LEN, 0);
		return NULL;
	}
	printf("4\n");
	printf("4\n");

	node->status = ONLINE;
	node->recv_sock = client_recv_sock;
	node->send_sock = client_send_sock;
	node->game_sock = client_game_sock;
	printf("\nUser logged in: %s", node->username);
	sprintf(buff, "LOGIN-SUCCESS-%s-%s", msg[1], msg[2]);
	send(client_recv_sock, buff, SEND_RECV_LEN, 0); // message
	return node;
}

void logout(char **msg, UserNode **current_user)
{
	if (updateUserStatus(users, msg[1], OFFLINE))
	{
		send((*current_user)->recv_sock, "LOGOUT-SUCCESS", SEND_RECV_LEN, 0); // message
		*current_user = NULL;
	}
}

void signup(char **msg, UserNode **current_user, int client_send_sock, int client_recv_sock, int client_game_sock)
{
	// add to users tree
	UserNode *node = searchUser(users, msg[1]);
	if (node != NULL)
	{
		send((*current_user)->recv_sock, "SIGNUP-EXISTS", SEND_RECV_LEN, 0);
		return;
	}
	users = addUser(users, msg[1], msg[2]);
	updateUserStatus(users, msg[1], ONLINE);

	// make to current_user
	*current_user = searchUser(users, msg[1]);
	(*current_user)->recv_sock = client_recv_sock;
	(*current_user)->send_sock = client_send_sock;
	(*current_user)->game_sock = client_game_sock;

	// write new account to users file
	FILE *fp = fopen(ACCOUNTS_PATH, "r+");
	if (fp == NULL)
	{
		printf("Can't open users records");
		send((*current_user)->recv_sock, "SIGNUP-FAIL", SEND_RECV_LEN, 0);
		return;
	}
	fprintf(fp, "%d", total_user + 1);
	fseek(fp, 0, SEEK_END);
	fprintf(fp, "%s %s\n", msg[1], msg[2]);
	fclose(fp);

	printf("\nUser signed up: %s\n", (*current_user)->username);
	// send sign up ACK
	char buff[BUFFSIZE];
	sprintf(buff, "SIGNUP-SUCCESS-%s-%s", (*current_user)->username, (*current_user)->password);
	send((*current_user)->recv_sock, buff, SEND_RECV_LEN, 0);
}

void changePassword(char **msg, UserNode **current_user) {
    char buff[BUFFSIZE];

    UserNode *node = searchUser(users, (*current_user)->username);

    if (strcmp(node->password, msg[1]) != 0) {
        send((*current_user)->recv_sock, "CHANGEPASSWORD-FAILED-WRONGPASS", SEND_RECV_LEN, 0);
        return;
    }

    strcpy(node->password, msg[2]);

    // Update password in the users file
    FILE *fp = fopen(ACCOUNTS_PATH, "r+");
    if (fp == NULL) {
        printf("Can't open users records");
        send((*current_user)->recv_sock, "CHANGEPASSWORD-FAIL", SEND_RECV_LEN, 0);
        return;
    }

	FILE *temp_fp = fopen("temp_accounts.txt", "w");
    if (temp_fp == NULL) {
        printf("Can't create temporary file");
        fclose(fp);
        send((*current_user)->recv_sock, "CHANGEPASSWORD-FAIL", SEND_RECV_LEN, 0);
        return;
    }

    char line[BUFFSIZE];
    int accountCount;

    // Read and skip the first line indicating the number of accounts
    if (fgets(line, sizeof(line), fp) != NULL) {
        sscanf(line, "%d", &accountCount);
        fprintf(temp_fp, "%d\n", accountCount);
    }

    // Process the rest of the file
    while (fgets(line, sizeof(line), fp)) {
        char username[BUFFSIZE];
        char password[BUFFSIZE];
        if (sscanf(line, "%s %s", username, password) == 2) {
            if (strcmp(username, node->username) == 0) {
                fprintf(temp_fp, "%s %s\n", node->username, node->password);
            } else {
                fprintf(temp_fp, "%s %s\n", username, password);
            }
        }
    }

    fclose(fp);
    fclose(temp_fp);

    if (remove(ACCOUNTS_PATH) != 0) {
        printf("Error removing file: %s\n", ACCOUNTS_PATH);
        send((*current_user)->recv_sock, "CHANGEPASSWORD-FAIL", SEND_RECV_LEN, 0);
        return;
    }

    if (rename("temp_accounts.txt", ACCOUNTS_PATH) != 0) {
        printf("Error renaming file\n");
        send((*current_user)->recv_sock, "CHANGEPASSWORD-FAIL", SEND_RECV_LEN, 0);
        return;
    }

    printf("\nPassword changed for user: %s\n", node->username);

    // Send password change ACK
    sprintf(buff, "CHANGEPASSWORD-SUCCESS-%s", node->username);
    send((*current_user)->recv_sock, buff, SEND_RECV_LEN, 0);
}

// void inOrderPrintToFile(UserNode *root, FILE *fp) {
//     if (root != NULL) {
//         inOrderPrintToFile(root->left, fp);
//         fprintf(fp, "%s %s\n", root->username, root->password);
//         inOrderPrintToFile(root->right, fp);
//     }
// }


// void deleteUser(char **msg, UserNode **current_user) {
//     char buff[BUFFSIZE];

//     UserNode *node = searchUser(users, msg[1]);

//     if (node == NULL) {
//         send((*current_user)->recv_sock, "DELETEUSER-FAILED-NONEXIST", SEND_RECV_LEN, 0);
//         return;
//     }

//     // Update users tree
//     users = deleteUserNode(users, node->username);

//     // Update users file
//     FILE *fp = fopen(ACCOUNTS_PATH, "w");
//     if (fp == NULL) {
//         printf("Can't open users records");
//         send((*current_user)->recv_sock, "DELETEUSER-FAIL", SEND_RECV_LEN, 0);
//         return;
//     }

//     // Re-write the user data excluding the deleted user
//     inOrderPrintToFile(users, fp);
//     fclose(fp);

//     printf("\nUser deleted: %s\n", node->username);

//     // Send delete user ACK
//     sprintf(buff, "DELETEUSER-SUCCESS-%s", node->username);
//     send((*current_user)->recv_sock, buff, SEND_RECV_LEN, 0);

//     // Clear the current_user
//     *current_user = NULL;
// }