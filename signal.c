/*
 * UserServer/0.1
 * signal.c - Signal handling
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
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "userv.h"
#include "functions.h"

int fcount = 0;

void
rehash(int signum) {
  Log(MT_MSG, "Signal caught, rehashing...");
  CleanUp(0);
  ParseConfig();
  StartListening();
}

void
shutdown(int signum) {
  Log(MT_MSG, "Signal caught, shutting down...");
  CleanUp(0);
  unlink("/var/run/userv.pid");
  exit(0);
}

void
fatal(int signum) {
  if (signum == SIGSEGV)
    Log(MT_ERROR, "segv");
  fcount++;
  Log(MT_ERROR, "Fatal signal caught, terminating...");
  PrintContext;
  unlink("/var/run/userv.pid");
  if (fcount > 1)
    exit(1);
  CleanUp(1);
  exit(1);
}

#define SA(x, y) memset(&sa, 0, sizeof(sa)); \
  sa.sa_handler = (y); \
  sigaction(x, &sa, NULL)
void
SetSignals(void) {
  struct sigaction sa;

  SA(SIGHUP, &rehash);
  SA(SIGINT, &shutdown);
  SA(SIGQUIT, &shutdown);
  SA(SIGILL, &fatal);
  SA(SIGABRT, &shutdown);
  SA(SIGFPE, &fatal);
  SA(SIGSEGV, &fatal);
  SA(SIGTERM, &shutdown);
  SA(SIGUSR1, SIG_IGN);
  SA(SIGUSR2, SIG_IGN);
  SA(SIGPIPE, SIG_IGN);
  SA(SIGCHLD, SIG_IGN);
  SA(SIGTTIN, SIG_IGN);
  SA(SIGTTOU, SIG_IGN);
}
#undef SA

