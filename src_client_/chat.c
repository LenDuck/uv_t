#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>


#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <pthread.h>
#include <ctype.h>

#include "chat.h"


int chat_send(chat_con_t *con, const void* buf,int len, int flags){
  ssize_t rva = 0;
  if (!con) return -1;
  if (con->logger) dlog_log_text("chat_send",con->logger);

  if (con->status) {
    if (con->logger) dlog_log_text("chat_send old error",con->logger);
    return -1;
  }
  
  rva = send(con->sockfd,buf,len,flags);
  if (con->logger) dlog_log_text(((rva == len)?"ok":"fail"),con->logger);
  return (rva == len);
}

int chat_set_user(chat_con_t *con, const char *data){
  return -1;
}

int chat_say(chat_con_t *con, const char *data){
  return -1;
}

int chat_quit(chat_con_t *con){
  return -1;
}
void* get_in_addr(struct sockaddr * sa){
  if (sa->sa_family == AF_INET){
    return &(
             ( (struct sockaddr_in*) sa)->sin_addr
    );
  }
  return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}



chat_con_t * chat_bootup(char *host,char *port){
  chat_con_t *ret = chat_connect(host,port);
  if (!ret) return NULL;
  
  return ret;
}


chat_con_t * chat_connect(char *host,char *port){
  chat_con_t * ret = malloc(sizeof(chat_con_t));
  
  int sockfd;
  
  struct addrinfo hints, *servinfo, *p;
  int rv;
  char s[INET6_ADDRSTRLEN];
  if (!ret) return NULL;
  
  ret->status = 0;
  s[0] = 0;
  ret->logger = dlog_newlog_file("chat thing", "log_internal", DLOG_FLAG_FFLUSH | DLOG_FLAG_NLINE);
  ret->raw_in = dlog_newlog_file("kat thing", "log_in", DLOG_FLAG_FFLUSH | DLOG_FLAG_RAW);
  ret->raw_out = dlog_newlog_file("kat thing", "log_out",DLOG_FLAG_FFLUSH| DLOG_FLAG_RAW);

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC; /* ipv4/ipv6 whichever*/
  hints.ai_socktype = SOCK_STREAM; /*tcp, not udp*/

  if ((rv = getaddrinfo(host, port, &hints, &servinfo) != 0)){
    fprintf(stderr,"error addrinfo %s", gai_strerror(rv));
  }
  
  for (p=servinfo; p != NULL; p = p->ai_next){
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
      perror("socket");
      continue;
    }
    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
      close(sockfd);
      perror("connect");
      continue;
    }
    /*connected*/
    break;
  }
  if (p == NULL) {
    perror("connect failed");
    return NULL;
  }
  
  inet_ntop(p->ai_family,
     get_in_addr((struct sockaddr *)p->ai_addr),
     s,
     sizeof(INET6_ADDRSTRLEN));
  fprintf(stderr,"CON:%s\n",s);  
  
  freeaddrinfo(servinfo);
  servinfo = NULL;
  
  ret->sockfd = sockfd;

  return ret;
}

int chat_close(chat_con_t *vid){
  if (vid){
    if (vid->logger) dlog_log_text("Closing",vid->logger);
    if (vid->logger) dlog_close_log(vid->logger);
    vid->logger = NULL;

    if (vid->raw_in) dlog_close_log(vid->raw_in);
    if (vid->raw_out) dlog_close_log(vid->raw_out);
    close(vid->sockfd);
    free(vid);
    return 0;
  }
  return -1;
}


