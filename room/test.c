#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "room.h"

Room* createJoinRoom(int room_id, int inroom_no, char** inroom_players){
    Room* joinroom = (Room*) malloc(sizeof(Room));
    joinroom->inroom_no = inroom_no + 1;
    joinroom->room_id = room_id;
    joinroom->status = WAITING;
    int i = 0;
    for(; i < inroom_no; i++){
        joinroom->players[i] = (char*) malloc(100);
        strcpy(joinroom->players[i], inroom_players[i]);
    }
    joinroom->players[i] = (char*) malloc(100);
    strcpy(joinroom->players[i++], "current_user");
    for(; i < MAX_PLAYER_PER_ROOM; i++){
        joinroom->players[i] = (char*) malloc(100);
    }
    return joinroom;
}

void main(){
    Room* rooms[MAX_ROOM_ALLOWED];
    for(int i = 0; i < MAX_ROOM_ALLOWED; i++)
        rooms[i] = NULL;
    addRoom(rooms, 0, "truong");
    addUserToRoom(rooms, 0, "ngoc");
    addUserToRoom(rooms, 0, "quan");
    addRoom(rooms, 2 ,"tuan");
    addUserToRoom(rooms,2,"nghia");
    removeUserFromRoom(rooms,0,"ngoc");
    printRooms(rooms);
    
    char* result1 = roomToString(rooms, 0);
    char* result2 = roomToString(rooms, 2);
    printf("\n<%s>\n", result1);
    printf("\n<%s>\n", result2);
}