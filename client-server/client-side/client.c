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
#include <semaphore.h>
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
int client_game;
int roll_control = 1;
int af_roll = 1;
int in_room = 1;
int level = 1;


char *name;
char *opponent;
int send_sock = 0, valread;
int recv_sock = 0;
int game_sock = 0;
int startlevel;
long int randomNum = 0;
int next;
int done_leaderboard = 1;
Room *my_room = NULL;

//----------User Interfaces------------

void home(int sock);
void roomLobby(int sock);
// void game(int sock);

//------------ Handlers --------------

void *recv_handler(void *recv_sock);
void *send_handler(void *send_sock);
void *gameMessageReceiver(void *game_socket);
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

    int client_game_sock;
    client_game_sock = socket(AF_INET, SOCK_STREAM, 0);

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

    if (connect(client_game_sock, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        puts("Unable to connect game-stream to server. Exit");
        exit(-1);
    }

    client_recv = client_recv_sock;
    client_send = client_send_sock;
    client_game = client_game_sock;

    // Threading
    pthread_t threads[3];

    if (pthread_create(&threads[0], NULL, recv_handler, &client_recv_sock) < 0)
    {
        puts("Unable to open recv thread. Exit");
        exit(-1);
    }
    if (pthread_create(&threads[1], NULL, send_handler, &client_send_sock) < 0)
    {
        puts("Unable to open send thread. Exit.");
        exit(-1);
    }
    // if (pthread_create(&threads[2], NULL, gameMessageReceiver, &client_game_sock) < 0){
    //     puts("Unable to open game recv thread. Exit");
    //     exit(-1);
    // }

    // join threads
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
    // pthread_join(threads[2], NULL);

    // close sockets
    close(client_send_sock);
    close(client_recv_sock);
    close(client_game_sock);

    return 0;
}

// void *gameMessageReceiver(void *game_socket) {
//     int game_sock = *(int *)game_socket;

//     char buff[BUFFSIZE];
//     char *msg[MSG_NUM];
//     int recv_bytes;

//     while ((recv_bytes = recv(game_sock, buff, SEND_RECV_LEN, 0)) > 0) {
//         printf(">> Receive: %s\n", buff);
//         meltMsg(buff, msg);
//         if (strcmp(msg[0], "COMPETITOR") == 0)
//         {
//             rivalPoint = atoi(msg[1]);
//             printf("\nrival score 1: %d\n", rivalPoint);
//         }
//     }

//     if (recv_bytes < 0) {
//         perror("Error occurs in connection");
//     } else {
//         puts("Connection closed");
//     }

//     return NULL;
// }

//------------------------------------------------------------

void home(int sock)
{
    int choice;

    do
    {
        if (state == LOGGED_IN && done_leaderboard == 1)
        {
            // system("clear");
            printf("\n-----------Sanh cho-----------");
            printf("\nXin chao %s", current_user->username);
            printf("\n1. Tao phong");
            printf("\n2. Tham gia phong");
            printf("\n3. Xem bang xep hang");
            printf("\n4. Doi mat khau");
            printf("\n5. Xoa tai khoan");
            printf("\n6. Thoat");
            printf("\nLua chon cua ban: ");
            scanf("%d%*c", &choice);
            fflush(stdout);
            switch (choice)
            {
            case 1:
                printf("Nhap level cua phong: ");
                scanf("%d%*c", &level);
                requestCreateRoom(sock, level);
                in_room = 1;
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
                done_leaderboard = 0;
                requestLeaderboard(sock);
                break;
            case 4:
                if(requestChangePassword(sock)){
                    home(sock);
                }
                break;
            case 5:
                break;
            case 6:
                requestLogout(sock);
                break;
            default:
                printf("\nLa sao? Nhap lai coi\n");
                break;
            }
        }

    } while (choice != 6);
}

