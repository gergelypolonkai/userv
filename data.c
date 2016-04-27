/*
 * UserServer/0.1
 * data.c - Handling local data
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
#include <stdlib.h>
#include <unistd.h>

#include "userv.h"
#include "functions.h"

int
AddListen(char *IP) {
  LST *temp;
  char *ip;

  if ((ip = strdup(IP)) == NULL)
    return ERR_NOMEM;

  if (laddr) {
    temp = laddr;
    while (temp->next)
      temp = temp->next;
    if ((temp->next = malloc(LSTS)) == NULL)
      return ERR_NOMEM;
    temp->next->next = NULL;
    temp->next->IP = ip;
  } else {
    if ((laddr = malloc(LSTS)) == NULL)
      return ERR_NOMEM;
    laddr->next = NULL;
    laddr->IP = ip;
  }
  
  return ERR_OK;
}

int
AddConfig(char *IP) {
  SST *temp;
  char *ip;

  if ((ip = strdup(IP)) == NULL)
    return ERR_NOMEM;

  if (saddr) {
    temp = saddr;
    while (temp->next)
      temp = temp->next;
    if ((temp->next = malloc(SSTS)) == NULL)
      return ERR_NOMEM;
    temp->next->next = NULL;
    temp->next->IP = ip;
  } else {
    if ((saddr = malloc(SSTS)) == NULL)
      return ERR_NOMEM;
    saddr->next = NULL;
    saddr->IP = ip;
  }

  return ERR_OK;
}

int
AddPeer(char *IP) {
  PST *temp;
  char *ip;

  if ((ip = strdup(IP)) == NULL)
    return ERR_NOMEM;

  if (paddr) {
    temp = paddr;
    while (temp->next)
      temp = temp->next;
    if ((temp->next = malloc(SSTS)) == NULL)
      return ERR_NOMEM;
    temp->next->next = NULL;
    temp->next->IP = ip;
  } else {
    if ((paddr = malloc(SSTS)) == NULL)
      return ERR_NOMEM;
    paddr->next = NULL;
    paddr->IP = ip;
  }

  return ERR_OK;
}

int
AddClient(char *IP) {
  CST *temp;
  char *ip;

  if ((ip = strdup(IP)) == NULL)
    return ERR_NOMEM;

  if (caddr) {
    temp = caddr;
    while (temp->next)
      temp = temp->next;
    if ((temp->next = malloc(CSTS)) == NULL)
      return ERR_NOMEM;
    temp->next->next = NULL;
    temp->next->IP = ip;
  } else {
    if ((caddr = malloc(CSTS)) == NULL)
      return ERR_NOMEM;
    caddr->next = NULL;
    caddr->IP = ip;
  }
  
  return ERR_OK;
}

int
AddUser(char *name) {
  UST *temp;
  char *un;

  if ((un = strdup(name)) == NULL)
    return ERR_NOMEM;

  if (auser) {
    temp = auser;
    while (temp->next)
      temp = temp->next;
    if ((temp->next = malloc(USTS)) == NULL)
      return ERR_NOMEM;
    temp->next->next = NULL;
    temp->next->uname = un;
  } else {
    if ((auser = malloc(USTS)) == NULL)
      return ERR_NOMEM;
    auser->next = NULL;
    auser->uname = un;
  }

  return ERR_OK;
}

void
CleanUp(int type) {
  if (laddr) {
    LST *temp;
    temp = laddr->next;
    if (laddr->IP)
      free(laddr->IP);
    free(laddr);
    while (temp) {
      LST *t = temp;
      temp = temp->next;
      if (t->IP)
        free(t->IP);
      free(t);
    }
  }
  laddr = NULL;

  if (paddr) {
    PST *temp;
    temp = paddr->next;
    if (paddr->IP)
      free(paddr->IP);
    free(paddr);
    while (temp) {
      PST *t = temp;
      temp = temp->next;
      if (t->IP)
        free(t->IP);
      free(t);
    }
  }
  paddr = NULL;

  if (saddr) {
    SST *temp;
    temp = saddr->next;
    if (saddr->IP)
      free(saddr->IP);
    free(saddr);
    while (temp) {
      SST *t = temp;
      temp = temp->next;
      if (t->IP)
        free(t->IP);
      free(t);
    }
  }
  saddr = NULL;

  if (caddr) {
    CST *temp;
    temp = caddr->next;
    if (caddr->IP)
      free(caddr->IP);
    free(caddr);
    while (temp) {
      CST *t = temp;
      temp = temp->next;
      if (t->IP)
        free(t->IP);
      free(t);
    }
  }
  caddr = NULL;

  if (auser) {
    UST *temp;
    temp = auser->next;
    if (auser->uname)
      free(auser->uname);
    free(auser);
    while (temp) {
      UST *t = temp;
      temp = temp->next;
      if (t->uname)
        free(t->uname);
      free(t);
    }
  }
  auser = NULL;

  while (nconn) {
    if ((nconn->sfd != -1) && (type == 0) && (nconn->type == CT_CLIENT))
      Send(nconn->sfd, "T\n");
    DelConn(nconn->sfd);
  }
  nconn = NULL;
}

