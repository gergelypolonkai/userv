/*
 * UserServer/0.1
 * main.c - Main facility
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

#define USERV_MAIN

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>

#include "userv.h"
#include "functions.h"

LST *laddr = NULL;
CST *caddr = NULL;
UST *auser = NULL;
NST *nconn = NULL;
SST *saddr = NULL;
PST *paddr = NULL;

int
main(void) {
  int t;
  struct timeval tv;
  FILE *pf;

  SetSignals();

  if (daemon(1, 0) == -1) {
    Log(MT_ERROR, "Couldn't detach from controlling terminal");
    CleanUp(0);
    unlink("/var/run/userv.pid");
    return -1;
  }

  if ((pf = fopen("/var/run/userv.pid", "w")) == NULL) {
    Log(MT_ERROR, "Unable to create pidfile");
    return -1;
  }
  fprintf(pf, "%d\n", getpid());
  fclose(pf);

  if (gettimeofday(&tv, NULL) != 0) {
    Log(MT_ERROR, "Unable to get system time");
    unlink("/var/run/userv.pid");
    return -1;
  }
  srand(tv.tv_usec);

  if (geteuid() != 0) {
    Log(MT_ERROR, "This server MUST be run as root!");
    unlink("/var/run/userv.pid");
    return -1;
  }

  t = ParseConfig();
  if (t != 0) {
    CleanUp(0);
    unlink("/var/run/userv.pid");
  }
  if (t > 0) {
    Log(MT_ERROR, "Erronous line at /etc/userv.conf:%d", t);
    return -1;
  } else if (t < 0) {
    switch (ParseConfig()) {
      case ERR_NOCF:
        Log(MT_ERROR, "Config file /etc/userv.conf doesn't exist");
        return -1;
      case ERR_NOMEM:
        Log(MT_ERROR, "Not enough memory while processing /etc/userv.conf");
        return -1;
      default:
        Log(MT_ERROR, "Undefined error\n");
        return -2;
    }
  }

  if (StartListening() != ERR_OK) {
    Log(MT_ERROR, "Error occured while starting networking");
    CleanUp(0);
    unlink("/var/run/userv.pid");
    return -1;
  }

  if (MainLoop() != ERR_OK)
    Log(MT_ERROR, "Error during loop");

  unlink("/var/run/userv.pid");

  CleanUp(0);

  return 0;
}

