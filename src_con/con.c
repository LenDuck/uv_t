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
#include <errno.h>

#include "dlogger.h"
#include "con.h"
struct struct_con{
  int sockfd;
  int connected;
  int status;

  dlog_t *logger;
  dlog_t *raw_in;
  dlog_t *raw_out;
  
  char *host;
  char *port;

  /*used for multi strings receive*/
  int buffer_index;
  char *buffer;
  int buffer_filled_to;
  int buffer_size;
  
};

/* error codes, copied from header
CON_ERROR_NONE (0)
CON_ERROR_CLOSED (1)
CON_ERROR_MEMORY (2)
CON_ERROR_UNKNOWN (3)
CON_ERROR_INCOMPLETE (4)
*/
#define CON_DISCONNECTED (0)
#define CON_CONNECTED (1)
#define CON_LISTENING (2)

con_t con_bootup(char *h,char *p){
  con_t ret = NULL;

  ret = malloc(sizeof(struct struct_con));
  if (!ret) return NULL;
  ret->host = NULL;
  ret->port = NULL;

  if (h){
    ret->host = malloc(1+strlen(h));
    if (!ret->host){
      free(ret);
      return NULL;
    }
    strcpy(ret->host,h);
  }
  if (p){
    ret->port = malloc(1+strlen(p));
    if (!ret->port){
      free(ret->host);
      free(ret);
      return NULL;
    }
    strcpy(ret->port,p);
  }

  ret->status = CON_ERROR_NONE;
  ret->buffer = NULL;
  ret->buffer_size = 0;
  ret->buffer_index = -1;
  ret->buffer_filled_to = -1;
  ret->logger = dlog_newlog_file("mainlog con","con.log",DLOG_FLAG_NLINE | DLOG_FLAG_FFLUSH);
  ret->raw_in = dlog_newlog_file("log in con","in.raw",DLOG_FLAG_RAW | DLOG_FLAG_FFLUSH);
  ret->raw_out = dlog_newlog_file("log out con","out.raw",DLOG_FLAG_RAW | DLOG_FLAG_FFLUSH);
  
  if (!(ret->logger && ret->raw_in && ret->raw_out)) ret->status = CON_ERROR_MEMORY;
  
  ret->connected = CON_DISCONNECTED;
  
  return ret;
}

int con_init_serve(con_t con, con_t *newcon){
  
  struct addrinfo hints,*servinfo, *p;
  int yes = 1;
/*  struct addrinfo client_info;*/
  int rva;

  if (!con) return CON_ERROR_MEMORY;
  if (! (con->connected == CON_DISCONNECTED)) return CON_ERROR_CLOSED;

  /*Prepare the connection settings thing*/
  memset(&hints, 0, sizeof(struct addrinfo));
//  hints.ai_family = AF_UNSPEC; /* ipv4/ipv6 whichever*/
  hints.ai_family = AF_INET;/*ipv4*/
  hints.ai_socktype = SOCK_STREAM; /*tcp, not udp*/
  hints.ai_flags = AI_PASSIVE;
  /*Lookup the address*/
  rva =  getaddrinfo(NULL, con->port, &hints, &servinfo);
  if (rva){
    fprintf(stderr,"error addrinfo %s", gai_strerror(rva));
  }

  for (p=servinfo; p != NULL; p = p->ai_next){
    
    con->sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (con->sockfd == -1){
      perror("sock");
      continue;
    }
    
    if (setsockopt(con->sockfd, SOL_SOCKET, SO_REUSEADDR ,&yes,sizeof(int)) == -1){
      perror("pockopt");
      return -1;
    }
    
    rva = bind(con->sockfd, p->ai_addr, p->ai_addrlen);
    if (rva == -1){
      close(con->sockfd);
      perror("bind");
      continue; 
    }
    /*If you made it here you are connected*/
    break;
  }
  if (p == NULL){
    perror("bind failed");
/*fixme, memory loss*/
    return CON_ERROR_MEMORY;
  }
  freeaddrinfo(servinfo);
  servinfo = NULL;
  if (listen(con->sockfd, 1) == -1 ){
    perror("Listen");
  }

  con->connected = CON_LISTENING;
  return con_serve_accept(con,newcon);
}
/*Accept a single connection, blocking until one is obtained*/
int con_serve_accept(con_t con, con_t *newcon){
  if ((!con) || (!newcon)) return CON_ERROR_MEMORY;
  if (! (con->connected == CON_LISTENING)) return CON_ERROR_CLOSED;
  while (1){
    con_t client = NULL;
    struct sockaddr_storage client_addr;
    unsigned int size_addr = sizeof(struct sockaddr_storage);
    int newfd = accept(con->sockfd, (struct sockaddr*) &client_addr, &size_addr);
    if (newfd == -1 ) {
      perror("accept");
      continue;
    }
    printf("Got con\n");
    client = con_bootup(NULL,con->port);
    /*check nil*/
    client->sockfd = newfd;
    client->connected = 1;
    *newcon = client;
  
    return CON_ERROR_NONE;
  }

}

