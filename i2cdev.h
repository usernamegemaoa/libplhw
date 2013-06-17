/*
 * Copyright (C) 2011, 2012 Plastic Logic Limited
 * All rights reserved.
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

#endif /* INCLUDE_I2C_DEV_H */
