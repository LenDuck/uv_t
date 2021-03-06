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

#ifndef H_CON
#define H_CON

/*These are the integers most of these functions return*/
#define CON_ERROR_NONE (0)
#define CON_ERROR_CLOSED (1)
#define CON_ERROR_MEMORY (2)
#define CON_ERROR_UNKNOWN (3)
#define CON_ERROR_INCOMPLETE (4)


typedef struct struct_con* con_t;

int con_is_ok(con_t x);

/*Constructor*/
con_t con_bootup(char *h,char *p);

/*Start being a client*/
int con_connect(con_t con);

/*Start serving*/
int con_serve(con_t con,con_t *newcon);
int con_init_serve(con_t con);
int con_serve_accept(con_t con, con_t *newcon);

/*Send data, or a line*/
int con_send(con_t con, void *data,int size);
int con_send_line(con_t con, char *data);

/* Receive:
  If *target == NULL, auto alocates (a block of *size big)
  If size == NULL, picks a size around 2K big (way too big for most packets)
*/
int con_recv(con_t con, void **target,int size);

/*Sets *target to a valid line, if one ever is received */
int con_line(con_t con, char **target);

/*Close and deallocate*/
int con_close(con_t con);

#endif
