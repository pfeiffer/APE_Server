/*
  Copyright (C) 2010  Philipp Fuehrer <pf@netzbeben.de>

  This file is part of APE Server.
  APE is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  APE is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with APE ; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

/* channel_history.h */

#ifndef _CHANNEL_HISTORY_H
#define _CHANNEL_HISTORY_H

#include "main.h"
#include "channel.h"
#include "json.h"
#include "raw.h"
#include "extend.h"
#include "config.h"
#include "users.h"


typedef struct CHANNEL_HISTORY_NODE
{
	struct RAW *value;
	struct CHANNEL_HISTORY_NODE *next;
} CHANNEL_HISTORY_NODE;

typedef struct CHANNEL_HISTORY_DEQUE
{
	int length;
	char *filter;	
	struct CHANNEL_HISTORY_NODE *head;
	struct CHANNEL_HISTORY_NODE *tail;
} CHANNEL_HISTORY_DEQUE;


const int get_channel_max_history_size(CHANNEL *chan); 
CHANNEL_HISTORY_DEQUE *init_channel_history(CHANNEL *chan, apeconfig *config);
void free_channel_history(CHANNEL_HISTORY_DEQUE *deque);

void push_raw_to_channel_history(CHANNEL *chan, RAW *raw, apeconfig *config);
void update_chan_history_size(CHANNEL *chan);
void post_channel_history(CHANNEL *chan, USERS *user, acetables *g_ape);

#endif
