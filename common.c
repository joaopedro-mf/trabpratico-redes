#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <sys/stat.h>     

typedef struct
{
   int id_msg; 
   int id_sender; 
   int id_receiver;
   char msg[256];
} message; 


void usage(int argc, char **argv) {
    printf("Uso Cliente: programa <endereço IP> <porta>\n");
    printf("Uso Servidor: %s <v4|v6> <porta do servidor>\n", argv[0]);
    exit(EXIT_FAILURE);
}

void logexit(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int addrparse(const char *addrstr, const char *portstr, struct sockaddr_storage *storage) {
    if (addrstr == NULL || portstr == NULL) {
        return -1;
    }

    uint16_t port = (uint16_t)atoi(portstr);
    if (port == 0) {
        return -1;
    }
    port = htons(port);

    struct in_addr inaddr4;
    if (inet_pton(AF_INET, addrstr, &inaddr4) == 1) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        addr4->sin_addr = inaddr4;
        return 0;
    }

    struct in6_addr inaddr6;
    if (inet_pton(AF_INET6, addrstr, &inaddr6) == 1) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = port;
        memcpy(&(addr6->sin6_addr), &inaddr6, sizeof(inaddr6));
        return 0;
    }

    return -1;
}

//#define INET_ADDRSTRLEN 46

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize) {
    int version;
    char addrstr[INET_ADDRSTRLEN + 1] = "";
    uint16_t port;

    if (addr->sa_family == AF_INET) {
        version = 4;
        struct sockaddr_in *addr4 = (struct sockaddr_in *)addr;
        if (inet_ntop(AF_INET, &(addr4->sin_addr), addrstr, INET_ADDRSTRLEN + 1) == NULL) {
            logexit("ntop");
        }
        port = ntohs(addr4->sin_port);
    } else if (addr->sa_family == AF_INET6) {
        version = 6;
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr;
        if (inet_ntop(AF_INET6, &(addr6->sin6_addr), addrstr, INET_ADDRSTRLEN + 1) == NULL) {
            logexit("ntop");
        }
        port = ntohs(addr6->sin6_port);
    } else {
        logexit("unknown protocol family.");
    }

    if (str) {
        snprintf(str, strsize, "IPv%d %s %hu", version, addrstr, port);
    }
}

int server_sockaddr_init(const char *proto, const char *portstr, struct sockaddr_storage *storage) {
    uint16_t port = (uint16_t)atoi(portstr);

    if (port == 0) {
        return -1;
    }

    port = htons(port);

    memset(storage, 0, sizeof(*storage));

    if (strcmp(proto, "v4") == 0) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_addr.s_addr = INADDR_ANY;
        addr4->sin_port = port;
        return 0;
    } else if (strcmp(proto, "v6") == 0) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_addr = in6addr_any;
        addr6->sin6_port = port;
        return 0;
    } else {
        return -1;
    }
}

int file_exists(const char *filename) {
   FILE *file;
    if ((file = fopen(filename, "r")))
    {
        fclose(file);
        return 1;
    }
    return 0;
}

const char* get_filename_ext(const char* filename) {
    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename)
        return "";
    return dot + 1;
}

int file_valid(const char* filename) {
    const char* fileextension = get_filename_ext(filename);

    return (strcmp(fileextension, "txt") == 0 || strcmp(fileextension, "c") == 0 ||
            strcmp(fileextension, "cpp") == 0 || strcmp(fileextension, "py") == 0 ||
            strcmp(fileextension, "tex") == 0 || strcmp(fileextension, "java") == 0);
}


char* get_file(char *filename)
{
    FILE *arquivo = fopen(filename, "r");

    if (arquivo == NULL)
        return NULL;

    fseek(arquivo, 0, SEEK_END);
    long tamanho = ftell(arquivo);
    fseek(arquivo, 0, SEEK_SET);

    char* conteudo = (char*)malloc(tamanho + 5);
    if (conteudo == NULL) {
        fclose(arquivo);
        return NULL;
    }

    strncpy(conteudo + tamanho, "\\end", 5); 
    fread(conteudo, 1, tamanho, arquivo);
     
    fclose(arquivo);
    return conteudo;
}

char* get_msg(char *filename)
{
    char* conteudo = get_file(filename);
    
    if(conteudo == NULL)
        return NULL;

    return strcat(filename, conteudo);  // Concatena str2 em str1
}

int save_file(const char* nomeArquivo, const char* conteudo) {
    FILE* arquivo = fopen(nomeArquivo, "w");
    if (arquivo == NULL) 
        return 0;

    fputs(conteudo, arquivo);
    fclose(arquivo);

    return 1;
}

//

void get_message( const message* msg, char* str, int size){
    snprintf(str, size, "%d,%d,%d,%s", msg->id_msg, msg->id_sender, msg->id_receiver, msg->msg);
}

message read_message( char *rcv_message ){
    
    int id_msg, id_send, id_rcvr;
    char msg[256];
    sscanf(rcv_message, "%d,%d,%d,%s", &id_msg, &id_send, &id_rcvr, &msg);

    message rtn_msg;
    rtn_msg.id_msg = id_msg;
    rtn_msg.id_sender = id_send;
    rtn_msg.id_receiver = id_rcvr;
    strcpy(rtn_msg.msg , msg);

    return rtn_msg;
}

