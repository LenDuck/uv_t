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

#ifndef H_DLOG
#define H_DLOG

#include <stdio.h>
#include <pthread.h>

typedef struct {
  pthread_mutex_t log_mutex;
  
  int file_logging;
  FILE *file_logger;
  
  int fd_logging;
  int fd_logger;
  int flags;
  char *info;  
} dlog_t;

#define DLOG_FLAG_RAW (1)
#define DLOG_FLAG_FFLUSH (1<<1)
#define DLOG_FLAG_DISABLE (1<<2)
#define DLOG_FLAG_NLINE (1<<3)


dlog_t * dlog_newlog_file(const char *info, const char *fname, int flags);

int dlog_set_filelog_fname(const char *fname, dlog_t *log);
int dlog_set_fdlog_fname(const char *fname, dlog_t *log);
int dlog_set_fdlog_int(int sockfd, dlog_t *log);

int dlog_log_text(const char *info, dlog_t *log);
int dlog_log_raw(const void *data,int size, dlog_t *log);

void dlog_close_log(dlog_t *vic);
#endif
