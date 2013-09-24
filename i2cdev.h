/*
  Plastic Logic hardware library - i2cdev

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

#ifndef INCLUDE_I2C_DEV_H
#define INCLUDE_I2C_DEV_H 1

#include <stdlib.h>

struct i2cdev;

enum i2cdev_flag {
	I2CDEV_VERBOSE_LOG,
	I2CDEV_IGNORE_WRITE_NAK,
	I2CDEV_IGNORE_READ_NAK,
};

extern struct i2cdev *i2cdev_init(const char *bus_device, char address);
extern void i2cdev_free(struct i2cdev *i2cdev);

extern void i2cdev_set_flag(struct i2cdev *d, enum i2cdev_flag f, int enable);
extern int i2cdev_read(struct i2cdev *d, void *data, size_t size);
extern int i2cdev_write(struct i2cdev *d, const void *data, size_t size);
extern int i2cdev_read_reg(struct i2cdev *d, const void *reg, size_t reg_sz,
			   void *data, size_t data_sz);
extern int i2cdev_read_reg8(struct i2cdev *d, char reg, void *data, size_t sz);
extern int i2cdev_write_reg(struct i2cdev *d, const void *reg, size_t reg_sz,
			    const void *data, size_t data_sz);
extern int i2cdev_write_reg8(struct i2cdev *d, char reg, const void *data,
			     size_t sz);

struct plconfig;

extern char i2cdev_get_config_addr(struct plconfig *p, const char *key,
				   char def);

#endif /* INCLUDE_I2C_DEV_H */
