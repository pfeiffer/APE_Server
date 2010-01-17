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

/* channel_history.c */

#include "channel_history.h"
#include "utils.h"


const int get_channel_max_history_size(CHANNEL *chan) {
    char *hs = get_property_val((chan)->properties, "max_history_size");
    return (hs != NULL) ? strtol(hs, NULL, 10) : 0;
}


void history_append_element(CHANNEL_HISTORY_DEQUE *deque, RAW *raw) {
	CHANNEL_HISTORY_NODE *node = xmalloc(sizeof(*node));
	node->value = raw;

	++deque->length;

	if (deque->head != NULL) {		
		node->next = deque->head;
		deque->tail = deque->tail->next = node;
		return;
	}

	node->next = node;
	deque->head = deque->tail = node;
}

void history_push_element(CHANNEL_HISTORY_DEQUE *deque, RAW *raw) {
	free_raw(deque->head->value);
	deque->head->value = raw;
	deque->tail = deque->head;	
	deque->head = deque->head->next;
}

void history_free_node(CHANNEL_HISTORY_NODE *node) {
	free_raw(node->value);
	free(node);
}

void deque_truncate_buffer(CHANNEL_HISTORY_DEQUE *deque, const int size) {
	int i = size;
	for (; i < deque->length; ++i) {
		deque->head = deque->head->next;
		history_free_node(deque->tail->next);
		deque->tail->next = deque->head;
	}
	if (size == 0 && i > size) {
		free(deque);
		deque = NULL;
	}
}


void free_channel_history(CHANNEL_HISTORY_DEQUE *deque) {
	if (deque == NULL) return;
	deque_truncate_buffer(deque, 0);
}

void update_chan_history_size(CHANNEL *chan) {
	if (chan->history == NULL) return;
	{
		const int max_history_size = get_channel_max_history_size(chan);

		if (max_history_size < chan->history->length) {
			deque_truncate_buffer(chan->history, max_history_size);
		}
	}
}


CHANNEL_HISTORY_DEQUE *new_channel_history(void) {
	struct CHANNEL_HISTORY_DEQUE *deque = xmalloc(sizeof(*deque));

	deque->length = 0;
	deque->filter = NULL;
	deque->head = deque->tail = NULL;

	return deque;
}

CHANNEL_HISTORY_DEQUE *init_channel_history(CHANNEL *chan, apeconfig *config) {
	char *config_val = CONFIG_VAL(Channels, history_default_max_size, config);
	if (config_val != NULL && atoi(config_val) > 0) {
		CHANNEL_HISTORY_DEQUE *deque = new_channel_history();
		add_property(&chan->properties, "max_history_size", config_val, EXTEND_STR, EXTEND_ISPRIVATE);

		config_val = CONFIG_VAL(Channels, history_default_raw_filter, config);
		if (config_val != NULL) {
			S_TOUPPER(config_val);
			if (strcmp(config_val, "ALL") != 0) {
				deque->filter = config_val;
			}
		}

		return deque;
	}

	return NULL;	
}


/* preconditions: memory for raw has been allocated and will not be freed later elsewhere, maxsize>0 */
void push_raw_to_channel_history(CHANNEL *chan, RAW *raw, apeconfig *config) {
	if (chan == NULL || chan->history == NULL) return;
	
	if (chan->history->filter != NULL) {
		ITERATE_CONFIG_VAL(chan->history->filter, " ", {
				char *type = strstr(raw->data, "\"raw\":\"");

				if (type == NULL || strncmp(type+7, token, strlen(token)) != 0) {
					free_raw(raw);
					return;
				}
		});
	}

	if (chan->history->length == get_channel_max_history_size(chan)) {
		history_push_element(chan->history, raw);
		return;
	} 

	history_append_element(chan->history, raw);		
}

void post_channel_history(CHANNEL *chan, USERS *user, acetables *g_ape) {
	if (get_channel_max_history_size(chan) > 0) {
		CHANNEL_HISTORY_NODE *node = chan->history->head;

		if (node != NULL) {
			do {
				post_raw(copy_raw_z(node->value), user, g_ape);
				node = node->next;
			} while (node != chan->history->head);
		}
	}

}

