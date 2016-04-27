#ifndef US_USERV_H
# define US_USERV_H

# define CONTEXT_DEBUG
# ifdef CONTEXT_DEBUG
#  define Context n_Context(__FILE__, __LINE__)
#  define PrintContext n_PrintContext()
# else
#  define Context
#  define PrintContext
# endif

# define ERR_OK 0
# define ERR_NOUSER -1
# define ERR_NOSHADOW -2
# define ERR_NOPASS -3
# define ERR_BADPASS -4
# define ERR_NOCF -5
# define ERR_NOMEM -6
# define ERR_NOLISTEN -7
# define ERR_SOCKET -8
# define ERR_OPT -9
# define ERR_BIND -10
# define ERR_LISTEN -11

# define CT_NONE 0
# define CT_LISTEN 1
# define CT_CLIENT 2
# define CT_CONFIG 3

# define MT_NONE 0
# define MT_ERROR 1
# define MT_MSG 2

typedef struct L_Struct {
  char *IP;
  struct L_Struct *next;
} LST;
# define LSTS sizeof(LST)

typedef struct S_Struct {
  char *IP;
  struct S_Struct *next;
} SST;
# define SSTS sizeof(SST)

typedef struct C_Struct {
  char *IP;
  struct C_Struct *next;
} CST;
# define CSTS sizeof(CST)

typedef struct P_Struct {
  char *IP;
  struct P_Struct *next;
} PST;
# define PSTS sizeof(PST)

typedef struct U_Struct {
  char *uname;
  struct U_Struct *next;
} UST;
# define USTS sizeof(UST)

typedef struct N_Struct {
  int type;
  char *IP;
  int sfd;
  unsigned long timeout;
  int loggedin;
  char *uname;
  struct N_Struct *next;
} NST;
# define NSTS sizeof(NST)

# ifndef USERV_MAIN
extern LST *laddr;
extern CST *caddr;
extern UST *auser;
extern NST *nconn;
extern SST *saddr;
extern PST *paddr;
# endif

#endif

