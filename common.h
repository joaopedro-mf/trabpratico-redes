#pragma once

#include <stdlib.h>

#ifndef COMMON_H   
#define COMMON_H

typedef struct
{
   int id_msg; 
   int id_sender; 
   int id_receiver;
   char msg [256];
} message; 

void usage(int argc, char **argv);
void logexit(const char* msg);
int addrparse(const char *addrstr, const char *portstr, struct sockaddr_storage *storage);
void addrtostr(const struct sockaddr *addr, char *str, size_t strsize);
int server_sockaddr_init(const char *proto, const char *portstr, struct sockaddr_storage *storage);

int file_exists(const char *filename);
int file_valid(const char *filename);
char* get_msg(char *filename);   
int save_file(const char* nomeArquivo, const char* conteudo);

void get_message( const message* send_message, char* str, int size);
message read_message( char *rcv_message );


#endif