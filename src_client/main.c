#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include "con.h"
#include "dlogger.h"

int main(int argc, char **argv) {
  char hostname[] = "localhost";
  char port[] = "55555";

  con_t con = NULL;
//  con_t client = NULL;
  dlog_t *mainlog = NULL;
  
  mainlog = dlog_newlog_file("Client log","client.log", DLOG_FLAG_FFLUSH  |DLOG_FLAG_NLINE);

  /*dlog_log_text("?",mainlog);*/
  dlog_log_text("Bootup start",mainlog);
  con = con_bootup(hostname,port);
  if (!con){
    dlog_log_text("con fail",mainlog);
    return -1;
  }
  if (con_connect(con)){
    dlog_log_text("connect fail",mainlog);
    return -1;
  }
  if (con){
    con_send_line(con,"Ohai There");
  }
    int num = 0;

  while (con){
    int rva;
    char *buffer = NULL;
    num++;
    rva = con_line(con,&buffer);
    printf("Got_%d,%d: %s\n",num,rva,buffer);
    if (rva < 1) break;
  }
  con_close(con);

  dlog_log_text("Main done, closing log",mainlog);
  dlog_close_log(mainlog);
  mainlog = NULL;
  fprintf(stderr,"done\n");

  (void)argc;
  (void)argv;

  return 0;
}

