/*
//      
//      Copyright 2010 Leen <chillhamster@gmail.com>
//                     LenDuck
//      
//      License:
//      
//      This code is provided as is,
//      only use/read/compile this code if the author has expressed some form of permission.
*/      

#ifndef H_DIRC
#define H_DIRC

#include "dlogger.h"
typedef struct {
  int sockfd;
  int status;

  dlog_t *logger;
  dlog_t *raw_in;
  dlog_t *raw_out;

} chat_con_t;

chat_con_t *chat_connect(char *host,char *port);

chat_con_t *chat_bootup(char *h,char *p);
int chat_set_user(chat_con_t *con,const char *data);
int chat_say(chat_con_t *con,const char *data);
int chat_quit(chat_con_t *con);

int chat_close(chat_con_t *con);

#endif
