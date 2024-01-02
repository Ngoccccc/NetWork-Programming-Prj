#ifndef __ROOM_H__
#define __ROOM_H__

#define MAX_PLAYER_PER_ROOM 5
#define MAX_ROOM_ALLOWED 5
#include "../game/tetris.h"
// Room status
typedef enum
{
    WAITING,
    PLAYING
} RoomStatus;

// Room struct
typedef struct Room
{
    int room_id;
    char *players[4];
    int point[2];
    int isEndGame[2];
    int inroom_no;
    int room_level;
    RoomStatus status;
} Room;

// Room util

int addRoom(Room **rooms, char *owner, int level);

Room *createRoom(int room_id, char *owner, int level);

Room *createBlankRoom(int room_id);

int addUserToRoom(Room **rooms, int room_id, char *username);

int removeUserFromRoom(Room **rooms, int room_id, char *username);

void delRoom(Room **rooms, int room_id);

// void startGame(Room** rooms, int room_id);

void printRooms(Room **rooms);

void printRoom(Room *room, char *current_user_name);

void freeRoom(Room *node);

char *roomToString(Room **root, int room_id);

#endif