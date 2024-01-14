#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <ncurses.h>
#include <arpa/inet.h>
#include "keys.h"
#include "settings.h"
#include "../client-server/client-side/client.h"
#include "tetris.h"
#include "../client-server/util.h"

// init variables
char piece;
extern char *name;
extern char *opponent;
extern int send_sock, valread;
extern int recv_sock;
extern int game_sock;
extern int level;
int *competitorPoint = 0;
int rp = 0;
// extern int rivalPoint = 0;
int score, showtext = 1, shownext = 1, end, clrlines = 0;
extern int next;
extern int randomNum;
extern int startlevel, dropped = 0;
int fixedpoint[2] = {0};
// screen is divided into three sections - left, right and center
char left[HEIGHT][WIDTH] = {0};
char center[HEIGHT][WIDTH] = {0};
char right[HEIGHT][WIDTH] = {0};
// all possible tetrominos
char TetrominoI[2][WIDTH] = {"<! . . . . . . . . . .!>",
                             "<! . . .[][][][] . . .!>"};
char TetrominoJ[2][WIDTH] = {"<! . . .[] . . . . . .!>",
                             "<! . . .[][][] . . . .!>"};
char TetrominoL[2][WIDTH] = {"<! . . . . .[] . . . .!>",
                             "<! . . .[][][] . . . .!>"};
char TetrominoO[2][WIDTH] = {"<! . . . .[][] . . . .!>",
                             "<! . . . .[][] . . . .!>"};
char TetrominoS[2][WIDTH] = {"<! . . . .[][] . . . .!>",
                             "<! . . .[][] . . . . .!>"};
char TetrominoT[2][WIDTH] = {"<! . . . .[] . . . . .!>",
                             "<! . . .[][][] . . . .!>"};
char TetrominoZ[2][WIDTH] = {"<! . . .[][] . . . . .!>",
                             "<! . . . .[][] . . . .!>"};
struct timeval t1, t2;

void show_next()
{
       // function shows next tetromino
       switch (next)
       {
       case 0:
              memcpy(left[11] + WIDTH - 10, TetrominoI[1] + 8, 8);
              return;
       case 1:
              memcpy(left[10] + WIDTH - 8, TetrominoJ[0] + 8, 2);
              memcpy(left[11] + WIDTH - 8, TetrominoJ[1] + 8, 6);
              return;
       case 2:
              memcpy(left[10] + WIDTH - 4, TetrominoL[0] + 12, 2);
              memcpy(left[11] + WIDTH - 8, TetrominoL[1] + 8, 6);
              return;
       case 3:
              memcpy(left[10] + WIDTH - 6, TetrominoO[0] + 10, 4);
              memcpy(left[11] + WIDTH - 6, TetrominoO[1] + 10, 4);
              return;
       case 4:
              memcpy(left[10] + WIDTH - 6, TetrominoS[0] + 10, 4);
              memcpy(left[11] + WIDTH - 8, TetrominoS[1] + 8, 4);
              return;
       case 5:
              memcpy(left[10] + WIDTH - 6, TetrominoT[0] + 10, 2);
              memcpy(left[11] + WIDTH - 8, TetrominoT[1] + 8, 6);
              return;
       case 6:
              memcpy(left[10] + WIDTH - 8, TetrominoZ[0] + 8, 4);
              memcpy(left[11] + WIDTH - 6, TetrominoZ[1] + 10, 4);
              return;
       }
}

int setSocketNonBlocking(int sockfd)
{
       int flags = fcntl(sockfd, F_GETFL, 0);
       if (flags == -1)
       {
              perror("fcntl F_GETFL");
              return -1;
       }

       if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1)
       {
              perror("fcntl F_SETFL O_NONBLOCK");
              return -1;
       }

       return 0;
}

void updatescrn()
{
       // self explanatory, refreshes screen
       clear();
       printw("\n");
       for (int i = 2; i < HEIGHT; ++i)
       {
              if (shownext)
                     show_next();
              printw("%s", left[i]);
              printw("%s", center[i]);
              if (showtext)
                     printw("%s", right[i]);
              printw("\n");
       }
       refresh();
       if (shownext)
       {
              memcpy(left[10] + WIDTH - 10, "        ", 8);
              memcpy(left[11] + WIDTH - 10, "        ", 8);
       }
}

void updatescore()
{
       // updates score
       char *tmp = malloc(sizeof *tmp * 15);
       sprintf(tmp, "%-14d", score);

       memcpy(left[7] + 9, tmp, 14);
       free(tmp);
}

void updateCompetitorScore()
{
       char buff[256];
       int recv_bytes;
       char *msg[256];
       setSocketNonBlocking(game_sock);
       // printf("client game sock %d\n", game_sock);

       if ((recv_bytes = recv(game_sock, buff, 256, 0)) > 0)
       {
              // printf(">> Receive: %s\n", buff);
              // send(send_sock, buff, 256, 0);
              meltMsg(buff, msg);
              // if ((strcmp(msg[0], "COMPETITOR") == 0) && strcmp(msg[1], opponent) == 0)
              if ((strcmp(msg[0], "COMPETITOR") == 0))
              {
                     rp = atoi(msg[2]);
              }
       }
       // updates Cscore
       char *tmp = malloc(sizeof *tmp * 15);
       sprintf(tmp, "%-6d", rp);

       memcpy(left[10] + 10, tmp, 6);
       free(tmp);
}

void toplist()
{
       // show toplist
       char *buffer = malloc(sizeof *buffer * TOPLSITMAXLINELENGTH);
       FILE *f;
       clear();
       if (!(f = fopen(TOPLIST, "r")))
              printw("\n\n\n    Toplist doesn't exist! Your score has to be higher than 0"
                     " to be added ;)\n");
       else
       {
              printw("\n");
              while (fgets(buffer, TOPLSITMAXLINELENGTH, f) != NULL)
                     printw("                        %s", buffer);
              fclose(f);
       }
       refresh();
       free(buffer);
       getch();
}

void addscore() {
    if (!score)
        return;

    FILE *f, *tmp;
    if (!(f = fopen(TOPLIST, "r"))) {
        if (!(f = fopen(TOPLIST, "w")))
            exit(1);

        fprintf(f, "NAME          LVL SCORE        \n%-13s %2d  %-14d\n",
                name, level, score);
        fclose(f);
        return;
    }

    int added = 0;
    char *buffer = malloc(sizeof *buffer * TOPLSITMAXLINELENGTH);
    if (!(tmp = fopen("tmp", "a+")))
        exit(1);

    while (fgets(buffer, TOPLSITMAXLINELENGTH, f) != NULL) {
        char currName[128];
        int numScore;
        sscanf(buffer, "%13s %*d %d", currName, &numScore);

        if (strcmp(name, currName) == 0) {
            // Update score only if it's higher
            if (score > numScore) {
                fprintf(tmp, "%-13s %2d  %-14d\n", name, level, score);
                added = 1;
            } else {
                fprintf(tmp, "%-13s %2d  %-14d\n", buffer, level, numScore);
            }
        } else {
            fprintf(tmp, "%-13s %2d  %-14d\n", buffer, level, numScore);
        }
    }

    if (!added)
        fprintf(tmp, "%-13s %2d  %-14d\n", name, level, score);

    fclose(f);
    fclose(tmp);
    remove(TOPLIST);
    rename("tmp", TOPLIST);
    free(buffer);
}

