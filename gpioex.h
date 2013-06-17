/*
  Plastic Logic hardware library - gpioex

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

#ifndef INCLUDE_GPIOEX_H
#define INCLUDE_GPIOEX_H 1

struct gpioex;

extern struct gpioex *gpioex_init(const char *i2c_bus, int i2c_address,
                                  char i_mask, char o_mask);
extern void gpioex_free(struct gpioex *gpioex);

extern int gpioex_get(struct gpioex *gpioex, char *value);
extern int gpioex_set(struct gpioex *gpioex, char value, int set_clear);
extern void gpioex_set_auto_write(struct gpioex *gpioex, int enable);

#endif /* INCLUDE_GPIOEX_H */
