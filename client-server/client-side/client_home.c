#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include "../message.h"
#include "client_user.h"
#include "client_home.h"
#include "client.h"
#include "../../room/room.h"
#include "../util.h"

Room* createJoinRoom(char** msg){
    int room_id = atoi(msg[2]);
    int inroom_no = atoi(msg[3]);
    int room_level = atoi(msg[4]);

    Room* joinroom = (Room*) malloc(BUFFSIZE);
    // joinroom->game = NULL;
    joinroom->inroom_no = inroom_no;
    joinroom->room_id = room_id;
    joinroom->room_level = room_level;
    joinroom->status = WAITING;
    int i = 0;
    for(; i < inroom_no; i++){
        joinroom->players[i] = (char*) malloc(30);
        strcpy(joinroom->players[i], msg[i+4]);
    }
    // for(; i < MAX_PLAYER_PER_ROOM; i++){
    //     joinroom->players[i] = (char*) malloc(10);
    // }
    // printf("ss %d", joinroom->inroom_no);
    return joinroom;
}

void requestCreateRoom(int sock, int level){
    char buff[BUFFSIZE];
    sprintf(buff, "NEWROOM-%s-%d", current_user->username, level);
    send(sock, buff, SEND_RECV_LEN, 0);
    state = WAITING_RESPONSE;
}

int requestJoinRoom(int sock){
    char buff[BUFFSIZE];
    while(1){
        while(state == WAITING_RESPONSE);
        send(sock, "ROOMS", SEND_RECV_LEN, 0);
        state = WAITING_RESPONSE;
        while(state == WAITING_RESPONSE);
        int join_id = -1;
        printf("\n> Moi nhap ma so phong tham gia (nhap '-1' de thoat): ");
        scanf("%d%*c", &join_id);
        if(join_id == -1) {
            return 0;
        } else {
            sprintf(buff, "JOINROOM-%s-%d\n", current_user->username, join_id);
            send(current_user->send_sock, buff, SEND_RECV_LEN, 0);
            state = WAITING_RESPONSE;
            while(state == WAITING_RESPONSE);
            if(state == IN_ROOM) break;
        }
    }
    return 1;
}

void exitRoom(int sock){
    // printf("1\n");
    char buff[BUFFSIZE];
    snprintf(buff, sizeof(buff), "EXITROOM-%s-%d", current_user->username, my_room->room_id); // message
    send(sock, buff, SEND_RECV_LEN, 0);
    Room* node = my_room;
    // printf("2\n");
    freeRoom(node);
    state = LOGGED_IN;
    my_room = NULL;
}