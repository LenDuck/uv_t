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
  con_t client = NULL;
  dlog_t *mainlog = NULL;
  
  mainlog = dlog_newlog_file("Server log", "server.log", DLOG_FLAG_FFLUSH  |DLOG_FLAG_NLINE);

  /*dlog_log_text("?",mainlog);*/
  dlog_log_text("Bootup start",mainlog);
  con = con_bootup(hostname,port);
  if (!con){
    dlog_log_text("con fail",mainlog);
    return -1;
  }
  if (con_serve(con,&client)){
    dlog_log_text("serve fail",mainlog);
    return -1;
  }
  if (client){
    con_send_line(client,"Phasers do not include batteries,\nPhaser don't even include tangibility\n");
  }
    int num = 0;

  while (client){
    char *in = NULL;
    int rva;
    char buffer[128];
    num++;
    sprintf(buffer,"P:%d\n",num);
    rva = con_send_line(client,buffer);
    printf("%d>%s\n",rva,buffer);
    if (rva) break;
  }


  dlog_log_text("Main done, closing log",mainlog);
  dlog_close_log(mainlog);
  mainlog = NULL;
  fprintf(stderr,"done\n");

  (void)argc;
  (void)argv;

  return 0;
}