int gameover()
{
       // prints game over screen

       nodelay(stdscr, FALSE);
       if (!end)
              addscore();
       end = 1;
       memcpy(left[9], "    __      __      ___ \0"
                       "  /'_ `\\  /'__`\\  /' __`\0"
                       " /\\ \\L\\ \\/\\ \\L\\.\\_/\\ \\/\\\0"
                       " \\ \\____ \\ \\__/.\\_\\ \\_\\ \0"
                       "  \\/___L\\ \\/__/\\/_/\\/_/\\\0"
                       "    /\\____/             \0"
                       "    \\_/__/              \0",
              WIDTH * 7);
       memcpy(center[9], "___      __         ___ \0"
                         " __`\\  /'__`\\      / __`\0"
                         " \\/\\ \\/\\  __/     /\\ \\L\\\0"
                         "\\_\\ \\_\\ \\____\\    \\ \\___\0"
                         "/_/\\/_/\\/____/     \\/___\0",
              WIDTH * 5);
       memcpy(right[9], "  __  __    __  _ __    \0"
                        "\\/\\ \\/\\ \\ /'__`/\\`'__\\  \0"
                        " \\ \\ \\_/ /\\  __\\ \\ \\/   \0"
                        "_/\\ \\___/\\ \\____\\ \\_\\   \0"
                        "/  \\/__/  \\/____/\\/_/   \0",
              WIDTH * 5);
       memcpy(center[16], "    : QUIT    : RESET   \0"
                          "        : TOPLIST       \0",
              WIDTH * 2);
       center[17][7] = toupper(TPLS);
       clear();
       printw("\n");
       for (int i = 2; i < HEIGHT; ++i)
       {
              if (i == 9 || i == 15 || i == 16)
                     attron(COLOR_PAIR(2));
              else if (i == 18)
                     attron(COLOR_PAIR(1));
              printw("%s", left[i]);
              if (i == 14 || i == 15)
                     attron(COLOR_PAIR(1));
              printw("%s", center[i]);
              printw("%s\n", right[i]);
       }
       refresh();
       return 1;
}

void checkclr()
{
       // check if a line should be cleared
       int cleared = 0;
       if (fixedpoint[0] > 2)
       {
              for (int i = -2; i < 2; ++i)
              {
                     if (!strncmp(center[fixedpoint[0] + i] + 3, "][][][][][][][][][", 18))
                     {
                            ++cleared;
                            for (int j = fixedpoint[0] + i; j > 0; --j)
                                   memcpy(center[j] + 2, center[j - 1] + 2, 20);
                     }
              }
              if (cleared)
              {
                     score += SCORE;
                     score += dropped;
                     dropped = 0;
                     char buff[128];
                     snprintf(buff, sizeof(buff), "UPGRADE-%s-%d", name, score);
                     send(send_sock, buff, strlen(buff), 0);
                     updatescore();
                     updatescrn();
              }
       }
       clrlines += cleared;
}

void initpiece()
{
       // initializes a new piece
       checkclr();
       int current;
       randomNum = randomNum - 5;
       current = next;

       if (randomNum == 1)
       {
              randomNum += 10;
       }
       if (randomNum < 10)
       {
              randomNum = randomNum * randomNum;
       }
       next = randomNum % 7;
       // int receivedNumber = randomNum;
       // while (receivedNumber / 7 < 1)
       // {
       //        // Gửi yêu cầu lấy số nguyên từ server
       //        char request[] = "SEND_RANDOM";
       //        send(sock, request, strlen(request), 0);

       //        // Nhận phản hồi từ server
       //        char buffer[1024];
       //        int bytesRead = recv(sock, buffer, sizeof(buffer), 0);
       //        if (bytesRead < 0)
       //        {
       //               perror("Receive failed");
       //               exit(EXIT_FAILURE);
       //        }

       //        // Chuyển đổi dữ liệu nhận được thành số nguyên
       //        receivedNumber = atoi(buffer);
       // }
       // randomNum = receivedNumber;

       switch (current)
       {
       case 0:
              memcpy(center[0], TetrominoI[0], WIDTH * 2);
              fixedpoint[0] = 1;
              fixedpoint[1] = 12;
              piece = 'I';
              return;
       case 1:
              memcpy(center[0], TetrominoJ[0], WIDTH * 2);
              fixedpoint[0] = 1;
              fixedpoint[1] = 10;
              piece = 'J';
              return;
       case 2:
              memcpy(center[0], TetrominoL[0], WIDTH * 2);
              fixedpoint[0] = 1;
              fixedpoint[1] = 10;
              piece = 'L';
              return;
       case 3:
              memcpy(center[0], TetrominoO[0], WIDTH * 2);
              fixedpoint[0] = 1;
              fixedpoint[1] = 10;
              piece = 'O';
              return;
       case 4:
              memcpy(center[0], TetrominoS[0], WIDTH * 2);
              fixedpoint[0] = 0;
              fixedpoint[1] = 10;
              piece = 'S';
              return;
       case 5:
              memcpy(center[0], TetrominoT[0], WIDTH * 2);
              fixedpoint[0] = 1;
              fixedpoint[1] = 10;
              piece = 'T';
              return;
       case 6:
              memcpy(center[0], TetrominoZ[0], WIDTH * 2);
              fixedpoint[0] = 0;
              fixedpoint[1] = 10;
              piece = 'Z';
              return;
       }
}

