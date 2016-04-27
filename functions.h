#ifndef _US_FUNCTIONS_H
# define _US_FUNCTIONS_H

# include <sys/types.h>
# include "userv.h"

int AddListen(char *);
int AddClient(char *);
int AddUser(char *);
int AddConfig(char *);
int AddPeer(char *);
void CleanUp(int);
int StartListening(void);
int valid_user(char *);
int valid_ip(char *);
char *trim(char *, char *);
int ip_allowed(char *);
int MainLoop(void);
int ParseConfig(void);
int pwcheck(char *, char *);
int IsNum(char *);
int Is_UID_exist(uid_t);
void salt(char *);
void tempname(char *, size_t);
int user_allowed(char *);
void Log(int, char *, ...);
void SetSignals(void);
void Send(int, char *, ...);
void DelConn(int);
# ifdef CONTEXT_DEBUG
void n_Context(char *, int);
void n_PrintContext(void);
# endif

#endif /* _US_FUNCTIONS_H */
