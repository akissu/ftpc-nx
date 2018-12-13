#include <errno.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <list>
#include <cstring>
#ifdef _3DS
#include <3ds.h>
#elif defined(__SWITCH__)
#include <switch.h>
#endif
#include "console.h"
#include "ftp.h"
#include "ftplib.h"

#define SRV_ADR "64.50.236.52:21"

static std::list<char *> dir_lst;
static int dir_lst_idx;
char * dir_lst_get()
{
	int i = 0;
  for (char * d : dir_lst) 
	{
		if(i == dir_lst_idx)
			return d;
		i++;
  }
}

static ftplib *ftp = NULL;
#define FTP_DECLARE(x) static void x(ftplib *ftp, const char *args)
FTP_DECLARE(CWD)
{
		char * d = dir_lst_get();
		if(d == NULL)
			d = ".";
    if(ftp->Chdir(d) != 1)
    {
      console_print(RED "CWD Failed%d\n" RESET, 0x2);
    }
		if(ftp->Nlst(NULL, ".", &dir_lst) != 1)
		{
			console_print(RED "Nlst Failed%d\n" RESET, 0x2);
		}
}
FTP_DECLARE(CDUP)
{
    if(ftp->Cdup() != 1)
    {
      console_print(RED "CDUP Failed%d\n" RESET, 0x2);
    }
		if(ftp->Nlst(NULL, ".", &dir_lst) != 1)
		{
			console_print(RED "Nlst Failed%d\n" RESET, 0x2);
		}
}
FTP_DECLARE(HELP)
{
	console_print(RED "The following commands are recognized\r\n"
	" ABOR ALLO APPE CDUP CWD DELE FEAT HELP LIST MDTM MKD MLSD MLST MODE\r\n"
	" NLST NOOP OPTS PASS PASV PORT PWD QUIT REST RETR RMD RNFR RNTO STAT\r\n"
	" STOR STOU STRU SYST TYPE USER XCUP XCWD XMKD XPWD XRMD\r\n"
	"214 End\r\n" RESET);
}
FTP_DECLARE(LIST)
{
		char * d = dir_lst_get();
		if(d == NULL)
			d = ".";
    if(ftp->Dir(NULL, d) != 1)
    {
      console_print(RED "LIST Failed%d\n" RESET, 0x2);
    }
}
FTP_DECLARE(NLST)
{
		char * d = dir_lst_get();
		if(d == NULL)
			d = ".";
	if(ftp->Nlst(NULL, d, &dir_lst) != 1)
	{
		console_print(RED "Nlst Failed%d\n" RESET, 0x2);
	}
}
FTP_DECLARE(GET)
{
		char * d = dir_lst_get();
		char * p = new char[strlen(d) + 1];
		
		if(d == NULL || d == "." || d == "..")
			return -1;

		strcpy(p + 1, d);    
		p[0] = '/';

	if(ftp->Get(p, d, ftplib::image, 0x0) != 1)
	{
		console_print(RED "GET Failed%d\n" RESET, 0x2);
	}
}
/*! ftp command descriptor */
typedef struct ftp_command
{
  const char *name;                                   /*!< command name */
  void       (*handler)(ftplib *, const char*); 				/*!< command callback */
} ftp_command_t;
/*! ftp command */
#define FTP_COMMAND(x) { #x, x, }
/*! ftp alias */
#define FTP_ALIAS(x,y) { #x, y, }
static ftp_command_t ftpc_commands[] =
{
  FTP_COMMAND(CWD),
  FTP_COMMAND(CDUP),
  FTP_COMMAND(HELP),
  FTP_COMMAND(LIST),
  FTP_COMMAND(NLST),
  FTP_COMMAND(GET),
};
static const size_t num_ftpc_commands = 6;
static int cur_ftpc_command = 0;

/*! looping mechanism
 *
 *  @param[in] callback function to call during each iteration
 *
 *  @returns loop status
 */
static loop_status_t
loop(loop_status_t (*callback)(void))
{
  loop_status_t status = LOOP_CONTINUE;

#ifdef _3DS
  while(aptMainLoop())
  {
    status = callback();
    console_render();
    if(status != LOOP_CONTINUE)
      return status;
  }
  return LOOP_EXIT;
#elif defined(__SWITCH__)
  while(appletMainLoop())
  {
    status = callback();
    console_render();
    if(status != LOOP_CONTINUE)
      return status;
  }
  return LOOP_EXIT;
#else
  while(status == LOOP_CONTINUE)
    status = callback();
  return status;
#endif
}

#ifdef _3DS
/*! wait until the B button is pressed
 *
 *  @returns loop status
 */
static loop_status_t
wait_for_b(void)
{
  /* update button state */
  hidScanInput();

  /* check if B was pressed */
  if(hidKeysDown() & KEY_B)
    return LOOP_EXIT;

  /* B was not pressed */
  return LOOP_CONTINUE;
}
#elif defined(__SWITCH__)
/*! wait until the B button is pressed
 *
 *  @returns loop status
 */
