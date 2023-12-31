all: server client

# ----------------------Server----------------------
server: server.o util.o user.o room.o server_room.o server_user.o
	gcc server.o util.o user.o room.o server_room.o server_user.o -pthread -o server

server.o: client-server/server-side/server.c
	gcc -c -Wall client-server/server-side/server.c

# ----------------------Client----------------------
client: client.o util.o user.o room.o client_user.o client_home.o client_game.o
	gcc client.o util.o user.o room.o client_user.o client_home.o client_game.o -pthread -o client

client.o: client-server/client-side/client.c
	gcc -c -Wall client-server/client-side/client.c

# -------------------Dependencies-------------------
util.o: client-server/util.c
	gcc -c -Wall client-server/util.c

user.o: user/user.c
	gcc -c -Wall user/user.c

room.o: room/room.c
	gcc -c -Wall room/room.c
	
server_room.o: client-server/server-side/server_room.c
	gcc -c -Wall client-server/server-side/server_room.c

server_user.o: client-server/server-side/server_user.c
	gcc -c -Wall client-server/server-side/server_user.c

client_user.o: client-server/client-side/client_user.c
	gcc -c -Wall client-server/client-side/client_user.c

client_home.o: client-server/client-side/client_home.c
	gcc -c -Wall client-server/client-side/client_home.c

client_game.o: client-server/client-side/client_game.c
	gcc -c -Wall client-server/client-side/client_game.c

# -------------------------------------------------
# To clean all object file
clean:
	rm -f *.o *~