int con_serve(con_t con, con_t *newcon){
  /*fix rebinding, here*/
  if (!con) return CON_ERROR_MEMORY;
  if (!newcon) return CON_ERROR_MEMORY;

  
  if (con->connected == CON_DISCONNECTED){
    return con_init_serve(con,newcon);
  }
  
  if (con->connected == CON_LISTENING){
    return con_serve_accept(con,newcon);
  }
  
  return CON_ERROR_CLOSED;
}

int con_connect(con_t con){
  struct addrinfo hints,*servinfo, *p;
  int rva;

  if (!con) return CON_ERROR_MEMORY;
  if (!(con->connected == CON_DISCONNECTED)) return CON_ERROR_CLOSED;
  /*Prepare the connection settings thing*/
  memset(&hints, 0, sizeof(struct addrinfo));
/*  hints.ai_family = AF_UNSPEC;  ipv4/ipv6 whichever*/
  hints.ai_family = AF_INET; 
  hints.ai_socktype = SOCK_STREAM; /*tcp, not udp*/
  
  /*Lookup the address*/
  rva =  getaddrinfo(con->host, con->port, &hints, &servinfo);
  if (rva){
    fprintf(stderr,"error addrinfo %s", gai_strerror(rva));
  }

  for (p=servinfo; p != NULL; p = p->ai_next){
    
    con->sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (con->sockfd == -1){
      perror("sock");
      continue;
    }
    
    rva = connect(con->sockfd, p->ai_addr, p->ai_addrlen);
    if (rva == -1){
      close(con->sockfd);
      perror("connect");
      continue; 
    }
    /*If you made it here you are connected*/
    break;
  }
  if (p == NULL){
    perror("Connect failed");
/*fixme, memory loss*/
    return CON_ERROR_MEMORY;
  }
  freeaddrinfo(servinfo);
  servinfo = NULL;
  con->connected = 1;

  return CON_ERROR_NONE;
}

/*Append your own newlines and stuff*/
/*if size == 0, uses strlen*/
int con_send(con_t con, void *data,int size){
  int status;
  int flags = MSG_NOSIGNAL;/*No signals please*/
  
  if (! (con && data && (size > -1))) return CON_ERROR_MEMORY;
  if (size == 0) size = strlen(data);
  
  if (!con->connected) return CON_ERROR_CLOSED;
  if (con->logger) dlog_log_text("Send",con->logger);
  if (con->raw_out) dlog_log_raw(data,size,con->raw_out);  
  errno = 0;
  status = send(con->sockfd, data,size,flags);
  if (status == -1){
    perror("sendfail");
    if (con->logger) dlog_log_text("Failure in sending",con->logger);
    return CON_ERROR_UNKNOWN;
  }
  if (errno == EPIPE){
    con->connected = 0;
    return CON_ERROR_CLOSED;
  }

  return CON_ERROR_NONE;
}
/*do your OWN newlines*/
int con_send_line(con_t con,char *data){
  return con_send(con,data,strlen(data));
}

