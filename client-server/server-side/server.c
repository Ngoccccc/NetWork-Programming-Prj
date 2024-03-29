#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "../util.h"
#include "../../user/user.h"
#include "../../room/room.h"
#include "server_room.h"
#include "server_user.h"
#include "../message.h"
#include "server.h"
#include "time.h"

//------------------Globals----------------------

int current_no_room; // current number of room on server
int total_user;
int server_socket;
UserNode *users;
Room *rooms[MAX_ROOM_ALLOWED];
//---------------------Base functions declarations---------------------

// Remember to use -pthread when compiling this server's source code
void *connection_handler(void *);
// void* gameloop_handler(void*);
void resolve(char *client_message, UserNode *current_user, int client_send_sock, int client_recv_sock, int client_game_sock);
void initGlobals();

//-------------------------- Main --------------------------

int main(int argc, const char *args[])
{

	// init globals
	initGlobals();

	// ------------- setup server socket -------------
	server_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (server_socket == -1)
	{
		perror("Socket initialization failed");
		exit(EXIT_FAILURE);
	}
	else
		printf("Server socket created successfully\n");

	int option = 1;
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(9999);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
	{
		printf("socket bind failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully binded..\n");

	if (listen(server_socket, 3) != 0)
	{
		printf("Listen failed...\n");
		exit(0);
	}
	else
		printf("Server listening..\n");

	// --------- Handling connection from clients -----------

	struct sockaddr_in client_addr;
	int client_send_sock;
	int client_recv_sock;
	int client_game_sock;
	unsigned int sin_size = sizeof((struct sockaddr *)&client_addr);

	int no_threads = 0; // number of threads accepted
	pthread_t threads[MAX_CLIENT_ALLOWED];

	while (no_threads < MAX_CLIENT_ALLOWED)
	{
		printf("Listening...\n");
		client_send_sock = accept(server_socket, (struct sockaddr *)&client_addr, &sin_size);
		if (client_send_sock < 0)
		{
			printf("Server failed to accept client's send-stream\n");
			exit(0);
		}
		else
			puts("> Client's send-sock accepted");

		client_recv_sock = accept(server_socket, (struct sockaddr *)&client_addr, &sin_size);
		if (client_recv_sock < 0)
		{
			printf("Server failed to accept client's recv-stream\n");
			exit(0);
		}
		else
			puts("> Client's recv-sock accepted");

		client_game_sock = accept(server_socket, (struct sockaddr *)&client_addr, &sin_size);
		if (client_game_sock < 0)
		{
			printf("Server failed to accept client's game-stream\n");
			exit(0);
		}
		else
			puts("> Client's game-sock accepted");

		puts("Connection accepted");

		ThrdHandlerArgs args;
		args.client_recv_sock = client_recv_sock;
		args.client_send_sock = client_send_sock;
		args.client_game_sock = client_game_sock;
		if (pthread_create(&threads[no_threads], NULL, connection_handler, &args) < 0)
		{
			perror("Could not create thread");
			return 1;
		}

		puts("Handler assigned\n");
		no_threads++;
	}

	int k = 0;
	for (k = 0; k < MAX_CLIENT_ALLOWED; k++)
	{
		pthread_join(threads[k], NULL);
	}

	close(server_socket);

	return 0;
}

//---------------------------Base functions definitions---------------------------

void initGlobals()
{
	printf("\n=====================================");
	printf("\nSetting up globals.....");
	current_no_room = 0;
	for (int i = 0; i < MAX_ROOM_ALLOWED; i++)
	{
		rooms[i] = NULL;
	}
	FILE *fp = fopen(ACCOUNTS_PATH, "r");
	if (fp == NULL)
	{
		printf("\n[ERROR] Unable to open accounts db");
		exit(1);
	}
	char username[BUFFSIZE];
	char passwd[BUFFSIZE];
	fscanf(fp, "%d\n", &total_user);
	for (int i = 1; i <= total_user; i++)
	{
		fscanf(fp, "%s %s\n", username, passwd);
		users = addUser(users, username, passwd);
	}
	fclose(fp);
	printf("\nDone setup globals");
	printf("\n=====================================\n");
}

void *connection_handler(void *client_sockets)
{
	// int socket = *(int*) client_socket;
	int client_send_sock = (*(ThrdHandlerArgs *)client_sockets).client_send_sock;
	int client_recv_sock = (*(ThrdHandlerArgs *)client_sockets).client_recv_sock;
	int client_game_sock = (*(ThrdHandlerArgs *)client_sockets).client_game_sock;

	char *msg[MSG_NUM];

	UserNode *current_user = NULL;

	int read_len;

	char client_message[BUFFSIZE];
	while ((read_len = recv(client_send_sock, client_message, SEND_RECV_LEN, 0)) > 0)
	{
		client_message[read_len] = '\0';
		printf("\n> Recv: %s\n", client_message);
		if (strcmp(client_message, "exit") == 0)
		{
			break;
		}

		meltMsg(client_message, msg);
		if (strcmp(msg[0], "LOGIN") == 0)
		{
			current_user = login(msg, client_send_sock, client_recv_sock, client_game_sock);
			continue;
		}
		if (strcmp(msg[0], "SIGNUP") == 0)
		{
			signup(msg, &current_user, client_send_sock, client_recv_sock, client_game_sock);
			continue;
		}
		if (strcmp(msg[0], "LOGOUT") == 0)
		{
			logout(msg, &current_user);
			continue;
		}
		if (strcmp(msg[0], "NEWROOM") == 0)
		{
			userCreateRoom(msg, &current_user);
			continue;
		}
		if (strcmp(msg[0], "EXITROOM") == 0)
		{
			userExitRoom(msg, &current_user);
			continue;
		}
		if (strcmp(msg[0], "JOINROOM") == 0)
		{
			userJoinRoom(msg, &current_user);
			continue;
		}
		if (strcmp(msg[0], "LEADERBOARD") == 0)
		{
			infoLeaderboard(&current_user);
			continue;
		}
		if (strcmp(msg[0], "CHANGEPASSWORD") == 0)
		{
			changePassword(msg, &current_user);
			continue;
		}

		if (strcmp(msg[0], "STARTC") == 0)
		{
			printf("> Recv: STARTC");
			time_t currentTime = time(NULL);
			printf("Thoi gian hien tai: %ld giay tu Epoch\n", currentTime);
			Room *room = rooms[current_user->room_id];
			room->point[0] = 0;
			room->point[1] = 0;
			if (room->inroom_no < 2)
			{
				send(current_user->recv_sock, "ONE", SEND_RECV_LEN, 0);
			}
			else
			{
				char buff[128];
				snprintf(buff, sizeof(buff), "START-%ld", currentTime);
				printf("Test random %s\n", buff);
				for (int i = 0; i < room->inroom_no; i++)
				{
					UserNode *user = searchUser(users, room->players[i]);
					send(user->recv_sock, buff, SEND_RECV_LEN, 0);
				}
				room->status = PLAYING;
			}
			continue;
		}
		if (strcmp(msg[0], "UPGRADE") == 0)
		{
			UserNode *user = searchUser(users, msg[1]);
			// printf("%s\n", user->username);
			Room *room = rooms[user->room_id];
			// printf("%s\n", room->players[0]);
			for (int i = 0; i < room->inroom_no; i++)
			{
				printf("%s\n", room->players[i]);
				if (strcmp(user->username, room->players[i]) == 0)
				{
					int point = atoi(msg[2]);
					room->point[i] = point;
					printf("User: %s have point %d\n", room->players[i], room->point[i]);
				}
				else
				{
					UserNode *user = searchUser(users, room->players[i]);
					char buff[128];
					snprintf(buff, sizeof(buff), "COMPETITOR-%s-%s", msg[1], msg[2]);
					send(user->game_sock, buff, SEND_RECV_LEN, 0);
					printf("server game sock: %d\n", user->game_sock);
					// printf("send: %d send2: %d\n", client_send_sock, user->send_sock);
					printf("\n> Send: %s\n", buff);
				}
			}
			continue;
		}
		if (strcmp(msg[0], "PAUSE") == 0)
		{
			UserNode *user = searchUser(users, msg[1]);
			Room *room = rooms[user->room_id];
			for (int i = 0; i < room->inroom_no; i++)
			{
				if (strcmp(user->username, room->players[i]) == 0)
				{
					int point = atoi(msg[2]);
					room->point[i] = point;
					printf("User: %s paused\n", room->players[i]);
				}
				else
				{
					UserNode *user = searchUser(users, room->players[i]);
					char buff[128];
					snprintf(buff, sizeof(buff), "PAUSE-%s", msg[1]);
					send(user->game_sock, buff, SEND_RECV_LEN, 0);
					printf("\n> Send: %s\n", buff);
				}
			}
			continue;
		}

		if (strcmp(msg[0], "RESUME") == 0)
		{
			UserNode *user = searchUser(users, msg[1]);
			Room *room = rooms[user->room_id];
			for (int i = 0; i < room->inroom_no; i++)
			{
				if (strcmp(user->username, room->players[i]) == 0)
				{
					int point = atoi(msg[2]);
					room->point[i] = point;
					printf("User: %s resumed\n", room->players[i]);
				}
				else
				{
					UserNode *user = searchUser(users, room->players[i]);
					char buff[128];
					snprintf(buff, sizeof(buff), "RESUME-%s", msg[1]);
					send(user->game_sock, buff, SEND_RECV_LEN, 0);
					printf("\n> Send: %s\n", buff);
				}
			}
			continue;
		}

		if (strcmp(msg[0], "GAMEOVER") == 0)
		{
			printf("\nUser: %s is game over\n", msg[1]);
			UserNode *user = searchUser(users, msg[1]);
			// printf("%s\n", user->username);
			Room *room = rooms[user->room_id];
			for (int i = 0; i < room->inroom_no; i++)
			{
				if (strcmp(user->username, room->players[i]) == 0)
				{
					room->isEndGame[i] = 1;
					printf("\nUser: %s has point %d\n", room->players[i], room->point[i]);
				}
			}

			if (room->isEndGame[0] + room->isEndGame[1] == 2)
			{
				char win[128];

				if (room->point[0] > room->point[1])
				{
					strcpy(win, room->players[0]);
				}
				else if (room->point[0] < room->point[1])
				{
					strcpy(win, room->players[1]);
				}
				else
				{
					strcpy(win, "DRAW");
				}

				char buff[256];
				snprintf(buff, sizeof(buff), "RESULT-%s", win);
				for (int i = 0; i < room->inroom_no; i++)
				{

					UserNode *user = searchUser(users, room->players[i]);
					send(user->recv_sock, buff, SEND_RECV_LEN, 0);
					user->room_id = -1;
					printf("Da gui %s den user: %s\n", buff, user->username);
				}
				printf("Winner is %s\n", buff);
				for (int i = 0; i < room->inroom_no; i++)
				{
					room->point[i] = 0;
					room->isEndGame[i] = 0;
				}
			}
			else
			{
				printf("Send waiting message");
				char buff[128];
				strcpy(buff, "WAITINGRESULT");
				send(user->recv_sock, buff, SEND_RECV_LEN, 0);
				printf("Send: %s\n", buff);
			}
			continue;
		}

		if (strcmp(msg[0], "ROOMS") == 0)
		{
			char buff[BUFFSIZE];
			char buff1[BUFFSIZE];
			sprintf(buff, "ROOMS-%d", current_no_room);
			for (int i = 0; i < MAX_ROOM_ALLOWED; i++)
			{
				if (rooms[i] == NULL)
					continue;
				sprintf(buff1, "-%d-%s-%d-%d", i, rooms[i]->players[0], rooms[i]->inroom_no, rooms[i]->room_level);
				strcat(buff, buff1);
			}
			printf("\n> Send: %s", buff);
			send(current_user->recv_sock, buff, SEND_RECV_LEN, 0);
			continue;
		}
		else
		{
			send(client_recv_sock, "UNKNOWN", SEND_RECV_LEN, 0); // message
			continue;
			continue;
		}
	}

	close(client_send_sock);
	close(client_recv_sock);

	return 0;
}
