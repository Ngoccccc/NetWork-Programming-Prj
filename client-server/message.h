#ifndef __MESSAGE_H_
#define __MESSAGE_H_

#define SEND_RECV_LEN 100
#define BUFFSIZE 100
#define MSG_NUM 30

#define LOGINS "LOGINS" // [from SERVER] require: client(s) to login
#define LOGINC "LOGINC" // [from CLIENT] inform: client logged in,

#define LOGINFAIL1 "LOGINFAIL1" // [from SERVER] inform: username/password incorrect | not exist
#define LOGINFAIL2 "LOGINFAIL2" // [from SERVER] inform: client already logged in (status == 1)
#define LOGGED "LOGGED"         // [from SERVER] inform: login successfully, set status = 1
#define EXIT "EXIT"             // [from CLIENT] --> send [EXIT-username]
#define CREATEROOM "CREATEROOM" // [from CLIENT] require: create a new room,
                                // --> send [CREATEROOM-username]

#define NOROOM "NOROOM"           // [from SERVER] inform: no room left
#define ROOMCREATED "ROOMCREATED" // [from SERVER] inform: a new room created,
                                  // --> send [ROOMCREATED-roomId]

#define STARTC "STARTC" // [from CLIENT] require: to start game,
                        // --> send [STARTC-roomId]

#define STARTED "STARTED"       // [from SERVER] --> send[STARTED-playOrderId] (order to play)
#define ONE "ONE"               // [from SERVER] inform: only 1 person in a room
#define EXITROOM "EXITROOM"     // [from CLIENT] client exit room (EXITROOM-username)
#define ALLOUTROOM "ALLOUTROOM" // [from SERVER] remove all client from a room

#define UPDATE "UPDATE" // [from SERVER] send after being updated state
                        // --> send [UPDATE-state] (state: a string)

#define WIN "WIN"   // [from SERVER] if a player win, send to all client
                    // --> send [WIN-userId-playOrderId]
#define LOSE "LOSE" // [from SERVER] if a player win, send to all client
                    // --> send [WIN-userId-playOrderId]

#define ENDGAME "ENDGAME" // [from SERVER] if only 1 person left when he/she still not won yet.

#endif