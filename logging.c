/*
 * UserServer/0.1
 * logging.c - Logging facility
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

#include <syslog.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>

#include "userv.h"
#include "functions.h"

static char *month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

void
Log(int type, char *fmt, ...) {
  char buf1[256];
  va_list ap;
  FILE *lf;
  int prior;
  time_t now;
  struct tm *tm;

  va_start(ap, fmt);
  vsnprintf(buf1, 255, fmt, ap);
  va_end(ap);
  if ((lf = fopen("/var/log/userv.log", "a")) != NULL) {
    now = time(NULL);
    tm = localtime((const time_t *)&now);
    
    fprintf(lf, "%s %2d %02d:%02d:%02d UserServer[%d] %s: %s\n", month[tm->tm_mon], tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, getpid(), (type == MT_ERROR) ? "ERR" : "MSG", buf1);
    
    fclose(lf);
  }

  openlog("UserServer", LOG_PID, LOG_DAEMON);
  prior = LOG_DAEMON;
  prior |= (type == MT_ERROR) ? LOG_ERR : LOG_NOTICE;
  syslog(prior, "%s: %s", (type == MT_ERROR) ? "ERR" : "MSG", buf1);
  closelog();
}

