/*
 * UserServer/0.1
 * misc.c - Miscellanous functions
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

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <pwd.h>

#include "userv.h"
#include "functions.h"

char *trim(char *text, char *spc) {
  int start, end;
  char *b;

  if (spc) {
    for (start = 0; start < strlen(text); start++)
      if (!strchr(spc, *(text + start)))
        break;
    for (end = strlen(text) - 1; end >= 0; end--)
      if (!strchr(spc, *(text + end)))
        break;
  } else {
    for (start = 0; start < strlen(text); start++)
      if (!isspace(*(text + start)))
        break;
    for (end = strlen(text) - 1; end >= 0; end--)
      if (!isspace(*(text + end)))
        break;
  }
  if (end == -1)
    return NULL;

  if ((b = malloc(end - start + 2)) == NULL)
    return NULL;
  memset(b, 0, end - start + 2);

  strncpy(b, text + start, end - start + 1);
  
  return b;
}

int
valid_ip(char *addr) {
  int i, np = 0, nn = 0;

  if (!addr)
    return 0;

  if (strlen(addr) > 15)
    return 0;

  if (!isdigit(addr[0]))
    return 0;

  for (i = 0; i < strlen(addr); i++) {
    if (*(addr + i) == '.')
      np++;
    if ((*(addr + i) != '.') && (!isdigit(*(addr + i))))
      return 0;
  }

  if (np != 3)
    return 0;

  np = 0;

  for (i = 0; i < strlen(addr); i++) {
    if (*(addr + i) == '.') {
      if (np)
        return 0;
      np = 1;
      nn = 0;
    }
    if (*(addr + i) != '.') {
      np = 0;
      nn++;
      if (nn == 4)
        return 0;
    }
    if (i > 0)
      if ((*(addr + i - 1) == '0') && (*(addr + i) != '.'))
        return 0;
  }

  if (strcmp(addr, "0.0.0.0") == 0)
    return 0;
  
  if (strcmp(addr, "255.255.255.255") == 0)
    return 0;

  return 1;
}

int
valid_user(char *name) {
  if (getpwnam(name) == NULL)
    return 0;
  return 1;
}

int
ip_allowed(char *IP) {
  CST *temp = caddr;
  while (temp) {
    if (strcmp(IP, temp->IP) == 0)
      return 1;
    temp = temp->next;
  }
  return 0;
}

int
user_allowed(char *uname) {
  UST *temp = auser;

  if (strcmp(uname, "root") == 0)
    return 0;

  while (temp) {
    if (strcmp(uname, temp->uname) == 0)
      return 1;
    temp = temp->next;
  }
  return 0;
}

int
IsNum(char *str) {
  int i;

  if (!str)
    return 0;

  for (i = 0; i < strlen(str); i++)
    if (!isdigit(*(str + i)))
      return 0;
  
  return 1;
}