void rotate()
{
       // rotate current piece (clockwise obviously)
       switch (piece)
       {
       case 'I':
              if (center[fixedpoint[0] + 1][fixedpoint[1]] == '[' || center[fixedpoint[0] + 1][fixedpoint[1]] == '=' || center[fixedpoint[0] - 1][fixedpoint[1]] == '[' || center[fixedpoint[0] - 2][fixedpoint[1]] == '[')
                     return;
              memcpy(center[fixedpoint[0] - 2] + fixedpoint[1],
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1],
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 4,
                     TetrominoI[0] + fixedpoint[1] - 4, 4);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1],
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              piece = 'i';
              return;
       case 'i':
              if (center[fixedpoint[0]][fixedpoint[1] + 2] == '[' || center[fixedpoint[0]][fixedpoint[1] + 2] == '!' || center[fixedpoint[0]][fixedpoint[1] - 2] == '[' || center[fixedpoint[0]][fixedpoint[1] - 4] == '[' || center[fixedpoint[0]][fixedpoint[1] - 4] == '<')
                     return;
              memcpy(center[fixedpoint[0] - 2] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 4,
                     TetrominoI[1] + 8, 4);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 2,
                     TetrominoI[1] + 12, 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 2);
              piece = 'I';
              return;
       case 'J':
              if (center[fixedpoint[0] + 1][fixedpoint[1]] == '[' || center[fixedpoint[0] + 1][fixedpoint[1]] == '=' || center[fixedpoint[0] - 1][fixedpoint[1]] == '[' || center[fixedpoint[0] - 1][fixedpoint[1] + 2] == '[')
                     return;
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] - 2,
                     TetrominoO[0] + 8, 6);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     TetrominoI[0] + fixedpoint[1] - 2, 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1],
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              piece = 'K';
              return;
       case 'K':
              if (center[fixedpoint[0]][fixedpoint[1] - 2] == '[' || center[fixedpoint[0]][fixedpoint[1] - 2] == '<' || center[fixedpoint[0]][fixedpoint[1] + 2] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] + 2] == '[')
                     return;
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 4);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 2,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1],
                     TetrominoT[0] + 8, 4);
              piece = 'j';
              return;
       case 'j':
              if (center[fixedpoint[0] + 1][fixedpoint[1]] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] - 2] == '[' || center[fixedpoint[0] - 1][fixedpoint[1]] == '[')
                     return;
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1],
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     TetrominoI[0] + fixedpoint[1] - 2, 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] - 2,
                     TetrominoO[0] + 10, 6);
              piece = 'k';
              return;
       case 'k':
              if (center[fixedpoint[0]][fixedpoint[1] + 2] == '[' || center[fixedpoint[0]][fixedpoint[1] + 2] == '!' || center[fixedpoint[0]][fixedpoint[1] - 2] == '[' || center[fixedpoint[0] - 1][fixedpoint[1] - 2] == '[')
                     return;
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] - 2,
                     TetrominoT[0] + 10, 4);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 2,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] - 2,
                     TetrominoI[0] + fixedpoint[1] - 2, 4);
              piece = 'J';
              return;
       case 'L':
              if (center[fixedpoint[0] + 1][fixedpoint[1]] == '[' || center[fixedpoint[0] + 1][fixedpoint[1]] == '=' || center[fixedpoint[0] - 1][fixedpoint[1]] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] + 2] == '[')
                     return;
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1],
                     TetrominoT[0] + 10, 4);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     TetrominoI[0] + fixedpoint[1] - 2, 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1],
                     TetrominoO[0] + 10, 4);
              piece = 'M';
              return;
       case 'M':
              if (center[fixedpoint[0]][fixedpoint[1] - 2] == '[' || center[fixedpoint[0]][fixedpoint[1] - 2] == '<' || center[fixedpoint[0]][fixedpoint[1] + 2] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] - 2] == '[')
                     return;
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 2,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] - 2,
                     TetrominoT[0] + 10, 6);
              piece = 'l';
              return;
       case 'l':
              if (center[fixedpoint[0] - 1][fixedpoint[1]] == '[' || center[fixedpoint[0] - 1][fixedpoint[1] - 2] == '[' || center[fixedpoint[0] + 1][fixedpoint[1]] == '[')
                     return;
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] - 2,
                     TetrominoT[0] + 8, 4);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     TetrominoI[0] + fixedpoint[1] - 2, 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] - 2,
                     TetrominoO[0] + 10, 4);
              piece = 'm';
              return;
       case 'm':
              if (center[fixedpoint[0]][fixedpoint[1] + 2] == '[' || center[fixedpoint[0]][fixedpoint[1] + 2] == '!' || center[fixedpoint[0]][fixedpoint[1] - 2] == '[' || center[fixedpoint[0] - 1][fixedpoint[1] + 2] == '[')
                     return;
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] - 2,
                     TetrominoT[0] + 6, 6);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 2,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 2);
              piece = 'L';
              return;
       case 'S':
              if (center[fixedpoint[0] - 1][fixedpoint[1]] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] + 2] == '[')
                     return;
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1],
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] - 2,
                     TetrominoT[0] + 6, 6);
              piece = 's';
              return;
       case 's':
              if (center[fixedpoint[0] + 1][fixedpoint[1]] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] - 2] == '<' || center[fixedpoint[0] + 1][fixedpoint[1] - 2] == '[')
                     return;
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] - 2,
                     TetrominoO[0] + 10, 6);
              piece = 'S';
              return;
       case 'T':
              if (center[fixedpoint[0] + 1][fixedpoint[1]] == '[' || center[fixedpoint[0] + 1][fixedpoint[1]] == '=')
                     return;
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     TetrominoI[0] + fixedpoint[1] - 2, 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1],
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              piece = 'U';
              return;
       case 'U':
              if (center[fixedpoint[0]][fixedpoint[1] - 2] == '[' || center[fixedpoint[0]][fixedpoint[1] - 2] == '<')
                     return;
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              piece = 't';
              return;
       case 't':
              if (center[fixedpoint[0] - 1][fixedpoint[1]] == '[')
                     return;
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1],
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              piece = 'u';
              return;
       case 'u':
              if (center[fixedpoint[0]][fixedpoint[1] + 2] == '[' || center[fixedpoint[0]][fixedpoint[1] + 2] == '!')
                     return;
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 2,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              piece = 'T';
              return;
       case 'Z':
              if (center[fixedpoint[0] - 1][fixedpoint[1] + 2] == '[' || center[fixedpoint[0]][fixedpoint[1] + 2] == '[')
                     return;
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] + 2,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     TetrominoO[0] + 8, 6);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] + 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              piece = 'z';
              return;
       case 'z':
              if (center[fixedpoint[0]][fixedpoint[1] - 2] == '[' || center[fixedpoint[0]][fixedpoint[1] - 2] == '<' || center[fixedpoint[0] + 1][fixedpoint[1] + 2] == '[')
                     return;
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] + 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     TetrominoO[0] + 10, 6);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] + 2,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              piece = 'Z';
              return;
       }
}

