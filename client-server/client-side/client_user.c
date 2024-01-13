#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include "../message.h"
#include "client_user.h"
#include "client.h"

extern UserNode* current_user;

// -------------- Request -------------------
int requestLogin(int sock){
    char username[BUFFSIZE];
    char password[BUFFSIZE];
    char message[BUFFSIZE];
    // char response[256];
    while(state == NOT_LOGGED_IN || state == WAITING_RESPONSE){
        if(state == NOT_LOGGED_IN){
            printf("\n----Dang nhap----");
            printf("\n> Ten nguoi dung (go 'quit' de thoat): "); scanf("%s", username);
            if(strcmp(username, "quit") == 0)
                return 0;
            printf("> Mat khau: "); scanf("%s", password);
            strcpy(message, "LOGIN-");        // message
            strcat(message, username);      // message created in these 4 lines
            strcat(message, "-");           //
            strcat(message, password);      //
            send(sock, message, SEND_RECV_LEN , 0);
            state = WAITING_RESPONSE;
        }
    }
    return 1;
}

void requestLogout(int sock){
    char buff[BUFFSIZE];
    strcpy(buff, "LOGOUT-");                // message created in these 2 lines
    strcat(buff, current_user->username);   //
    send(sock, buff, SEND_RECV_LEN, 0);
    state = WAITING_RESPONSE;
}

int requestSignup(int sock){
    char username[BUFFSIZE];
    char password[BUFFSIZE];
    char repassword[BUFFSIZE];
    char message[BUFFSIZE];
    while(state == NOT_LOGGED_IN || state == WAITING_RESPONSE){
        if(state == NOT_LOGGED_IN){
            while(1){
                // system("clear");
                printf("\n----Dang ki----");
                printf("\n> Ten nguoi dung (go 'quit' de thoat): "); scanf("%s", username);
                if(strcmp(username, "quit") == 0)
                    return 0;
                printf("> Mat khau: "); scanf("%s", password);
                printf("> Xac nhan lai mat khau: "); scanf("%s", repassword);
                if(strcmp(password, repassword) == 0)
                    break;
                else printf(">> Xac nhan mat khau khong chinh xac. Vui long nhap lai.");
            }
            snprintf(message, sizeof(message), "SIGNUP-%s-%s", username,password);
            send(sock, message, SEND_RECV_LEN , 0);
            state = WAITING_RESPONSE;
        }
    }
    return 1;
}

int requestChangePassword(int sock) {
    char oldPassword[BUFFSIZE];
    char newPassword[BUFFSIZE];
    char confirmPassword[BUFFSIZE];
    char message[BUFFSIZE];

    printf("\n----Doi mat khau----");
    printf("\n> Mat khau hien tai: "); scanf("%s", oldPassword);
    printf("> Mat khau moi: "); scanf("%s", newPassword);
    printf("> Xac nhan lai mat khau moi: "); scanf("%s", confirmPassword);

    if (strcmp(newPassword, confirmPassword) != 0) {
        printf(">> Xac nhan mat khau moi khong chinh xac. Vui long nhap lai.\n");
        return 0;
    }

    snprintf(message, sizeof(message), "CHANGEPASSWORD-%s-%s-%s", oldPassword, newPassword, confirmPassword);
    send(sock, message, SEND_RECV_LEN , 0);
    printf(">> Doi mat khau thanh cong\n");
    state = WAITING_RESPONSE;

    return 1;
}

// int requestDeleteUser(int sock) {
//     char username[BUFFSIZE];
//     char message[BUFFSIZE];

//     printf("\n----Xoa tai khoan----");
//     printf("\n> Nhap ten nguoi dung (go 'quit' de thoat): "); scanf("%s", username);
    
//     if(strcmp(username, "quit") == 0)
//         return 0;

//     snprintf(message, sizeof(message), "DELETEUSER-%s", username);
//     send(sock, message, SEND_RECV_LEN , 0);
//     state = WAITING_RESPONSE;

//     return 1;
// }