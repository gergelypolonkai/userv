/*
 * UserServer/0.1
 * networking.c - Handling network connections
 *
 * Copyright (C) 2002 Gergely Polonkai
 * Copyright (C) 2002-2006 P-Team Programmers' Group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have recieved a copy of the GNU General Public License
 * along with this program; if not, write to Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA, 02111-1307, USA.
 *
 * GNU GPL is in doc/COPYING
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <crypt.h>
#include <sys/wait.h>
#include <stdarg.h>

#include "userv.h"
#include "functions.h"

fd_set master, readable;
int maxfd = 0;

int
AddConn(int type, int sfd, char *IP) {
  char *ip;

  if ((ip = strdup(IP)) == NULL)
    return ERR_NOMEM;

  if (nconn) {
    NST *temp = nconn;
    while (temp->next)
      temp = temp->next;
    if ((temp->next = malloc(NSTS)) == NULL) {
      free(ip);
      return ERR_NOMEM;
    }
    temp->next->next = NULL;
    temp->next->IP = ip;
    temp->next->type = type;
    temp->next->sfd = sfd;
    temp->next->timeout = 0;
    temp->next->loggedin = 0;
  } else {
    if ((nconn = malloc(NSTS)) == NULL) {
      free(ip);
      return ERR_NOMEM;
    }
    nconn->next = NULL;
    nconn->IP = ip;
    nconn->type = type;
    nconn->sfd = sfd;
    nconn->timeout = 0;
    nconn->loggedin = 0;
  }
  FD_SET(sfd, &master);
  if (sfd > maxfd)
    maxfd = sfd;
  return ERR_OK;
}

void
DelConn(int sfd) {
  NST *temp = nconn;
  NST *p = NULL;
  NST *n;

  while (temp) {
    n = temp->next;
    if (temp->sfd == sfd) {
      if (temp->IP)
        free(temp->IP);
      if (sfd != -1)
        close(sfd);
      if (p)
        p->next = temp->next;
      else
        nconn = n;
      free(temp);
      FD_CLR(sfd, &master);
      return;
    }
    p = temp;
    temp = temp->next;
  }
}

int
Type(int sfd) {
  NST *temp = nconn;
  while (temp) {
    if (temp->sfd == sfd)
      return temp->type;
    temp = temp->next;
  }
  return CT_NONE;
}

int
LoggedIn(int sfd) {
  NST *temp = nconn;
  while (temp) {
    if (temp->sfd == sfd)
      return temp->loggedin;
    temp = temp->next;
  }
  return 0;
}

int
SetLogin(int sfd, char *uname) {
  if (uname) {
    char *un;
    NST *temp = nconn;

    if ((un = strdup(uname)) == NULL)
      return ERR_NOMEM;
    else {
      while (temp) {
        if (temp->sfd == sfd) {
          temp->loggedin = 1;
          temp->uname = un;
          return ERR_OK;
        }
        temp = temp->next;
      }
    }
  }

  return ERR_NOUSER;
}

char *
GetIP(int sfd) {
  NST *temp = nconn;

  while (temp) {
    if (temp->sfd == sfd)
      return temp->IP;
    temp = temp->next;
  }
  return NULL;
}

char *
GetUser(int sfd) {
  NST *temp = nconn;

  while (temp) {
    if (temp->sfd == sfd)
      return temp->uname;
    temp = temp->next;
  }
  return NULL;
}

void
IncTimeout(int sfd) {
  NST *temp = nconn;
  while (temp) {
    if (temp->sfd == sfd) {
      temp->timeout++;
      return;
    }
    if (temp->timeout == 600) {
      Log(MT_MSG, "Timeout at %s", GetIP(sfd));
      DelConn(temp->sfd);
      return;
    }
    temp = temp->next;
  }
}

int
StartListening(void) {
  {
    LST *temp;

    FD_ZERO(&master);
    FD_ZERO(&readable);

    temp = laddr;
    while (temp) if (temp->IP) {
      int sfd;
      struct sockaddr_in la;
      int yes = 1;

      if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return ERR_SOCKET;
      if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        return ERR_OPT;

      la.sin_family = AF_INET;
      la.sin_addr.s_addr = inet_addr(temp->IP);
      la.sin_port = htons(8778);
      memset(&(la.sin_zero), 0, 8);

      if (bind(sfd, (struct sockaddr *)&la, sizeof(la)) == -1)
        return ERR_BIND;

      if (listen(sfd, 10) == -1)
        return ERR_LISTEN;

      if (AddConn(CT_LISTEN, sfd, temp->IP) != ERR_OK)
        return ERR_NOMEM;
    
      temp = temp->next;
    }
  }

  {
    SST *temp;

    temp = saddr;
    while (temp) if (temp->IP) {
      int sfd;
      struct sockaddr_in la;
      int yes = 1;

      if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return ERR_SOCKET;
      if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        return ERR_OPT;

      la.sin_family = AF_INET;
      la.sin_addr.s_addr = inet_addr(temp->IP);
      la.sin_port = htons(8777);
      memset(&(la.sin_zero), 0, 8);

      if (bind(sfd, (struct sockaddr *)&la, sizeof(la)) == -1)
        return ERR_BIND;

      if (listen(sfd, 10) == -1)
        return ERR_LISTEN;

      if (AddConn(CT_CONFIG, sfd, temp->IP) != ERR_OK)
        return ERR_NOMEM;
    
      temp = temp->next;
    }
  }
  
  return ERR_OK;
}

void
Send(int sfd, char *msg, ...) {
  va_list ap;

  va_start(ap, msg);
  vdprintf(sfd, msg, ap);
  va_end(ap);
}

int
MainLoop(void) {
  struct timeval tv;

  while (1) {
    int t;
    readable = master;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    t = select(maxfd + 1, &readable, NULL, NULL, &tv);
    if (t != -1) {
      int i;

      for (i = 0; i <= maxfd; i++) {
        if (FD_ISSET(i, &readable)) {
          if (Type(i) == CT_LISTEN) {
            struct sockaddr_in ra;
            int al = sizeof(ra);
            int nfd;

            if ((nfd = accept(i, (struct sockaddr *)&ra, &al)) != -1) {
              if (ip_allowed(inet_ntoa(ra.sin_addr))) {
                AddConn(CT_CLIENT, nfd, inet_ntoa(ra.sin_addr));
                Log(MT_MSG, "Connection from allowed IP %s", inet_ntoa(ra.sin_addr));
                Send(nfd, "R\n");
              } else {
                Send(nfd, "D\n");
                Log(MT_MSG, "Attempted connection from denied IP %s", inet_ntoa(ra.sin_addr));
                close(nfd);
              }
            }
          }
          if (Type(i) == CT_CONFIG) {
            struct sockaddr_in ra;
            int al = sizeof(ra);
            int nfd;

            if ((nfd = accept(i, (struct sockaddr *)&ra, &al)) != -1) {
              if (ip_allowed(inet_ntoa(ra.sin_addr))) {
                PST *temp = paddr->next;
                Send(nfd, "P %s\n", paddr->IP);
                while (temp) {
                  Send(nfd, "P %s\n", temp->IP);
                  temp = temp->next;
                }
                Send(nfd, "S\n");
                close(nfd);
                Log(MT_MSG, "Sent configuration to %s", inet_ntoa(ra.sin_addr));
              } else {
                Send(nfd, "D\n");
                Log(MT_MSG, "Attempted connection from denied IP %s", inet_ntoa(ra.sin_addr));
                close(nfd);
              }
            }
          }
          if (Type(i) == CT_CLIENT) {
            int r;
            char buf[1024];

            memset(&buf, 0, 1024);
            r = recv(i, &buf, 1023, 0);
            if (r == 0) {
              Log(MT_MSG, "Connection closed from %s", GetIP(i));
              DelConn(i);
            }
            if (r > 0) {
              char *line = trim(buf, NULL);
              if (line) {
                if (strncmp(line, "L ", 2) == 0) {
                  if (LoggedIn(i))
                    Send(i, "L\n");
                  else {
                    char *uname, *passw;
                    int start;

                    for (start = 1; start < strlen(line); start++)
                      if (isspace(*(line + start)))
                        break;
                    uname = trim(line + start, NULL);
                  
                    for (start = 1; start < strlen(uname); start++)
                      if (isspace(*(uname + start)))
                        break;
                  
                    passw = trim(uname + start, NULL);
                  
                    for (start = 0; start < strlen(uname); start++)
                      if (isspace(*(uname + start)))
                        *(uname + start) = 0;

                    if (!uname || !passw)
                      Send(i, "E\n");
                    else {
                      if (!user_allowed(uname)) {
                        Log(MT_MSG, "Attempted login by denied user %s from %s", uname, GetIP(i));
                        Send(i, "D\n");
                      } else {
                        if (pwcheck(uname, passw) != ERR_OK) {
                          Log(MT_MSG, "Login with bad password by %s from %s", uname, GetIP(i));
                          Send(i, "B\n");
                        } else {
                          if (SetLogin(i, uname) == ERR_OK) {
                            Log(MT_MSG, "Successful login by %s at %s", uname, GetIP(i));
                            Send(i, "S\n");
                          } else {
                            Log(MT_MSG, "Failed to log in user %s from %s", uname, GetIP(i));
                            Send(i, "F\n");
                          }
                        }
                      }
                    }
                  }
                }
                if (strcmp(line, "Q") == 0) {
                  Send(i, "S\n");
                  Log(MT_MSG, "Quit from %s, by %s", GetIP(i), GetUser(i));
                  DelConn(i);
                }
                if (strncmp(line, "U ", 2) == 0) {
                  if (!LoggedIn(i))
                    Send(i, "L\n");
                  else {
                    int start;
                    char *param;

                    for (start = 2; start < strlen(line); start++)
                      if (!isspace(*(line + start)))
                        break;
                    param = line + start;
                    for (start = 0; start < strlen(param); start++)
                      if (isspace(*(param + start)))
                        break;
                    *(param + start) = 0;
                    if (!IsNum(param))
                      Send(i, "E\n");
                    else {
                      uid_t uid = (uid_t)atoi(param);
                      if (Is_UID_exist(uid))
                        Send(i, "U\n");
                      else
                        Send(i, "F\n");
                    }
                  }
                }
                if (strncmp(line, "N ", 2) == 0) {
                  if (!LoggedIn(i))
                    Send(i, "L\n");
                  else {
                    int start;
                    char *param;

                    for (start = 2; start < strlen(line); start++)
                      if (!isspace(*(line + start)))
                        break;
                    param = line + start;
                    for (start = 0; start < strlen(param); start++)
                      if (isspace(*(param + start)))
                        break;
                    *(param + start) = 0;
                    if (valid_user(param))
                      Send(i, "U\n");
                    else
                      Send(i, "F\n");
                  }
                }
                if (strncmp(line, "A ", 2) == 0) {
                  if (!LoggedIn(i))
                    Send(i, "L\n");
                  else {
                    int start;
                    char *param;

                    for (start = 2; start < strlen(line); start++)
                      if (!isspace(*(line + start)))
                        break;
                    param = trim(line + start, NULL);
                    if (param) {
                      char *uname, *c_uid, *tanar, *samba, *passwd, *comment;

                      uname = param;
                      c_uid = strchr(uname, ':') + 1;
                      if (c_uid - 1 == NULL)
                        Send(i, "E\n");
                      else {
                        *(strchr(uname, ':')) = 0;
                        tanar = strchr(c_uid, ':') + 1;
                        if (tanar - 1 == NULL)
                          Send(i, "E\n");
                        else {
                          *(strchr(c_uid, ':')) = 0;
                          samba = strchr(tanar, ':') + 1;
                          if (samba - 1 == NULL)
                            Send(i, "E\n");
                          else {
                            *(strchr(tanar, ':')) = 0;
                            passwd = strchr(samba, ':') + 1;
                            if (passwd - 1 == NULL)
                              Send(i, "E\n");
                            else {
                              *(strchr(samba, ':')) = 0;
                              comment = strchr(passwd, ':') + 1;
                              if (comment - 1 == NULL)
                                Send(i, "E\n");
                              else {
                                *(strchr(passwd, ':')) = 0;
                                if (!IsNum(c_uid))
                                  Send(i, "E\n");
                                else {
                                  uid_t uid;

                                  uid = (uid_t)atoi(c_uid);
                                  if (Is_UID_exist(uid) || valid_user(uname))
                                    Send(i, "U\n");
                                  else {
                                    if ((strcmp(tanar, "0") != 0) && (strcmp(tanar, "1") != 0) && (strcmp(samba, "0") != 0) && (strcmp(samba, "1") != 0))
                                      Send(i, "E\n");
                                    else {
                                      char s[13], *cpass, tempfilename[13], scriptfilename[13];
                                      FILE *tf, *sf;

                                      if (strcmp(c_uid, "0") == 0) {
                                        Log(MT_MSG, "Trying to add user with uid 0 from %s by %s", GetIP(i), GetUser(i));
                                        Send(i, "F\n");
                                      } else {
                                        salt((char *)&s);
                                        cpass = crypt(passwd, s);
                                        tempname((char *)&tempfilename, 12);
                                        tempname((char *)&scriptfilename, 12);

                                        if ((tf = fopen(tempfilename, "w")) == NULL) {
                                          Log(MT_ERROR, "Unable to create tempfile (by %s at %s)", GetUser(i), GetIP(i));
                                          Send(i, "F\n");
                                        } else {
                                          fprintf(tf, "%s\n%s\n", passwd, passwd);
                                          fclose(tf);

                                          if ((sf = fopen(scriptfilename, "w")) == NULL) {
                                            Log(MT_ERROR, "Unabke to create scriptfile (by %s at %s)", GetUser(i), GetIP(i));
                                            Send(i, "F\n");
                                          } else {
                                            pid_t pid;

                                            fprintf(sf, "/usr/sbin/useradd -u %s -g %s -s %s %s-c \"%s\" -p %s %s > /dev/null 2>&1\n", c_uid, (*tanar == '1') ? "tanar" : "tanulo", (*samba == '1') ? "/bin/bash" : "/bin/false", (*samba == '1') ? "-m " : "", comment, cpass, uname);
                                            fprintf(sf, "r1=$?\n");
                                            if (*samba == '1') {
                                              fprintf(sf, "/usr/bin/smbpasswd -a -s %s < %s > /dev/null 2>&1\n", uname, tempfilename);
                                              fprintf(sf, "r2=$?\n");
                                              fprintf(sf, "/usr/bin/smbpasswd -e %s > /dev/null 2>&1\n", uname);
                                              fprintf(sf, "r3=$?\n");
                                              fprintf(sf, "if [ $r1 != 0 -o $r2 != 0 -o $r3 != 0 ]; then\n");
                                            } else
                                              fprintf(sf, "if [ $r1 != 0 ]; then\n");
                                            fprintf(sf, "  exit 1\n");
                                            fprintf(sf, "else\n");
                                            fprintf(sf, "  exit 0\n");
                                            fprintf(sf, "fi\n");
                                            fclose(sf);

                                            if ((pid = fork()) == 0)
                                              execl("/bin/sh", "sh", scriptfilename);
                                            else {
                                              int ec;

                                              waitpid(pid, &ec, 0);
                                              if (ec == 0) {
                                                Log(MT_MSG, "Added user %s with uid %s from %s by %s", uname, c_uid, GetIP(i), GetUser(i));
                                                Send(i, "S\n");
                                              } else {
                                                Log(MT_MSG, "Failed to add user %s with uid %s from %s bye %s", uname, c_uid, GetIP(i), GetUser(i));
                                                Send(i, "F\n");
                                              }

                                            unlink(scriptfilename);
                                            }
                                          }
                                          unlink(tempfilename);
                                        }
                                      }
                                    }
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
                if (strncmp(line, "D ", 2) == 0) {
                  if (!LoggedIn(i))
                    Send(i, "L\n");
                  else {
                    int start;
                    char *uname;
                    
                    for (start = 2; start < strlen(line); start++)
                      if (!isspace(*(line + start)))
                        break;
                    
                    uname = line + start;
                    for (start = 0; start < strlen(uname); start++)
                      if (isspace(*(uname + start))) {
                        *(uname + start) = 0;
                        break;
                      }

                    if (!valid_user(uname))
                      Send(i, "E\n");
                    else {
                      FILE *sf;
                      char scriptfilename[13];

                      tempname((char *)&scriptfilename, 12);
                      if ((sf = fopen(scriptfilename, "w")) == NULL)
                        Send(i, "F\n");
                      else {
                        pid_t pid;
                        fprintf(sf, "userdel -r %s > /dev/null 2>&1\n", uname);
                        fprintf(sf, "r=$?\n");
                        fprintf(sf, "if [ $r = 12 -o $r = 0 ]; then\n");
                        fprintf(sf, "  r1=0\n");
                        fprintf(sf, "else\n");
                        fprintf(sf, "  r1=1\n");
                        fprintf(sf, "fi\n");
                        fprintf(sf, "smbpasswd -x %s > /dev/null 2>&1\n", uname);
                        fprintf(sf, "r2=$?\n");
                        fprintf(sf, "if [ $r1 != 0 -o $r2 != 0 ]; then\n");
                        fprintf(sf, "  exit 1\n");
                        fprintf(sf, "else\n");
                        fprintf(sf, "  exit 0\n");
                        fprintf(sf, "fi\n");
                        fclose(sf);
                        
                        if ((pid = fork()) == 0) { //child
                          execl("/bin/sh", "sh", scriptfilename);
                        } else { //parent
                          int ec;
                          waitpid(pid, &ec, 0);
                          if (ec == 0) {
                            Log(MT_MSG, "Deleted user %s from %s by %s", uname, GetIP(i), GetUser(i));
                            Send(i, "S\n");
                          } else {
                            Log(MT_MSG, "Failed to delete %s from %s by %s", uname, GetIP(i), GetUser(i));
                            Send(i, "F\n");
                          }
                          unlink(scriptfilename);
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        } else
          if (Type(i) == CT_CLIENT)
            IncTimeout(i);
      }
    }
  }
  
  return ERR_OK;
}

