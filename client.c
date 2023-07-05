#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSZ 1024

char* msg = NULL;
int id_client = 0;


char* getUserCommand()
{
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);
    printf("Comando> ");
    fgets(buf, BUFSZ-1, stdin);

    buf[strcspn(buf, "\n")] = 0; // remove enter from final char

    char* command = (char*)malloc(strlen(buf) + 1);

    strcpy(command, buf);
    return command;
}

int convertCommand(char* command, char* compl)
{
    if (strcmp(command, "send") == 0 && strcmp(compl, "to") == 0 ) {
        return 1;
    }
    else if (strcmp(command, "send") == 0 && strcmp(compl, "all") == 0 ){
        return 2;
    }
    else if(strcmp(command, "close") == 0)
    {
        return 0 ;
    }
    else {
        return -1;
    }
}

void selectFile(char* filename)
{
    int exists = file_exists(filename);
    int valid = file_valid(filename);

    if (exists == 0 ){
        printf("%s does not exist\n", filename);
        return;
    }

    if (valid == 0 ){
        printf("%s not valid!\n", filename);
        return;
    }

    printf("%s selected\n", filename);  
    msg = get_msg(filename);
    
}

void sendFile(int s)
{   
    if(msg == NULL){
        printf("no file selected!\n");
        return;
    }

    size_t count = send(s, msg, strlen(msg)+1, 0);
    if (count != strlen(msg)+1) 
        logexit("Send");

    memset(msg, 0, BUFSZ);
    unsigned total = 0;
    while (1) {
        count = recv(s, msg + total, BUFSZ - total, 0);
        if (count <= 0) 
            break;
        total += count;
    }
    printf("%s \n",msg);

    msg = NULL;
}

///


message sendMessage(int s, message msg_snd)
{   
    char msg[BUFSZ];
    get_message( &msg_snd ,msg, sizeof(msg));
    printf("send %s \n",msg);

    size_t count = send(s, msg, strlen(msg)+1, 0);
    if (count != strlen(msg)+1) 
        logexit("Send");


    memset(msg, 0, BUFSZ);
    unsigned total = 0;
    count = recv(s, msg + total, BUFSZ - total, 0);
    
    printf("receive %s \n",msg);

    return read_message(&msg);
}

void killsession(int s)
{   
    char* command = "killsession";

    size_t count = send(s, command, strlen(command)+1, 0);
    if (count != strlen(command)+1) 
        logexit("Send");

    // memset(command, 0, BUFSZ);
    // unsigned total = 0;
    // while (1) {
    //     count = recv(s, command + total, BUFSZ - total, 0);
    //     if (count <= 0) 
    //         break;
    //     total += count;
    // }

}

int GetConnection(int s){
    message conn_msg, resp_msg;
    conn_msg.id_msg = 1;
    conn_msg.id_receiver = 0;
    conn_msg.id_sender = 0;
    strcpy(conn_msg.msg , "teste");

    //printf("%s", conn_msg.msg);

    resp_msg = sendMessage(s, conn_msg);

    if(resp_msg.id_msg == 8 )
    {   
        printf("receive %i \n",resp_msg.id_receiver);
        id_client = resp_msg.id_sender;
        return 1 ;
    }

        
    else return 0;

}



int main(int argc, char **argv) {
	if (argc < 3) {
		usage(argc, argv);
	}

    struct sockaddr_storage storage;
    if (addrparse(argv[1], argv[2], &storage) != 0) {
		usage(3, argv);
	}

	int s;
	s = socket(storage.ss_family, SOCK_STREAM, 0);
	if (s == -1) {
		logexit("socket");
	}

    struct sockaddr *addr = (struct sockaddr *)(&storage);
	if (0 != connect(s, addr, sizeof(storage))) {
		logexit("connect");
	}

	char addrstr[BUFSZ];
	addrtostr(addr, addrstr, BUFSZ);

    int resultGetConnection = GetConnection(s);

    if( resultGetConnection == 1)
    {
        int action = 0;
        do
        {
            char comando[20];
            char complemento[20];
            char id_user[20];
            char msg[250];

            char* input = getUserCommand();
        
            sscanf(input, "%s %s %d %s", comando, complemento, id_user, msg);

            printf("%s %s %d %s", comando, complemento, id_user, msg);

            action = convertCommand(comando, complemento);

            if (action == -1)
                printf("invalid command!\n");
            // else if(action == 0)
            // killsession(s);
            else if (action == 1)
            {
                message msg_snd;
                msg_snd.id_msg = 6;
                msg_snd.id_receiver = id_user;
                msg_snd.id_sender = 6;
                //strcpy(msg_snd.msg , 'teste');
                message msg_rslt = sendMessage(s, msg_snd);
            }
            // else if (action == 2)
            //     sendFile(s);

        } while(action != 0);
    }

    
    //printf("connection closed \n");
    close(s);

    exit(EXIT_SUCCESS);

}