/*
  If *target == NULL, auto alocates (a block of *size big)
  If size == NULL, picks a size around 2K big (2k > MTU ?)
  Returns the length of the received data or -1
  or even 0 if the connection is closed
*/
int con_recv(con_t con, void **target,int size){
  int buffersize = 2048;
  void *buffer = NULL;
  int rva = 0;
  int flags = 0;

  if (!(con && target)) return -1;
  if (! con->connected) return -1;
  if (con->logger) dlog_log_text("Recv?",con->logger);
  if (!size){
    if (con->logger) dlog_log_text("nosize",con->logger);
   /*Using standard size, might add print here*/
    size = buffersize; 
  }

  if (! *target){
    if (con->logger) dlog_log_text("nobuffer",con->logger);
    /*allocate a big buffer*/
    buffer = malloc(size);
    if (!buffer){
      if (con->logger) dlog_log_text("Buffer error",con->logger);
      return -1;
    }
    *target = buffer;
  }
  if (con->logger) dlog_log_text("R",con->logger);

  rva = recv(con->sockfd, *target, size, flags);
  if (rva == 0) {
    perror("ccon");
    printf("data %p,%d,%d\n",(void*)target,buffersize,flags);
    fflush(stdout);
    printf("buffer %p\n",(void*)*target);
    if (con->logger) dlog_log_text("Closed the connection",con->logger);
  } else if (rva == -1){
    if (con->logger) dlog_log_text("Error in receive",con->logger);
    con->status = CON_ERROR_UNKNOWN;
  }
  if (con->raw_in) dlog_log_raw(*target,rva,con->raw_in);

  return rva;
}

static int is_linesep(char x){
  return (x == '\n') || (x == '\r') || (x == 0);
}


/*Sets *target to a valid line, if one ever is received, returns leftover size in packet*/
/*Cuts of all newlines, con_error returned*/
int con_line(con_t con, char **target){
  int rva = 0;
  char *buffer = NULL;
  int bufsize = 0;

  if (!(con && target)) return CON_ERROR_MEMORY;
  
  if (! (con->connected || (con->buffer && (con->buffer_index > -1)))) return CON_ERROR_CLOSED;
  if (con->logger) dlog_log_text("Line",con->logger);
  while (1){
    if (con->logger) dlog_log_text("While",con->logger);

    /*buffer still full?*/
    if ((con->buffer_index > -1) && (con->buffer_size > con->buffer_index)){
      
      int id = 0;
      
      if (! con->buffer) exit(2);
      if (con->logger) dlog_log_text("bw",con->logger);

      for (id=0;con->buffer_index < con->buffer_filled_to; con->buffer_index++){
        if (buffer && is_linesep(con->buffer[con->buffer_index]) && strlen(buffer)){
          /*not to append, but if buffer is filled accept as line*/ 
          *target = buffer;
          return CON_ERROR_NONE;
        } else {
          if (bufsize <= (id +1 )) {
            bufsize = id + 16;
            buffer = realloc(buffer,bufsize );
          }
          if (!buffer) exit(1);/*fixme*/
          buffer[id] = con->buffer[con->buffer_index];
          id++;
          buffer[id] = 0;
        }
        
      }
      if (con->logger) dlog_log_text("cb",con->logger);

      con->buffer_filled_to = 0;
    }
    if (con->buffer_index >= con->buffer_filled_to){
      con->buffer_index = 0;    
      con->buffer_filled_to = 0;
    }
    if (con->buffer_size == 0){
      con->buffer_size = 2048;
      con->buffer = malloc(con->buffer_size);
      printf("alloc buffer\n");
      if (!con->buffer) exit(1);
    }
    con->buffer[0] = 0;
    if (con->logger) dlog_log_text("rc",con->logger);
    rva = con_recv(con,(void**) &con->buffer,con->buffer_size);
    if (con->logger) dlog_log_text("rc_",con->logger);

    if (rva < 0 ){
      con->status = CON_ERROR_UNKNOWN;
      return CON_ERROR_UNKNOWN;
    } else if (rva == 0){
      printf("closed\n");
      con->status = CON_ERROR_CLOSED;
      *target = buffer;
      return CON_ERROR_NONE;
    }
    con->buffer_filled_to = rva;
    if (con->logger) dlog_log_text("nw",con->logger); 
  }
}

int con_is_ok(con_t con){
  if (! con) return 0;
  if (! (con->status == CON_ERROR_NONE)) return 0;
  if (! con->connected) return 0;


  return 1;
}

int con_close(con_t con){
  if (!(con) ) return CON_ERROR_MEMORY;
  
  if ( con->connected ) close(con->sockfd);
  if (con->logger) dlog_log_text("Closing",con->logger);
  if (con->raw_in) dlog_close_log(con->raw_in);
  if (con->raw_out) dlog_close_log(con->raw_out);
  if (con->raw_out) dlog_close_log(con->logger);
  if (con->buffer) free(con->buffer);
  if (con->host) free(con->host);
  if (con->port) free(con->port);
  
  free(con);
  
  return CON_ERROR_NONE;
}