void moveleft()
{
       // move current piece to the left
       switch (piece)
       {
       case 'I':
              if (center[fixedpoint[0]][fixedpoint[1] + -6] == '[' || center[fixedpoint[0]][fixedpoint[1] - 6] == '<')
                     return;
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 6,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              fixedpoint[1] -= 2;
              return;
       case 'i':
              if (center[fixedpoint[0]][fixedpoint[1] - 2] == '<')
                     return;
              for (int i = -2; i < 2; ++i)
              {
                     if (center[fixedpoint[0] + i][fixedpoint[1] - 2] == '[')
                            return;
              }
              for (int i = -2; i < 2; ++i)
              {
                     memcpy(center[fixedpoint[0] + i] + fixedpoint[1] - 2,
                            TetrominoT[0] + 10, 4);
              }
              fixedpoint[1] -= 2;
              return;
       case 'J':
              if (center[fixedpoint[0]][fixedpoint[1] - 4] == '[' || center[fixedpoint[0]][fixedpoint[1] - 4] == '<' || center[fixedpoint[0] - 1][fixedpoint[1] - 4] == '[')
                     return;
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 4,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] - 4,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] - 2,
                     TetrominoI[0] + fixedpoint[1] - 2, 2);
              fixedpoint[1] -= 2;
              return;
       case 'K':
              if (center[fixedpoint[0]][fixedpoint[1] - 2] == '[' || center[fixedpoint[0]][fixedpoint[1] - 2] == '<' || center[fixedpoint[0] - 1][fixedpoint[1] - 2] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] - 2] == '[')
                     return;
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] - 2,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] + 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     TetrominoT[0] + 10, 4);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] - 2,
                     TetrominoT[0] + 10, 4);
              fixedpoint[1] -= 2;
              return;
       case 'j':
              if (center[fixedpoint[0]][fixedpoint[1] - 4] == '[' || center[fixedpoint[0]][fixedpoint[1] - 4] == '<' || center[fixedpoint[0] + 1][fixedpoint[1]] == '[')
                     return;
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 4,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1],
                     TetrominoT[0] + 10, 4);
              fixedpoint[1] -= 2;
              return;
       case 'k':
              if (center[fixedpoint[0]][fixedpoint[1] - 2] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] - 4] == '<' || center[fixedpoint[0] - 1][fixedpoint[1] - 2] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] - 4] == '[')
                     return;
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] - 2,
                     TetrominoT[0] + 10, 4);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     TetrominoT[0] + 10, 4);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] - 4,
                     center[fixedpoint[0]] + fixedpoint[1] - 2, 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 2);
              fixedpoint[1] -= 2;
              return;
       case 'L':
              if (center[fixedpoint[0]][fixedpoint[1] - 4] == '[' || center[fixedpoint[0]][fixedpoint[1] - 4] == '<' || center[fixedpoint[0] - 1][fixedpoint[1]] == '[')
                     return;
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 4,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1],
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] + 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              fixedpoint[1] -= 2;
              return;
       case 'M':
              if (center[fixedpoint[0]][fixedpoint[1] - 2] == '[' || center[fixedpoint[0]][fixedpoint[1] - 2] == '<' || center[fixedpoint[0] - 1][fixedpoint[1] - 2] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] - 2] == '[')
                     return;
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] - 2,
                     TetrominoT[0] + 10, 4);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] + 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     TetrominoT[0] + 10, 4);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] - 2,
                     TetrominoT[0] + 10, 2);
              fixedpoint[1] -= 2;
              return;
       case 'l':
              if (center[fixedpoint[0]][fixedpoint[1] - 4] == '[' || center[fixedpoint[0]][fixedpoint[1] - 4] == '<' || center[fixedpoint[0] + 1][fixedpoint[1] - 4] == '[')
                     return;
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 4,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] - 4,
                     TetrominoT[0] + 10, 4);
              fixedpoint[1] -= 2;
              return;
       case 'm':
              if (center[fixedpoint[0]][fixedpoint[1] - 2] == '[' || center[fixedpoint[0] - 1][fixedpoint[1] - 4] == '<' || center[fixedpoint[0] + 1][fixedpoint[1] - 2] == '[' || center[fixedpoint[0] - 1][fixedpoint[1] - 4] == '[')
                     return;
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] - 2,
                     TetrominoT[0] + 10, 4);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     TetrominoT[0] + 10, 4);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] - 4,
                     center[fixedpoint[0]] + fixedpoint[1] - 2, 2);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 2);
              fixedpoint[1] -= 2;
              return;
       case 'O':
              if (center[fixedpoint[0]][fixedpoint[1] - 2] == '[' || center[fixedpoint[0]][fixedpoint[1] - 2] == '<' || center[fixedpoint[0] - 1][fixedpoint[1] - 2] == '[')
                     return;
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] - 2,
                     center[fixedpoint[0] - 1] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] + 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              fixedpoint[1] -= 2;
              return;
       case 'S':
              if (center[fixedpoint[0]][fixedpoint[1] - 2] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] - 4] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] - 4] == '<')
                     return;
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] - 4,
                     center[fixedpoint[0] + 1] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 2);
              fixedpoint[1] -= 2;
              return;
       case 's':
              if (center[fixedpoint[0] - 1][fixedpoint[1] - 2] == '[' || center[fixedpoint[0] - 1][fixedpoint[1] - 2] == '<' || center[fixedpoint[0]][fixedpoint[1] - 2] == '[' || center[fixedpoint[0] + 1][fixedpoint[1]] == '[')
                     return;
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] - 2,
                     TetrominoT[0] + 10, 4);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     TetrominoO[0] + 10, 6);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1],
                     TetrominoT[0] + 10, 4);
              fixedpoint[1] -= 2;
              return;
       case 'T':
              if (center[fixedpoint[0]][fixedpoint[1] - 4] == '[' || center[fixedpoint[0]][fixedpoint[1] - 4] == '<' || center[fixedpoint[0] - 1][fixedpoint[1] - 2] == '[')
                     return;
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 4,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] - 2,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 2);
              fixedpoint[1] -= 2;
              return;
       case 'U':
              if (center[fixedpoint[0]][fixedpoint[1] - 2] == '[' || center[fixedpoint[0]][fixedpoint[1] - 2] == '<' || center[fixedpoint[0] - 1][fixedpoint[1] - 2] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] - 2] == '[')
                     return;
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] - 2,
                     TetrominoT[0] + 10, 4);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     TetrominoO[0] + 10, 6);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] - 2,
                     TetrominoT[0] + 10, 4);
              fixedpoint[1] -= 2;
              return;
       case 't':
              if (center[fixedpoint[0]][fixedpoint[1] - 4] == '[' || center[fixedpoint[0]][fixedpoint[1] - 4] == '<' || center[fixedpoint[0] + 1][fixedpoint[1] - 2] == '[')
                     return;
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 4,
                     TetrominoT[0] + 10, 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] - 2,
                     TetrominoT[0] + 10, 4);
              fixedpoint[1] -= 2;
              return;
       case 'u':
              if (center[fixedpoint[0]][fixedpoint[1] - 4] == '[' || center[fixedpoint[0]][fixedpoint[1] - 4] == '<' || center[fixedpoint[0] - 1][fixedpoint[1] - 2] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] - 2] == '[')
                     return;
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] - 2,
                     TetrominoT[0] + 10, 4);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 4,
                     TetrominoO[0] + 10, 6);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] - 2,
                     TetrominoT[0] + 10, 4);
              fixedpoint[1] -= 2;
              return;
       case 'Z':
              if (center[fixedpoint[0]][fixedpoint[1] - 4] == '[' || center[fixedpoint[0]][fixedpoint[1] - 4] == '<' || center[fixedpoint[0] + 1][fixedpoint[1] - 2] == '[')
                     return;
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 4,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] - 2,
                     center[fixedpoint[0] + 1] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] + 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              fixedpoint[1] -= 2;
              return;
       case 'z':
              if (center[fixedpoint[0] - 1][fixedpoint[1]] == '[' || center[fixedpoint[0]][fixedpoint[1] - 2] == '<' || center[fixedpoint[0]][fixedpoint[1] - 2] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] - 2] == '[')
                     return;
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1],
                     TetrominoT[0] + 10, 4);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     TetrominoO[0] + 10, 6);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] - 2,
                     TetrominoT[0] + 10, 4);
              fixedpoint[1] -= 2;
              return;
       }
}