void roomLobby(int sock)
{
    int choice;
    // printf("in room: %d\n", in_room);
    while (state == IN_ROOM || state == WAITING_RESPONSE)
    {
        // if (state == IN_GAME)
        //     break;
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
                    exitRoom(sock);
                    in_room = 0;
                    break;
                case 3:
                    printf("San sang\n");
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
    // while (state == IN_GAME)
    // {
    //     // play game and send data
    // }
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
            scanf("%d%*c", &choice);
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
        printf("> Recv: %s\n", buff);
        meltMsg(buff, msg);
        if (strcmp(msg[0], "LEADERBOARD") == 0)
        {
            printLeaderboard(msg);
            state = LOGGED_IN;
            done_leaderboard = 1;
            continue;
        }
        if (strcmp(msg[0], "CHANGEPASSWORD") == 0)
        {
            if(strcmp(msg[1], "SUCCESS") == 0){
                puts("\nDoi mat khau thanh cong!!");
                state = LOGGED_IN;
                continue;
            }
            state = LOGGED_IN;
            continue;
        }
        if (strcmp(msg[0], "LOGIN") == 0)
        {
            if (strcmp(msg[1], "SUCCESS") == 0)
            {
                puts("\nDang nhap thanh cong");
                current_user = createUserNode(msg[2], msg[3]);
                current_user->recv_sock = client_recv;
                current_user->send_sock = client_send;
                current_user->game_sock = client_game;
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
            randomNum = atol(msg[1]);
            state = IN_GAME;
            
            send_sock = current_user->send_sock;
            recv_sock = current_user->recv_sock;
            game_sock = current_user->game_sock;
            name = current_user->username;
            srand(time(NULL));
            initscr();
            start_color();
            cbreak();
            init_pair(1, COLOR_GREEN, COLOR_BLACK);
            init_pair(2, COLOR_RED, COLOR_BLACK);
            attron(COLOR_PAIR(1));
            // Room *room = rooms[current_user->room_id];
            startlevel = my_room->room_level;
            for (int i=0;i<=1;i++){
                if (strcmp(current_user->username, my_room->players[i]) != 0){
                    opponent = my_room->players[i];
                    break;
                }
            }

            noecho();
            curs_set(0);
            next = randomNum % 7;
            // while (!game())
            //     {}
            game();
            
            free(name);
            endwin();
            fflush(stdout);
            af_roll = 0;

            continue;
        }

        // if (strcmp(msg[0], "COMPETITOR") == 0)
        // {
        //     printf("Nhan diem doi thu la %s\n", msg[1]);
        //     int competitorScore = atoi(msg[1]);
        //     rivalPoint = atoi(msg[1]);
        //     printf("rival score 1: %d\n", rivalPoint);
        //     continue;
        // }

        if (strcmp(msg[0], "WAITINGRESULT") == 0)
        {
            puts("Vui long doi nguoi choi con lai choi not");
            state = IN_GAME;
            continue;
        }

        if (strcmp(msg[0], "RESULT") == 0)
        {
            puts("\n-------------KET QUA VAN DAU-------------");
            if (strcmp(msg[1], "DRAW") == 0)
            {
                printf("Tran dau hoa");
            }
            else
            {
                printf("Nguoi chien thang la: %s\n", msg[1]);
            }
            puts("\n-------------Tro choi ket thuc-------------\n");
            // printf("test randomNum %ld", randomNum);
            // state = IN_ROOM;
            // room_updating = 0;
            send(send_sock, "exit", SEND_RECV_LEN, 0);
            printf("\nChoi xong roi, ca lang giai tan!!\n");
            continue;
        }
        if (strcmp(msg[0], "ROOMS") == 0)
        {
            // system("clear");
            printf("\n%-62s", "=====================Danh sach cac phong=====================");
            printf("\n%5s|%20s|%14s|%17s|", "ID", "Chu phong", "So nguoi choi", "Cap do cua phong");
            int room_no = atoi(msg[1]);
            for (int i = 0; i < room_no; i++)
            {
                printf("\n%5s|%20s|%12s/2|%17s|", msg[2 + 4 * i], msg[3 + 4 * i], msg[4 + 4 * i], msg[5 + 4 * i]);
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