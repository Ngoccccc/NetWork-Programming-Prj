#ifndef __CLIENTHOME_H__
#define __CLIENTHOME_H__

#include "../../room/room.h"

//----------Globals-----------
extern Room* my_room;

//-------Functions-----------

void requestCreateRoom(int sock, int level);

int requestJoinRoom();
void printLeaderboard(char** msg);
void requestLeaderboard(int sock);

void requestFindRoom();

void exitRoom(int sock);

Room* createJoinRoom(char** msg);

#endif