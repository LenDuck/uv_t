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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include "dlogger.h"

/*
* This code is mainly used for logging keywords from within loops and
* nested loops, those which are hell to use gdb for...
*
* I had it laying around from another experiment (irc client...unfinished.)
*/
dlog_t * dlog_newlog_file(const char *info, const char *fname, int flags){
  dlog_t *ret = malloc(sizeof(dlog_t));
  if (!ret) return NULL;
  
  pthread_mutex_init(&ret->log_mutex, NULL);
  
  ret->file_logging = 0;
  ret->file_logger = fopen(fname,"a");
  if (ret->file_logger) ret->file_logging = 1;
  if (ret->file_logging) fprintf(ret->file_logger,"Log: %s\n\n",info);

  ret->info = malloc(1+strlen(info));
  if (ret->info) strcpy(ret->info,info);

  ret->flags = flags;
  ret->fd_logging = 0;
  ret->fd_logger = -1;
  return ret;
}

int dlog_set_filelog_fname(const char *fname, dlog_t *ll){
  FILE *x = NULL;
  if (ll && fname && (x = fopen(fname, "a"))) {
    ll->file_logging = 1;
    ll->file_logger = x;
    return 0;
  }
  return -1;
}

int dlog_set_fdlog_fname(const char *fname, dlog_t *ll){
  int x;
  if (ll && fname && (x = open(fname, O_APPEND | O_CREAT))) {
    ll->fd_logging = 1;
    ll->fd_logger = x;
    return 0;
  }
  return -1;
}

int dlog_set_fdlog_int(int sockfd, dlog_t *ll){
  if (ll ) {
    ll->fd_logging = 1;
    ll->fd_logger = sockfd;
    return 0;
  }
  return -1;  
  
}

int dlog_log_raw(const void *data,int size, dlog_t *log){
  char buff[128] = {0};
  if (!(log)) return -1;
  
  if (log->flags & (DLOG_FLAG_DISABLE)) return 0;
  
  pthread_mutex_lock(&log->log_mutex);

  if (log->file_logging) {
    if (log->flags & (DLOG_FLAG_RAW)){
      fwrite( data,size,1,log->file_logger);
    } else {
       sprintf(buff,"%%%dhh#x\n",size);
      fprintf(log->file_logger,buff,data);
    }
    if (log->flags & (DLOG_FLAG_FFLUSH)) fflush(log->file_logger);
  }


  if (log->fd_logging) write(log->fd_logger, data, size);
  pthread_mutex_unlock(&log->log_mutex);  
  return 0;
}

int dlog_log_text(const char *data, dlog_t *log){
  
  if (!(log)) return -1;

  if (log->flags & (DLOG_FLAG_DISABLE)) return 0;
    
  pthread_mutex_lock(&log->log_mutex);

  if (log->file_logging) {
    if (log->flags & (DLOG_FLAG_RAW)){
      fwrite( data,strlen(data),1,log->file_logger);
    } else {
      fprintf(log->file_logger,"%s%s",data,((log->flags & DLOG_FLAG_NLINE)?"\n":""));
    }
    if (log->flags & (DLOG_FLAG_FFLUSH)) fflush(log->file_logger);
  }

  if (log->fd_logging) write(log->fd_logger, data, strlen(data));
  pthread_mutex_unlock(&log->log_mutex);  
  
  return 0;
}

void dlog_close_log(dlog_t *vic){
  if (vic){
    pthread_mutex_lock(&vic->log_mutex);
    if (vic->file_logging) fclose(vic->file_logger);
    if (vic->fd_logging) close(vic->fd_logger);
    if (vic->info) free(vic->info);
    free(vic);
  } 
}

