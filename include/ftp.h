#pragma once

/*! Loop status */
typedef enum
{
  LOOP_CONTINUE, /*!< Continue looping */
  LOOP_RESTART,  /*!< Reinitialize */
  LOOP_EXIT,     /*!< Terminate looping */
} loop_status_t;

int           ftp_init(void);
loop_status_t ftp_loop(void);
void          ftp_exit(void);
int simple_recv(int s, char * buffer);
int simple_recv_code(int s, char * buffer);
int simple_send(int s, char * cmd, int cmd_len);
int pasv_create_port(int sock, char * b);
int pasv_do_cmd(int sock, int pasv_port, char * b, char * cmd, int cmd_len);
int ftpc_ch_cmd();
int ftpc_do_cmd();
