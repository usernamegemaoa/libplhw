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

#ifndef INCLUDE_UTIL_H
#define INCLUDE_UTIL_H 1

typedef int (*cmd_func_t)(void *ctx, int cmd_id);
typedef int (*cmd_set_func_t)(void *ctx, int cmd_id, int on);

extern int wait_cmd(void *ctx, int cmd, int set_value, int get_value,
		    unsigned poll_us, unsigned timeout,
		    cmd_func_t read, cmd_func_t get, cmd_set_func_t set);

#endif /* INCLUDE_UTIL_H */
