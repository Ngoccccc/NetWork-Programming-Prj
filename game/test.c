#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <ncurses.h>
#include <arpa/inet.h>
#include "keys.h"
#include "settings.h"
#include "tetris.h"
char *name;
int sock = 0, valread;
int startlevel;
int randomNum = 1010011010012;
int next;
int main(void)
{
  // main function
  struct sockaddr_in serv_addr;
  char buffer[1024] = {0};
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n Socket creation error \n");
    return -1;
  }
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  // Kết nối tới server
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("\nConnection Failed \n");
    return -1;
  }
  name = malloc(sizeof *name * 14);
  srand(time(NULL));
  initscr();
  start_color();
  cbreak();
  init_pair(1, COLOR_GREEN, COLOR_BLACK);
  init_pair(2, COLOR_RED, COLOR_BLACK);
  attron(COLOR_PAIR(1));
  do
  {
    clear();
    printw("\n\n\n       Enter starting level (1-%d): ", MAXLEVEL);
    refresh();
    scanw("%d ", &startlevel);
  } while (startlevel < 1 || startlevel > MAXLEVEL);
  clear();
  printw("\n\n\n       Enter your name: ");
  refresh();
  scanw("%13s ", name);
  noecho();
  curs_set(0);
  next = randomNum % 7;
  while (!game())
    ;
  free(name);
  endwin();
  close(sock);
  return 0;
}