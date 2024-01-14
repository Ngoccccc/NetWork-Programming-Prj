#ifndef __SERVERUSER_H__
#define __SERVERUSER_H__

#include "../../user/user.h"

#define ACCOUNTS_PATH "client-server/accounts.txt"

//-------------Globals----------------
extern UserNode* users;
extern int total_user;

//------------Functions---------------

UserNode* login(char** msg, int client_send_sock, int client_recv_sock, int client_game_sock);

void logout(char** msg, UserNode** current_user);

void signup(char** msg, UserNode** current_user, int client_send_sock, int client_recv_sock, int client_game_sock);

void changePassword(char **msg, UserNode **current_user);

// void deleteUser(char **msg, UserNode **current_user);
#endif