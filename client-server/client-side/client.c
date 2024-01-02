#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
// #include "../../gameplay/chessboard.h"
#include "../util.h"
#include "../../room/room.h"
#include "client_user.h"
#include "client_home.h"
#include "client_game.h"
#include "../message.h"
#include "../../user/user.h"
#include "../../game/tetris.h"
#include "client.h"

//-------------Globals-----------------

UserNode *current_user = NULL;
int state = NOT_LOGGED_IN;
int room_updating = 0;
int game_state = 0;
int client_recv;
int client_send;
int roll_control = 1;
int af_roll = 1;
int in_room = 1;
int level = 1;
char *name;
int sock = 0, valread;
int startlevel;
int randomNum = 1010011010012;
int next;
Room *my_room = NULL;

//----------User Interfaces------------

void home(int sock);
void roomLobby(int sock);
// void game(int sock);

//------------ Handlers --------------

void *recv_handler(void *recv_sock);
void *send_handler(void *send_sock);

//------------------------------------

int main(int argc, const char *argv[])
{
    if (argc != 2)
    {
        printf("\nKhong ro dinh dang. Yeu cau nhap: ./client <address>\n");
        exit(1);
    }
    // create sockets
    int client_send_sock;
    client_send_sock = socket(AF_INET, SOCK_STREAM, 0);

    int client_recv_sock;
    client_recv_sock = socket(AF_INET, SOCK_STREAM, 0);

    // specify an address for the socket
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    server_address.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(client_send_sock, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        puts("Unable to connect send-stream to server. Exit");
        exit(-1);
    }

    if (connect(client_recv_sock, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        puts("Unable to connect recv-stream to server. Exit");
        exit(-1);
    }

    client_recv = client_recv_sock;
    client_send = client_send_sock;

    // Threading
    pthread_t threads[2];

    if (pthread_create(&threads[0], NULL, send_handler, &client_send_sock) < 0)
    {
        puts("Unable to open send thread. Exit.");
        exit(-1);
    }

    if (pthread_create(&threads[1], NULL, recv_handler, &client_recv_sock) < 0)
    {
        puts("Unable to open recv thread. Exit");
        exit(-1);
    }

    // join threads
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    // close sockets
    close(client_send_sock);
    close(client_recv_sock);

    return 0;
}

//------------------------------------------------------------

void home(int sock)
{
    int choice;
    do
    {
        // system("clear");
        printf("\n-----------Sanh cho-----------");
        printf("\nXin chao %s", current_user->username);
        printf("\n1. Tao phong");
        printf("\n2. Tham gia phong");
        printf("\n3. Thoat");
        printf("\nLua chon cua ban: ");
        scanf("%d%*c", &choice);
        switch (choice)
        {
        case 1:

            printf("Nhap level cua phong: ");
            scanf("%d%*c", &level);
            requestCreateRoom(sock, level);
            roomLobby(sock);
            while (in_room)
            {
            }
            break;
        case 2:
            if (requestJoinRoom(sock))
            {
                roomLobby(sock);
                while (in_room)
                {
                }
            }
            break;
        case 3:
            requestLogout(sock);
            break;
        default:
            printf("\nLa sao? Nhap lai coi\n");
            break;
        }
    } while (choice != 3);
}

void roomLobby(int sock)
{
    int choice;
    while (state == IN_ROOM || state == WAITING_RESPONSE)
    {
        if (state == IN_GAME)
            break;
        if (state == IN_ROOM && room_updating == 0)
        {
            if (state != IN_GAME)
            {
                scanf("%d%*c", &choice);
            }
            if (in_room == 1)
            {
                switch (choice)
                {
                case 1:
                    startGame(sock);
                    in_room = 1;
                    while (af_roll)
                    {
                    }
                    break;
                case 2:
                    printf("exit\n");
                    exitRoom(sock);
                    in_room = 0;
                    break;
                default:
                    printf("\nKhong ro cau lenh.\n");
                    break;
                }
                if (choice == 2 || state == IN_GAME)
                    break;
            }
        }
    }
    while (state == IN_GAME)
    {
        // play game and send data

        // int choice2;
        // scanf("%d", &choice2);
        // char buff[BUFFSIZE];
        // int dice;
        // switch(choice2){
        //     case 1:
        //         dice = rollDice();
        //         printf("\nBan tung duoc %d\n", dice);
        //         sprintf(buff, "DICE-%d", dice);
        //         send(current_user->send_sock, buff, SEND_RECV_LEN, 0);
        //         while(roll_control){}
        //         break;
        //     default: printf("\n ban nhap sai\n"); break;
        // }
    }
}

//------------------------------------------------------------

void *send_handler(void *send_sock)
{
    int send_socket = *(int *)send_sock;
    int choice;
    while (state == WAITING_RESPONSE || state == NOT_LOGGED_IN)
    {
        if (state == NOT_LOGGED_IN)
        {
            printf("\n-------------GAME TETRIS-------------");
            printf("\n1. Dang nhap");
            printf("\n2. Dang ki");
            printf("\n3. Thoat");
            printf("\nLua chon cua ban: ");
            scanf("%d", &choice);
            switch (choice)
            {
            case 1:
                if (requestLogin(send_socket))
                {
                    home(send_socket);
                }
                break;
            case 2:
                if (requestSignup(send_socket))
                {
                    home(send_socket);
                }
                break;
            case 3:
                send(send_socket, "exit", SEND_RECV_LEN, 0);
                printf("\nHen gap lai!!\n");
                break;
            default:
                printf("\nKhong hieu? Chon lai di.\n");
                break;
            }
            if (choice == 3)
                break;
        }
    }

    return 0;
}

void *recv_handler(void *recv_sock)
{
    int recv_socket = *(int *)recv_sock;
    int recv_bytes;
    char buff[BUFFSIZE];
    char *msg[MSG_NUM];
    while ((recv_bytes = recv(recv_socket, buff, SEND_RECV_LEN, 0) > 0))
    {
        meltMsg(buff, msg);
        if (strcmp(msg[0], "LOGIN") == 0)
        {
            if (strcmp(msg[1], "SUCCESS") == 0)
            {
                puts("\nDang nhap thanh cong");
                current_user = createUserNode(msg[2], msg[3]);
                current_user->recv_sock = client_recv;
                current_user->send_sock = client_send;
                state = LOGGED_IN;
                continue;
            }
            if (strcmp(msg[1], "FAILED") == 0)
            {
                if (strcmp(msg[2], "NONEXIST") == 0 || strcmp(msg[2], "WRONGPASS") == 0)
                {
                    puts("\nTen nguoi dung hoac mat khau khong chinh xac.");
                }
                else
                {
                    puts("\n Nguoi dung da hoat dong tren he thong.");
                }
                // puts("\nlogin failed.");
                state = NOT_LOGGED_IN;
                continue;
            }
        }
        if (strcmp(msg[0], "LOGOUT") == 0)
        {
            if (strcmp(msg[1], "SUCCESS") == 0)
            {
                state = NOT_LOGGED_IN;
                UserNode *node = current_user;
                free(node);
                current_user = NULL;
                continue;
            }
            else
            {
                state = LOGGED_IN;
                continue;
            }
        }
        if (strcmp(msg[0], "SIGNUP") == 0)
        {
            if (strcmp(msg[1], "SUCCESS") == 0)
            {
                current_user = createUserNode(msg[2], msg[3]);
                state = LOGGED_IN;
                continue;
            }
            else
            {
                state = NOT_LOGGED_IN;
                continue;
            }
        }
        if (strcmp(msg[0], "NEWROOM") == 0)
        {
            if (strcmp(msg[1], "SUCCESS") == 0)
            {
                room_updating = 1;
                // system("clear");
                my_room = createRoom(atoi(msg[2]), current_user->username, atoi(msg[3]));
                printRoom(my_room, current_user->username);
                state = IN_ROOM;
                room_updating = 0;
                continue;
            }
        }
        if (strcmp(msg[0], "UPDATEROOM") == 0)
        {
            if (strcmp(msg[1], "JOIN") == 0)
            {
                // system("clear");
                strcpy(my_room->players[my_room->inroom_no], msg[2]);
                my_room->inroom_no += 1;
                room_updating = 1;
                printf("%s joined", msg[2]);
                printRoom(my_room, current_user->username);
                room_updating = 0;
                continue;
            }
            if (strcmp(msg[1], "ALLEXIT") == 0)
            {
                // system("clear");
                printf("\nChu phong %s roi phong. Phong choi giai tan !\nNhan '1' de quay lai trang chu: \n", msg[2]);
                Room *room = my_room;
                freeRoom(room);
                my_room = NULL;
                state = LOGGED_IN;
                in_room = 0;
                continue;
            }
            if (strcmp(msg[1], "EXIT") == 0)
            {
                int i = 0;
                while (strcmp(msg[2], my_room->players[i]) != 0)
                    i++;
                while (i < my_room->inroom_no - 1)
                {
                    strcpy(my_room->players[i], my_room->players[i + 1]);
                    i++;
                }
                my_room->inroom_no -= 1;
                room_updating = 1;
                // system("clear");
                printf("\n%s left\n", msg[2]);
                printRoom(my_room, current_user->username);
                room_updating = 0;
                continue;
            }
        }
        if (strcmp(msg[0], "JOINROOM") == 0)
        { // message
            if (strcmp(msg[1], "SUCCESS") == 0)
            { // message
                my_room = createJoinRoom(msg);
                room_updating = 1;
                // system("clear");
                // for (int k = 0;k<sizeof(msg) / sizeof(msg[0]) ;k++){
                //     printf("%s ", msg[k]);
                // }
                printf("\n");
                printf("\n>> Tham gia phong thanh cong\n");
                printRoom(my_room, current_user->username);
                state = IN_ROOM;
                room_updating = 0;
                continue;
            }
            else if (strcmp(msg[1], "FULL") == 0)
            { // message
                printf("\n>> So nguoi choi trong phong da dat toi da ... Thong bao tu dong dong sau 3s\n");
                sleep(3);
                state = LOGGED_IN;
                continue;
            }
            else
            {
                printf("\n>> Tham gia phong khong thanh cong ... Thong bao tu dong dong sau 3s\n");
                sleep(3);
                state = LOGGED_IN;
                continue;
            }
        }
        if (strcmp(msg[0], "ONE") == 0)
        {
            printf("\nPhong khong du nguoi choi\n");
            continue;
        }
        if (strcmp(msg[0], "START") == 0)
        {
            int xacnhan;
            printf("Nhap 1 de xac nhan bat dau: ");
            scanf("%d", &xacnhan);
            if (xacnhan == 1)
            {
                sock = current_user->send_sock;
                name = current_user->username;
                srand(time(NULL));
                initscr();
                start_color();
                cbreak();
                init_pair(1, COLOR_GREEN, COLOR_BLACK);
                init_pair(2, COLOR_RED, COLOR_BLACK);
                attron(COLOR_PAIR(1));
                startlevel = my_room->room_level;
                noecho();
                curs_set(0);
                next = randomNum % 7;
                while (!game())
                    ;
                free(name);
                endwin();
                printf("Da bat dau");
                state = IN_GAME;
            }
            continue;
        }
        // if(strcmp(msg[0], "ROLL") == 0){
        //     game_state = 1;
        //     state = IN_GAME;
        //     printf("\nDen luot ban !!\n");
        //     printf("Moi nhap 1\n");
        //     af_roll = 0;
        //     roll_control = 1;
        //     continue;
        // }

        // if(strcmp(msg[0], WIN) == 0){
        //     printf("Nguoi choi %s da chien thang !!",my_room->players[atoi(msg[1])]);
        //     printf("\n");
        //     continue;
        // }
        // if(strcmp(msg[0], ENDGAME) == 0){
        //     printf("Tro choi ket thuc !!");
        //     printf("\n");
        //     roll_control = 0;
        //     af_roll = 0;
        //     state = IN_ROOM;
        //     continue;
        // }
        if (strcmp(msg[0], "ROOMS") == 0)
        {
            // system("clear");
            printf("\n%-62s", "=====================Danh sach cac phong=====================");
            printf("\n%-5s|%-20s|%s", "ID", "Chu phong", "So nguoi choi");
            int room_no = atoi(msg[1]);
            for (int i = 0; i < room_no; i++)
            {
                printf("\n%-5s|%-20s|%s/4", msg[2 + 3 * i], msg[3 + 3 * i], msg[4 + 3 * i]);
            }
            printf("\n=============================================================");
            state = LOGGED_IN;
            continue;
        }
    }

    if (recv_bytes < 0)
    {
        puts("\nError occurs in connection");
    }
    else
    {
        puts("\nConnection closed");
    }

    return 0;
}