static loop_status_t
wait_for_b(void)
{
  /* update button state */
  hidScanInput();

  /* check if B was pressed */
  if(hidKeysDown(CONTROLLER_P1_AUTO) & KEY_B)
    return LOOP_EXIT;

  /* B was not pressed */
  if(hidKeysDown(CONTROLLER_P1_AUTO) & KEY_X)
  {
    ftp = new ftplib();
    if(!ftp->Connect(SRV_ADR))
    {
      console_print(RED "Connect Failed%d\n" RESET, 0x2);
      return LOOP_CONTINUE;
    }
    if(ftp->Login("anonymous", "pass") != 1)
    {
      console_print(RED "Login Failed%d\n" RESET, 0x2);
      return LOOP_CONTINUE;
    }
		if(ftp->Nlst(NULL, ".", &dir_lst) != 1)
		{
			console_print(RED "Nlst Failed%d\n" RESET, 0x2);
		}
  }
  if(hidKeysDown(CONTROLLER_P1_AUTO) & KEY_A)
  {
    //consoleClear();
		ftpc_commands[cur_ftpc_command].handler(ftp, NULL);
  }
  if(hidKeysDown(CONTROLLER_P1_AUTO) & KEY_DUP)
  {
		cur_ftpc_command = (cur_ftpc_command + 1) % num_ftpc_commands;
		console_set_status(WHITE "%s\t\t| %s\n" RESET, ftpc_commands[cur_ftpc_command].name, dir_lst_get());
  }
  if(hidKeysDown(CONTROLLER_P1_AUTO) & KEY_DDOWN)
  {
		cur_ftpc_command = (cur_ftpc_command - 1) % num_ftpc_commands;
		console_set_status(WHITE "%s\t\t| %s\n" RESET, ftpc_commands[cur_ftpc_command].name, dir_lst_get());
  }
  if(hidKeysDown(CONTROLLER_P1_AUTO) & KEY_DRIGHT)
  {
		dir_lst_idx = (dir_lst_idx + 1) % dir_lst.size();
		console_set_status(WHITE "%s\t\t| %s\n" RESET, ftpc_commands[cur_ftpc_command].name, dir_lst_get());
  }
  if(hidKeysDown(CONTROLLER_P1_AUTO) & KEY_DLEFT)
  {
		dir_lst_idx = (dir_lst_idx - 1) % dir_lst.size();
		console_set_status(WHITE "%s\t\t| %s\n" RESET, ftpc_commands[cur_ftpc_command].name, dir_lst_get());
  }

  return LOOP_CONTINUE;
}
#endif

/*! entry point
 *
 *  @param[in] argc unused
 *  @param[in] argv unused
 *
 *  returns exit status
 */
int
main(int  argc,
     char *argv[])
{
  loop_status_t status = LOOP_RESTART;

#ifdef _3DS
  /* initialize needed 3DS services */
  acInit();
  gfxInitDefault();
  gfxSet3D(false);
  sdmcWriteSafe(false);
  /* initialize needed Switch services */
#elif defined(__SWITCH__)
  //gfxInitResolution(644, 480);
  nifmInitialize();
  gfxInitDefault();
#endif

  /* initialize console subsystem */
  console_init();

#ifdef ENABLE_LOGGING
  /* open log file */
#ifdef _3DS
  FILE *fp = freopen("/ftpd.log", "wb", stderr);
#else
  FILE *fp = freopen("ftpd.log", "wb", stderr);
#endif
  if(fp == NULL)
  {
    console_print(RED "freopen: 0x%08X\n" RESET, errno);
    goto log_fail;
  }

  /* truncate log file */
  if(ftruncate(fileno(fp), 0) != 0)
  {
    console_print(RED "ftruncate: 0x%08X\n" RESET, errno);
    goto log_fail;
  }
#endif

  console_set_status("\n" GREEN STATUS_STRING
#ifdef ENABLE_LOGGING
                     " DEBUG"
#endif
                     RESET);

  while(status == LOOP_RESTART)
  {
    /* initialize ftp subsystem */
    if(ftp_init() == 0)
    {
      /* ftp loop */
      status = loop(ftp_loop);

      /* done with ftp */
      ftp_exit();
    }
    else
      status = LOOP_EXIT;
  }

#if defined(_3DS) || defined(__SWITCH__)
  console_print("Press B to exit\n");
#endif

#ifdef ENABLE_LOGGING
log_fail:
  if(fclose(stderr) != 0)
    console_print(RED "fclose(%d): 0x%08X\n" RESET, fileno(stderr), errno);
#endif

#ifdef _3DS
  loop(wait_for_b);

  /* deinitialize 3DS services */
  gfxExit();
  acExit();
#elif defined(__SWITCH__)
  loop(wait_for_b);

  /* deinitialize Switch services */
  gfxExit();
  nifmExit();
#endif
  return 0;
}
