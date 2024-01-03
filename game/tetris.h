#ifndef __TETRIS_H__
#define __TETRIS_H__

#define PORT 8080
// DO NOT CHANGE
#define WIDTH 25
#define HEIGHT 24
#define TOPLSITMAXLINELENGTH 34

typedef struct
{
  int roomID;
  int playerNum;
  char *p[4];
} Game;

void show_next();
void updatescrn();
void updatescore();
void toplist();
void addscore();
int gameover();
void checkclr();
void initpiece();
void rotate();
void moveleft();
void moveright();
int movedown();
void init();
void updatelevel();
void setkeybind();
int game();
Game *makeGame(int roomID, int playerNum, char username1[], char username2[], char username3[], char username4[]);

#endif