void moveright()
{
       // move current piece to the right
       switch (piece)
       {
       case 'I':
              if (center[fixedpoint[0]][fixedpoint[1] + 4] == '[' || center[fixedpoint[0]][fixedpoint[1] + 4] == '!')
                     return;
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 4,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 4,
                     TetrominoI[0] + fixedpoint[1] - 4, 2);
              fixedpoint[1] += 2;
              return;
       case 'i':
              if (center[fixedpoint[0]][fixedpoint[1] + 2] == '!')
                     return;
              for (int i = -2; i < 2; ++i)
              {
                     if (center[fixedpoint[0] + i][fixedpoint[1] + 2] == '[')
                            return;
              }
              for (int i = -2; i < 2; ++i)
              {
                     memcpy(center[fixedpoint[0] + i] + fixedpoint[1],
                            TetrominoT[0] + 8, 4);
              }
              fixedpoint[1] += 2;
              return;
       case 'J':
              if (center[fixedpoint[0]][fixedpoint[1] + 4] == '[' || center[fixedpoint[0]][fixedpoint[1] + 4] == '!' || center[fixedpoint[0] - 1][fixedpoint[1]] == '[')
                     return;
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 4,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     TetrominoI[0] + fixedpoint[1] - 2, 2);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1],
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] - 2,
                     TetrominoI[0] + fixedpoint[1] - 2, 2);
              fixedpoint[1] += 2;
              return;
       case 'K':
              if (center[fixedpoint[0] - 1][fixedpoint[1] + 4] == '[' || center[fixedpoint[0] - 1][fixedpoint[1] + 4] == '!' || center[fixedpoint[0]][fixedpoint[1] + 2] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] + 2] == '[')
                     return;
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] + 4,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1],
                     TetrominoT[0] + 8, 4);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1],
                     TetrominoT[0] + 8, 4);
              fixedpoint[1] += 2;
              return;
       case 'j':
              if (center[fixedpoint[0]][fixedpoint[1] + 4] == '[' || center[fixedpoint[0]][fixedpoint[1] + 4] == '!' || center[fixedpoint[0] + 1][fixedpoint[1] + 4] == '[')
                     return;
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 4,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] + 2,
                     TetrominoT[0] + 8, 4);
              fixedpoint[1] += 2;
              return;
       case 'k':
              if (center[fixedpoint[0]][fixedpoint[1] + 2] == '[' || center[fixedpoint[0]][fixedpoint[1] + 2] == '!' || center[fixedpoint[0] - 1][fixedpoint[1] + 2] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] + 2] == '[')
                     return;
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1],
                     TetrominoT[0] + 8, 4);
              memcpy(center[fixedpoint[0]] + fixedpoint[1],
                     TetrominoT[0] + 8, 4);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] - 2,
                     TetrominoI[0] + fixedpoint[1] - 2, 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] + 2,
                     center[fixedpoint[0]] + fixedpoint[1] + 2, 2);
              fixedpoint[1] += 2;
              return;
       case 'L':
              if (center[fixedpoint[0]][fixedpoint[1] + 4] == '[' || center[fixedpoint[0]][fixedpoint[1] + 4] == '!' || center[fixedpoint[0] - 1][fixedpoint[1] + 4] == '[')
                     return;
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 4,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     TetrominoI[0] + fixedpoint[1] - 2, 2);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] + 4,
                     center[fixedpoint[0]] + fixedpoint[1] + 4, 2);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] + 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              fixedpoint[1] += 2;
              return;
       case 'M':
              if (center[fixedpoint[0] - 1][fixedpoint[1] + 2] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] + 4] == '!' || center[fixedpoint[0]][fixedpoint[1] + 2] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] + 4] == '[')
                     return;
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] + 4,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1],
                     TetrominoT[0] + 8, 4);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1],
                     TetrominoT[0] + 8, 4);
              fixedpoint[1] += 2;
              return;
       case 'l':
              if (center[fixedpoint[0]][fixedpoint[1] + 4] == '[' || center[fixedpoint[0]][fixedpoint[1] + 4] == '!' || center[fixedpoint[0] + 1][fixedpoint[1]] == '[')
                     return;
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 4,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] - 2,
                     TetrominoT[0] + 8, 4);
              fixedpoint[1] += 2;
              return;
       case 'm':
              if (center[fixedpoint[0]][fixedpoint[1] + 2] == '[' || center[fixedpoint[0]][fixedpoint[1] + 2] == '!' || center[fixedpoint[0] - 1][fixedpoint[1] + 2] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] + 2] == '[')
                     return;
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1],
                     TetrominoT[0] + 8, 4);
              memcpy(center[fixedpoint[0]] + fixedpoint[1],
                     TetrominoT[0] + 8, 4);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] - 2,
                     TetrominoI[0] + fixedpoint[1] - 2, 2);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] + 2,
                     center[fixedpoint[0]] + fixedpoint[1] + 2, 2);
              fixedpoint[1] += 2;
              return;
       case 'O':
              if (center[fixedpoint[0]][fixedpoint[1] + 4] == '[' || center[fixedpoint[0]][fixedpoint[1] + 4] == '!' || center[fixedpoint[0] - 1][fixedpoint[1] + 4] == '[')
                     return;
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 4,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] + 4,
                     center[fixedpoint[0] - 1] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 2);
              fixedpoint[1] += 2;
              return;
       case 'S':
              if (center[fixedpoint[0]][fixedpoint[1] + 4] == '[' || center[fixedpoint[0]][fixedpoint[1] + 4] == '!' || center[fixedpoint[0] + 1][fixedpoint[1] + 2] == '[')
                     return;
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 4,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] + 2,
                     center[fixedpoint[0] + 1] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] - 2,
                     TetrominoI[0] + fixedpoint[1] - 2, 2);
              fixedpoint[1] += 2;
              return;
       case 's':
              if (center[fixedpoint[0] - 1][fixedpoint[1] + 2] == '[' || center[fixedpoint[0]][fixedpoint[1] + 4] == '!' || center[fixedpoint[0]][fixedpoint[1] + 4] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] + 4] == '[')
                     return;
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1],
                     TetrominoT[0] + 8, 4);
              memcpy(center[fixedpoint[0]] + fixedpoint[1],
                     TetrominoO[0] + 8, 6);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] + 2,
                     TetrominoT[0] + 8, 4);
              fixedpoint[1] += 2;
              return;
       case 'T':
              if (center[fixedpoint[0]][fixedpoint[1] + 4] == '[' || center[fixedpoint[0]][fixedpoint[1] + 4] == '!' || center[fixedpoint[0] - 1][fixedpoint[1] + 2] == '[')
                     return;
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 4,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     TetrominoI[0] + fixedpoint[1] - 2, 2);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] + 2,
                     center[fixedpoint[0]] + fixedpoint[1] + 2, 2);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 2);
              fixedpoint[1] += 2;
              return;
       case 'U':
              if (center[fixedpoint[0]][fixedpoint[1] + 4] == '[' || center[fixedpoint[0]][fixedpoint[1] + 4] == '!' || center[fixedpoint[0] - 1][fixedpoint[1] + 2] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] + 2] == '[')
                     return;
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1],
                     TetrominoT[0] + 8, 4);
              memcpy(center[fixedpoint[0]] + fixedpoint[1],
                     TetrominoO[0] + 8, 6);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1],
                     TetrominoT[0] + 8, 4);
              fixedpoint[1] += 2;
              return;
       case 't':
              if (center[fixedpoint[0]][fixedpoint[1] + 4] == '[' || center[fixedpoint[0]][fixedpoint[1] + 4] == '!' || center[fixedpoint[0] + 1][fixedpoint[1] + 2] == '[')
                     return;
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 4,
                     TetrominoT[0] + 10, 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1],
                     TetrominoT[0] + 8, 4);
              fixedpoint[1] += 2;
              return;
       case 'u':
              if (center[fixedpoint[0]][fixedpoint[1] + 2] == '[' || center[fixedpoint[0]][fixedpoint[1] + 2] == '!' || center[fixedpoint[0] - 1][fixedpoint[1] + 2] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] + 2] == '[')
                     return;
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1],
                     TetrominoT[0] + 8, 4);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     TetrominoO[0] + 8, 6);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1],
                     TetrominoT[0] + 8, 4);
              fixedpoint[1] += 2;
              return;
       case 'Z':
              if (center[fixedpoint[0]][fixedpoint[1] + 2] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] + 4] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] + 4] == '!')
                     return;
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 2,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     TetrominoI[0] + fixedpoint[1] - 2, 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] + 4,
                     center[fixedpoint[0] + 1] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 2);
              fixedpoint[1] += 2;
              return;
       case 'z':
              if (center[fixedpoint[0] - 1][fixedpoint[1] + 4] == '[' || center[fixedpoint[0]][fixedpoint[1] + 4] == '!' || center[fixedpoint[0]][fixedpoint[1] + 4] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] + 2] == '[')
                     return;
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] + 2,
                     TetrominoT[0] + 8, 4);
              memcpy(center[fixedpoint[0]] + fixedpoint[1],
                     TetrominoO[0] + 8, 6);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1],
                     TetrominoT[0] + 8, 4);
              fixedpoint[1] += 2;
              return;
       }
}
int movedown()
{
       // move current piece down
       switch (piece)
       {
       case 'I':
              if (center[fixedpoint[0] + 1][fixedpoint[1] - 4] == '=')
              {
                     initpiece();
                     return 0;
              }
              for (int i = 0; i < 4; ++i)
              {
                     if (center[fixedpoint[0] + 1][fixedpoint[1] - 4 + i * 2] == '[')
                     {
                            if (fixedpoint[0] == 1 || fixedpoint[0] == 2)
                                   return gameover();
                            initpiece();
                            return 0;
                     }
              }
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] - 4,
                     center[fixedpoint[0]] + fixedpoint[1] - 4, 8);
              memcpy(center[fixedpoint[0]++] + fixedpoint[1] - 4,
                     TetrominoI[0] + fixedpoint[1] - 4, 8);
              return 0;
       case 'i':
              if (center[fixedpoint[0] + 2][fixedpoint[1]] == '[' || center[fixedpoint[0] + 2][fixedpoint[1]] == '=')
              {
                     if (fixedpoint[0] == 1 || fixedpoint[0] == 2)
                            return gameover();
                     initpiece();
                     return 0;
              }
              memcpy(center[fixedpoint[0] + 2] + fixedpoint[1],
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]++ - 2] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 2);
              return 0;
       case 'J':
              if (center[fixedpoint[0] + 1][fixedpoint[1] - 2] == '=')
              {
                     initpiece();
                     return 0;
              }
              for (int i = 0; i < 3; ++i)
              {
                     if (center[fixedpoint[0] + 1][fixedpoint[1] - 2 + i * 2] == '[')
                     {
                            if (fixedpoint[0] == 1 || fixedpoint[0] == 2)
                                   return gameover();
                            initpiece();
                            return 0;
                     }
              }
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] - 2,
                     center[fixedpoint[0]] + fixedpoint[1] - 2, 6);
              memcpy(center[fixedpoint[0]] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 4);
              memcpy(center[fixedpoint[0]++ - 1] + fixedpoint[1] - 2,
                     TetrominoI[0] + fixedpoint[1] - 2, 2);
              return 0;
       case 'K':
              if (center[fixedpoint[0] + 2][fixedpoint[1]] == '[' || center[fixedpoint[0] + 2][fixedpoint[1]] == '=' || center[fixedpoint[0]][fixedpoint[1] + 2] == '[')
              {
                     if (fixedpoint[0] == 1 || fixedpoint[0] == 2)
                            return gameover();
                     initpiece();
                     return 0;
              }
              memcpy(center[fixedpoint[0] + 2] + fixedpoint[1],
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 4);
              memcpy(center[fixedpoint[0]++] + fixedpoint[1] + 2,
                     TetrominoT[0] + 10, 2);
              return 0;
       case 'j':
              if (center[fixedpoint[0] + 2][fixedpoint[1] + 2] == '[' || center[fixedpoint[0] + 2][fixedpoint[1] + 2] == '=' || center[fixedpoint[0] + 1][fixedpoint[1]] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] - 2] == '[')
              {
                     if (fixedpoint[0] == 1 || fixedpoint[0] == 2)
                            return gameover();
                     initpiece();
                     return 0;
              }
              memcpy(center[fixedpoint[0] + 2] + fixedpoint[1] + 2,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     TetrominoI[0] + fixedpoint[1] - 2, 6);
              memcpy(center[fixedpoint[0]++ + 1] + fixedpoint[1] - 2,
                     TetrominoO[0] + 10, 4);
              return 0;
       case 'k':
              if (center[fixedpoint[0] + 2][fixedpoint[1]] == '[' || center[fixedpoint[0] + 2][fixedpoint[1]] == '=' || center[fixedpoint[0] + 2][fixedpoint[1] - 2] == '[')
              {
                     if (fixedpoint[0] == 1 || fixedpoint[0] == 2)
                            return gameover();
                     initpiece();
                     return 0;
              }
              memcpy(center[fixedpoint[0] + 2] + fixedpoint[1] - 2,
                     TetrominoO[0] + 10, 4);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]++ + 1] + fixedpoint[1] - 2,
                     TetrominoI[0] + fixedpoint[1] - 2, 2);
              return 0;
       case 'L':
              if (center[fixedpoint[0] + 1][fixedpoint[1] - 2] == '=')
              {
                     initpiece();
                     return 0;
              }
              for (int i = 0; i < 3; ++i)
              {
                     if (center[fixedpoint[0] + 1][fixedpoint[1] - 2 + i * 2] == '[')
                     {
                            if (fixedpoint[0] == 1 || fixedpoint[0] == 2)
                                   return gameover();
                            initpiece();
                            return 0;
                     }
              }
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] - 2,
                     center[fixedpoint[0]] + fixedpoint[1] - 2, 6);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     TetrominoI[0] + fixedpoint[1] - 2, 4);
              memcpy(center[fixedpoint[0]++ - 1] + fixedpoint[1] + 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              return 0;
       case 'M':
              if (center[fixedpoint[0] + 2][fixedpoint[1]] == '[' || center[fixedpoint[0] + 2][fixedpoint[1]] == '=' || center[fixedpoint[0] + 2][fixedpoint[1] + 2] == '[')
              {
                     if (fixedpoint[0] == 1 || fixedpoint[0] == 2)
                            return gameover();
                     initpiece();
                     return 0;
              }
              memcpy(center[fixedpoint[0] + 2] + fixedpoint[1],
                     TetrominoO[0] + 10, 4);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]++ + 1] + fixedpoint[1] + 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              return 0;
       case 'l':
              if (center[fixedpoint[0] + 2][fixedpoint[1] - 2] == '[' || center[fixedpoint[0] + 2][fixedpoint[1] - 2] == '=' || center[fixedpoint[0] + 1][fixedpoint[1]] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] + 2] == '[')
              {
                     if (fixedpoint[0] == 1 || fixedpoint[0] == 2)
                            return gameover();
                     initpiece();
                     return 0;
              }
              memcpy(center[fixedpoint[0] + 2] + fixedpoint[1] - 2,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     TetrominoI[0] + fixedpoint[1] - 2, 6);
              memcpy(center[fixedpoint[0]++ + 1] + fixedpoint[1],
                     TetrominoO[0] + 10, 4);
              return 0;
       case 'm':
              if (center[fixedpoint[0] + 2][fixedpoint[1]] == '[' || center[fixedpoint[0] + 2][fixedpoint[1]] == '=' || center[fixedpoint[0]][fixedpoint[1] - 2] == '[')
              {
                     if (fixedpoint[0] == 1 || fixedpoint[0] == 2)
                            return gameover();
                     initpiece();
                     return 0;
              }
              memcpy(center[fixedpoint[0] + 2] + fixedpoint[1],
                     TetrominoT[0] + 10, 2);
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] - 2,
                     TetrominoI[0] + fixedpoint[1] - 2, 4);
              memcpy(center[fixedpoint[0]++] + fixedpoint[1] - 2,
                     TetrominoT[0] + 10, 2);
              return 0;
       case 'O':
              if (center[fixedpoint[0] + 1][fixedpoint[1]] == '=')
              {
                     initpiece();
                     return 0;
              }
              for (int i = 0; i < 2; ++i)
              {
                     if (center[fixedpoint[0] + 1][fixedpoint[1] + i * 2] == '[')
                     {
                            if (fixedpoint[0] == 1 || fixedpoint[0] == 2)
                                   return gameover();
                            initpiece();
                            return 0;
                     }
              }
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1],
                     center[fixedpoint[0]] + fixedpoint[1], 4);
              memcpy(center[fixedpoint[0]++ - 1] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 4);
              return 0;
       case 'S':
              if (center[fixedpoint[0] + 2][fixedpoint[1] - 2] == '=')
              {
                     initpiece();
                     return 0;
              }
              for (int i = 0; i < 2; ++i)
              {
                     if (center[fixedpoint[0] + 2][fixedpoint[1] - 2 + i * 2] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] + 2] == '[')
                     {
                            if (fixedpoint[0] == 1 || fixedpoint[0] == 2)
                                   return gameover();
                            initpiece();
                            return 0;
                     }
              }
              memcpy(center[fixedpoint[0] + 2] + fixedpoint[1] - 2,
                     center[fixedpoint[0] + 1] + fixedpoint[1] - 2, 4);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] - 2,
                     TetrominoS[0] + 8, 6);
              memcpy(center[fixedpoint[0]++] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 4);
              return 0;
       case 's':
              if (center[fixedpoint[0] + 1][fixedpoint[1]] == '[' || center[fixedpoint[0] + 2][fixedpoint[1]] == '=' || center[fixedpoint[0] + 2][fixedpoint[1] + 2] == '[')
              {
                     if (fixedpoint[0] == 1 || fixedpoint[0] == 2)
                            return gameover();
                     initpiece();
                     return 0;
              }
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1],
                     TetrominoT[0] + 10, 4);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1],
                     TetrominoO[0] + 10, 4);
              memcpy(center[fixedpoint[0]++ + 2] + fixedpoint[1] + 2,
                     TetrominoT[0] + 10, 2);
              return 0;
       case 'T':
              if (center[fixedpoint[0] + 1][fixedpoint[1] - 2] == '=')
              {
                     initpiece();
                     return 0;
              }
              for (int i = 0; i < 3; ++i)
              {
                     if (center[fixedpoint[0] + 1][fixedpoint[1] - 2 + i * 2] == '[')
                     {
                            if (fixedpoint[0] == 1 || fixedpoint[0] == 2)
                                   return gameover();
                            initpiece();
                            return 0;
                     }
              }
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] - 2,
                     center[fixedpoint[0]] + fixedpoint[1] - 2, 6);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     TetrominoT[0] + 8, 6);
              memcpy(center[fixedpoint[0]++ - 1] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 2);
              return 0;
       case 'U':
              if (center[fixedpoint[0] + 2][fixedpoint[1]] == '[' || center[fixedpoint[0] + 2][fixedpoint[1]] == '=' || center[fixedpoint[0] + 1][fixedpoint[1] + 2] == '[')
              {
                     if (fixedpoint[0] == 1 || fixedpoint[0] == 2)
                            return gameover();
                     initpiece();
                     return 0;
              }
              memcpy(center[fixedpoint[0] + 2] + fixedpoint[1],
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] + 2,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] + 2,
                     TetrominoI[0] + fixedpoint[1] + 2, 2);
              memcpy(center[fixedpoint[0]++ - 1] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 2);
              return 0;
       case 't':
              if (center[fixedpoint[0] + 2][fixedpoint[1]] == '[' || center[fixedpoint[0] + 2][fixedpoint[1]] == '=' || center[fixedpoint[0] + 1][fixedpoint[1] + 2] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] - 2] == '[')
              {
                     if (fixedpoint[0] == 1 || fixedpoint[0] == 2)
                            return gameover();
                     initpiece();
                     return 0;
              }
              memcpy(center[fixedpoint[0] + 2] + fixedpoint[1],
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] + 2,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] - 2,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]++] + fixedpoint[1] - 2,
                     TetrominoI[0] + fixedpoint[1] - 2, 6);
              return 0;
       case 'u':
              if (center[fixedpoint[0] + 2][fixedpoint[1]] == '[' || center[fixedpoint[0] + 2][fixedpoint[1]] == '=' || center[fixedpoint[0] + 1][fixedpoint[1] - 2] == '[')
              {
                     if (fixedpoint[0] == 1 || fixedpoint[0] == 2)
                            return gameover();
                     initpiece();
                     return 0;
              }
              memcpy(center[fixedpoint[0] + 2] + fixedpoint[1],
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] - 2,
                     center[fixedpoint[0]] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1] - 2,
                     TetrominoI[0] + fixedpoint[1] - 2, 2);
              memcpy(center[fixedpoint[0]++ - 1] + fixedpoint[1],
                     TetrominoI[0] + fixedpoint[1], 2);
              return 0;
       case 'Z':
              if (center[fixedpoint[0] + 2][fixedpoint[1]] == '=')
              {
                     initpiece();
                     return 0;
              }
              for (int i = 0; i < 2; ++i)
              {
                     if (center[fixedpoint[0] + 2][fixedpoint[1] + i * 2] == '[' || center[fixedpoint[0] + 1][fixedpoint[1] - 2] == '[')
                     {
                            if (fixedpoint[0] == 1 || fixedpoint[0] == 2)
                                   return gameover();
                            initpiece();
                            return 0;
                     }
              }
              memcpy(center[fixedpoint[0] + 2] + fixedpoint[1],
                     center[fixedpoint[0] + 1] + fixedpoint[1], 4);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1] - 2,
                     TetrominoZ[0] + 8, 6);
              memcpy(center[fixedpoint[0]++] + fixedpoint[1] - 2,
                     TetrominoI[0] + fixedpoint[1] - 2, 4);
              return 0;
       case 'z':
              if (center[fixedpoint[0] + 1][fixedpoint[1] + 2] == '[' || center[fixedpoint[0] + 2][fixedpoint[1]] == '=' || center[fixedpoint[0] + 2][fixedpoint[1]] == '[')
              {
                     if (fixedpoint[0] == 1 || fixedpoint[0] == 2)
                            return gameover();
                     initpiece();
                     return 0;
              }
              memcpy(center[fixedpoint[0] - 1] + fixedpoint[1] + 2,
                     TetrominoI[0] + fixedpoint[1], 2);
              memcpy(center[fixedpoint[0]] + fixedpoint[1],
                     TetrominoT[0] + 8, 4);
              memcpy(center[fixedpoint[0] + 1] + fixedpoint[1],
                     TetrominoO[0] + 10, 4);
              memcpy(center[fixedpoint[0]++ + 2] + fixedpoint[1],
                     TetrominoT[0] + 10, 2);
              return 0;
       }
       return 0;
}

