/*
 * UserServer/0.1
 * context.c - Context debugging
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

#include "userv.h"
#include "functions.h"

#ifdef CONTEXT_DEBUG
# include <string.h>
# include <stdio.h>

char fn[40] = "";
int ln = 0;

void
n_Context(char *fname, int line) {
  memset(&fn, 0, 40);
  strncpy((char *)&fn, fname, 39);
  ln = line;
}

void
n_PrintContext(void) {
  Log(MT_MSG, "Last context was at %s:%d", fn, ln);
}

#endif

