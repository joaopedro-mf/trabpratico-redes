#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <arpa/inet.h>

#define BUFSZ 1024

int kill = 0;

char *open_conn[15]; 
int last_id_user = 0;

const char* get_extension(const char* msg) {
    char* ext = ".c";
    const char* dot = strchr(msg, '.');

    const char* primeiraletra = dot + 1;
    const char* segundaletra = dot + 2;

    if (*primeiraletra == 'j')
        ext = ".java";
    else if (*primeiraletra == 'p')
        ext = ".py";
    else if (*primeiraletra == 't') {
        if (*segundaletra == 'x')
            ext = ".txt";
        else
            ext = ".tex";
    } else if (*primeiraletra == 'c' && (*segundaletra == 'p'))
        ext = ".cpp";
    
    return ext;
}

char* get_file_name(const char* msg, const char* str) {
    const char* extension = strrchr(msg, '.');
    const char* file_name;
    size_t name_length;
    char* concatenated_string;

    name_length = extension - msg;
    file_name = msg;

    // Calcula o tamanho da nova string concatenada
    size_t new_length = name_length + strlen(str);

    // Aloca memória para a nova string concatenada
    concatenated_string = malloc((new_length + 1) * sizeof(char));
    if (concatenated_string == NULL) {
        fprintf(stderr, "Erro ao alocar memória\n");
        return NULL;
    }

    // Copia o nome do arquivo para a nova string
    strncpy(concatenated_string, file_name, name_length);

    // Concatena a string adicional no final
    strcat(concatenated_string, str);

    return concatenated_string;
}

void remove_first_occurrence(char* str, const char* sub) {
    char* match = strstr(str, sub);
    if (match != NULL) {
        size_t len = strlen(sub);
        memmove(match, match + len, strlen(match + len) + 1);
    }
}

void remove_last_occurrence(char* str, const char* sub) {
    char* match = NULL;
    char* next_match = strstr(str, sub);
    
    while (next_match != NULL) {
        match = next_match;
        next_match = strstr(match + 1, sub);
    }
    
    if (match != NULL) {
        size_t len = strlen(sub);
        memmove(match, match + len, strlen(match + len) + 1);
    }
}


//
//

message add_new_user() {
    int number_clients = (int)( sizeof(clients) / sizeof(clients[0]));
    message resp_msg;

    printf("number clients %d", number_clients);

    if (number_clients >= 15)
    {
        resp_msg.id_msg = 7;
        resp_msg.id_receiver = 0;
        resp_msg.id_sender = 0;
        strcpy(resp_msg.msg , "User limit exceeded");
        return resp_msg; 
    }
        
    last_id_user++;

    resp_msg.id_msg = 8;
    resp_msg.id_receiver = 0;
    resp_msg.id_sender = last_id_user;

    // save client in array

    printf("User %i added", last_id_user);

    //NotifyAll()
    return resp_msg; 

};



struct client_data {
    int csock;
    struct sockaddr_storage storage;
};

void *client_thread(void *data) {

    struct client_data *cdata = (struct client_data *)data;
    struct sockaddr *caddr = (struct sockaddr *)(&cdata->storage);

    char caddrstr[BUFSZ];
    addrtostr(caddr, caddrstr, BUFSZ);
    
    int first_exec = 1;
    int close_conn = 0;
    do{
        char buf[BUFSZ];
        memset(buf, 0, BUFSZ);
        int count = recv(cdata->csock, buf, BUFSZ - 1, 0);
        printf("receive %s \n",buf);

        message arr_message, send_message;
        arr_message = read_message(&buf);

        //printf("%s",arr_message.id_msg);

        if(first_exec == 1)
        {
            if (arr_message.id_msg == 1)
            {
                send_message = add_new_user();
            }
            else{
                printf("user not connect");
                close_conn =1;
            }
           first_exec = 0; 
        }            

        char msg[BUFSZ];
        get_message( &send_message ,msg, sizeof(msg));

        count = send(cdata->csock, msg, strlen(msg) + 1, 0);
        if (count != strlen(msg) + 1)
            logexit("send");

     }while( close_conn == 0);
    

    close(cdata->csock);  
      
    pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
    if (argc < 3) 
        usage(argc, argv);

    struct sockaddr_storage storage;
    if (server_sockaddr_init(argv[1], argv[2], &storage) != 0) 
        usage(argc, argv);

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) 
        logexit("socket");

    int enable = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) != 0) 
        logexit("setsockopt");

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (bind(s, addr, sizeof(storage)) != 0) 
        logexit("bind");

    if (listen(s, 10) != 0) 
        logexit("listen");

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);

    while (1) {
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1)    
        {
            logexit("accept");
        }
        
        struct client_data *cdata = malloc(sizeof(*cdata));
        if (!cdata) 
            logexit("malloc");
        
        cdata->csock = csock;
        memcpy(&(cdata->storage), &storage, sizeof(storage));

        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, cdata);
        pthread_detach(tid);

    }

    exit(EXIT_SUCCESS);
}