void init()
{
       // initializes screen
       memcpy(left[0], "                        \0"
                       "                        \0"
                       "                        \0"
                       "  PLAYER:               \0"
                       "                        \0"
                       "  LEVEL:                \0"
                       "                        \0"
                       "  SCORE:                \0"
                       "                        \0"
                       "  OPP:                  \0"
                       "  OSCORE:               \0"
                       "                        \0"
                       "                        \0"
                       "                        \0"
                       "                        \0"
                       "                        \0"
                       "                        \0"
                       "                        \0"
                       "                        \0"
                       "                        \0"
                       "                        \0"
                       "                        \0"
                       "                        \0"
                       "                        \0",
              (HEIGHT) * (WIDTH));
       memcpy(center[0], "<! . . . . . . . . . .!>\0"
                         "<! . . . . . . . . . .!>\0"
                         "<! . . . . . . . . . .!>\0"
                         "<! . . . . . . . . . .!>\0"
                         "<! . . . . . . . . . .!>\0"
                         "<! . . . . . . . . . .!>\0"
                         "<! . . . . . . . . . .!>\0"
                         "<! . . . . . . . . . .!>\0"
                         "<! . . . . . . . . . .!>\0"
                         "<! . . . . . . . . . .!>\0"
                         "<! . . . . . . . . . .!>\0"
                         "<! . . . . . . . . . .!>\0"
                         "<! . . . . . . . . . .!>\0"
                         "<! . . . . . . . . . .!>\0"
                         "<! . . . . . . . . . .!>\0"
                         "<! . . . . . . . . . .!>\0"
                         "<! . . . . . . . . . .!>\0"
                         "<! . . . . . . . . . .!>\0"
                         "<! . . . . . . . . . .!>\0"
                         "<! . . . . . . . . . .!>\0"
                         "<! . . . . . . . . . .!>\0"
                         "<! . . . . . . . . . .!>\0"
                         "<!====================!>\0"
                         "  \\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\0",
              (HEIGHT) * (WIDTH));
       memcpy(right[0], "                        \0"
                        "                        \0"
                        "                        \0"
                        "    : LEFT     :RIGHT   \0"
                        "         :ROTATE        \0"
                        "    : DROP              \0"
                        "    : SHOW/HIDE NEXT    \0"
                        "    : HIDE THIS TEXT    \0"
                        "                        \0"
                        "                        \0"
                        "                        \0"
                        "                        \0"
                        "                        \0"
                        "                        \0"
                        "                        \0"
                        "                        \0"
                        "                        \0"
                        "                        \0"
                        "                        \0"
                        "                        \0"
                        "                        \0"
                        "                        \0"
                        "                        \0"
                        "                        \0",
              (HEIGHT) * (WIDTH));
}

