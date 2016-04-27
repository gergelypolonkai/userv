/*
 * UserServer/0.1
 * common.c - Common functions
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

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <pwd.h>
#include <crypt.h>

#include "userv.h"
#include "functions.h"

char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

int
ParseConfig(void) {
  FILE *cf;
  char buf[1024];
  char *line;
  int nl = 0;

  if ((cf = fopen("/etc/userv.conf", "r")) == NULL)
    return ERR_NOCF;

  memset(&buf, 0, 1024);
  while (fgets((char *)&buf, 1023, cf)) {
    char *param;
    int i;

    nl++;
    if ((line = trim(buf, NULL))) {
      if (strncmp(line, "listen ", 7) == 0) {
        param = strchr(line, ' ') + 1;
        for (i = 0; i < strlen(param); i++)
          if (isspace(*(param + i))) {
            *(param + i) = 0;
            break;
          }
        if (!valid_ip(param))
          return nl;
        if (AddListen(param) != ERR_OK)
          return ERR_NOMEM;
      }
      if (strncmp(line, "confserv ", 9) == 0) {
        param = strchr(line, ' ') + 1;
        for (i = 0; i < strlen(param); i++)
          if (isspace(*(param + 1))) {
            *(param + i) = 0;
            break;
          }
        if (!valid_ip(param))
          return nl;
        if (AddConfig(param) != ERR_OK)
          return ERR_NOMEM;
      }
      if (strncmp(line, "server ", 7) == 0) {
        param = strchr(line, ' ') + 1;
        for (i = 0; i < strlen(param); i++)
          if (isspace(*(param + 1))) {
            *(param + i) = 0;
            break;
          }
        if (!valid_ip(param))
          return nl;
        if (AddPeer(param) != ERR_OK)
          return ERR_NOMEM;
      }
      if (strncmp(line, "client ", 7) == 0) {
        param = strchr(line, ' ') + 1;
        for (i = 0; i < strlen(param); i++)
          if (isspace(*(param + i))) {
            *(param + i) = 0;
            break;
          }
        if (!valid_ip(param))
          return nl;
        if (AddClient(param) != ERR_OK)
          return ERR_NOMEM;
      }
      if (strncmp(line, "user ", 5) == 0) {
        param = strchr(line, ' ') + 1;
        for (i = 0; i < strlen(param); i++)
          if (isspace(*(param + i))) {
            *(param + i) = 0;
            break;
          }
        if (!valid_user(param))
          return nl;
        if (AddUser(param) != ERR_OK)
          return ERR_NOMEM;
      }
      free(line);
    }
    memset(buf, 0, 1024);
  }
  fclose(cf);

  return ERR_OK;
}

int
Is_UID_exist(uid_t uid) {
  if (getpwuid(uid) == NULL)
    return 0;
  return 1;
}

int
pwcheck(char *uname, char *passw) {
  struct passwd *pwentry;
  char buf[1024];
  FILE *fd;
  char *pwd;

  if ((pwentry = getpwnam(uname)) == NULL)
    return ERR_NOUSER;
  
  if (strcmp(pwentry->pw_passwd, "x") == 0) {
    if ((fd = fopen("/etc/shadow", "r")) == NULL)
      return ERR_NOSHADOW;

    memset(&buf, 0, 1024);
    while (fgets((char *)&buf, 1023, fd)) {
      if (strncmp((char *)&buf, uname, strlen(uname)) == 0)
        break;
      memset(&buf, 0, 1024);
    }
    fclose(fd);
  } else
    strcpy((char *)&buf, pwentry->pw_passwd);

  pwd = strchr((char *)&buf, ':') + 1;
  if (pwd - 1 == NULL)
    return ERR_NOPASS;

  *(strchr(pwd, ':')) = 0;

  if (strcmp(crypt(passw, pwd), pwd) == 0)
    return ERR_OK;
  
  return ERR_BADPASS;
}

void
salt(char *where) {
  int i;
  if (!where)
    return;

  strncpy(where, "$1$", 3);
  for (i = 3; i < 11; i++)
    where[i] = table[(int)(1.0 * strlen(table) * rand() / (RAND_MAX + 1.0))];
  where[11] = '$';
  where[12] = 0;
}

void
tempname(char *where, size_t len) {
  int i;
  if (!where)
    return;

  for (i = 0; i < len; i++)
    where[i] = table[(int)(1.0 * strlen(table) * rand() / (RAND_MAX + 1.0))];
  where[len] = 0;
}

