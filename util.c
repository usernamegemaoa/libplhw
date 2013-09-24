/*
  Plastic Logic hardware library - util

  Copyright (C) 2011, 2012, 2013 Plastic Logic Limited

      Guillaume Tucker <guillaume.tucker@plasticlogic.com>

  This program is free software: you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
  License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "util.h"
#include <assert.h>
#include <unistd.h>

#define LOG_TAG "util"
#include <plsdk/log.h>

int wait_cmd(void *ctx, int cmd, int set_value, int get_value,
	     unsigned poll_us, unsigned timeout,
	     cmd_func_t read, cmd_func_t get, cmd_set_func_t set)
{
	int stat;
	int cmd_res;

	assert(read != NULL);
	assert(get != NULL);
	assert(set != NULL);

	stat = set(ctx, cmd, set_value);

	if (stat) {
		LOG("failed to set cmd %i to %i", cmd, set_value);
		return -1;
	}

	do {
		usleep(poll_us);
		stat = read(ctx, cmd);

		if (stat) {
			LOG("failed to read cmd");
			return -1;
		}

		cmd_res = get(ctx, cmd);

		if (timeout > poll_us) {
			timeout -= poll_us;
		} else if (cmd_res != get_value) {
			LOG("timeout while waiting for cmd %i to be %i",
			    cmd, get_value);
			return -1;
		}
	} while (cmd_res != get_value);

	return 0;
}