void updatelevel()
{
       // updates level
       char *tmp = malloc(sizeof *tmp * 15);
       sprintf(tmp, "%-14d", level);
       memcpy(left[5] + 9, tmp, 14);
       free(tmp);
}

void setkeybind()
{
       // reads key bindings from config
       right[3][3] = toupper(MOVL);
       right[3][14] = toupper(MOVR);
       right[4][8] = toupper(ROTA);
       right[5][3] = toupper(DROP);
       right[6][3] = toupper(SNXT);
       right[7][3] = toupper(STXT);
}

int game()
{
       // simulates a game of tetris
       nodelay(stdscr, FALSE);
       init();
       setkeybind();
       clear();
       end = 0;
       score = 0;
       level = startlevel;
       clrlines = 0;
       memcpy(left[3] + 10, name, strlen(name));
       memcpy(left[9] + 10, opponent, strlen(opponent));
       getch();
       updatescore();

       updatelevel();
       initpiece();
       updatescrn();
       nodelay(stdscr, TRUE);
       gettimeofday(&t1, NULL);
       while (!usleep(DELAY))
       {
              updateCompetitorScore();
              if (end)
              {
                     char gameover[128];
                     snprintf(gameover, sizeof(gameover), "GAMEOVER-%s", name);
                     send(send_sock, gameover, strlen(gameover), 0);
                     return 1;
              }
              switch (getch())
              {
              case DROP:
                     if (movedown())
                            continue;
                     ++dropped;
                     updatescrn();
                     continue;
              case STXT:
                     if (end)
                            continue;
                     showtext = !showtext;
                     updatescrn();
                     continue;
              case SNXT:
                     if (end)
                            continue;
                     shownext = !shownext;
                     updatescrn();
                     continue;
              case MOVR:
                     if (end)
                            continue;
                     moveright();
                     updatescrn();
                     continue;
              case MOVL:
                     if (end)
                            continue;
                     moveleft();
                     updatescrn();
                     continue;
              case ROTA:
                     if (end || fixedpoint[0] < 2)
                            continue;
                     rotate();
                     updatescrn();
                     continue;
              case TPLS:
                     if (!end)
                            continue;
                     toplist();
              }
              if (end)
              {
                     gameover();
                     continue;
              }
              if (clrlines == LINESFORLVLUP)
              {
                     if (level < MAXLEVEL)
                     {
                            ++level;
                            updatelevel();
                            clrlines = 0;
                     }
              }
              gettimeofday(&t2, NULL);
              if ((((t2.tv_sec - t1.tv_sec) * 1000) + ((t2.tv_usec - t1.tv_usec) / 1000)) > DROPINTERVAL)
              {
                     if (movedown())
                            continue;
                     updatescrn();
                     gettimeofday(&t1, NULL);
              }
       }
       return 1;
}

void setCompetitorPoint(int *point)
{
       competitorPoint = point;
}
