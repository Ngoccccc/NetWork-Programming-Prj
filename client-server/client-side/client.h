#ifndef __CLIENT_H__
#define __CLIENT_H__

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9999

enum {
    WAITING_RESPONSE,
    NOT_LOGGED_IN,
    LOGGED_IN,
    IN_ROOM,
    IN_GAME
};

#endif