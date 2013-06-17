/*
 * Copyright (C) 2011, 2012 Plastic Logic Limited
 * All rights reserved.
 */

#include "gpioex.h"
#include "i2cdev.h"
#include "libplhw.h"
#include <assert.h>

#define LOG_TAG "gpioex"
#include "log.h"

struct gpioex {
	struct i2cdev *i2c;
	char i_value;
	char o_value;
	char i_mask;
	char o_mask;
	struct {
		int auto_write:1;
	} flags;
};

static int read_value(struct gpioex *g);
static int write_value(struct gpioex *g);

struct gpioex *gpioex_init(const char *i2c_bus, int i2c_address,
                           char i_mask, char o_mask)
{
	struct gpioex *g;
	int error = 1;

	assert(!(i_mask & o_mask));

	g = malloc(sizeof (struct gpioex));
	assert(g != NULL);

	g->i2c = i2cdev_init(i2c_bus, i2c_address);

	if (g->i2c == NULL)
		LOG("failed to initialise I2C");
	else if (i2cdev_write(g->i2c, &i_mask, 1) < 0)
		LOG("failed to intialise inputs");
	else if (read_value(g) < 0)
		LOG("failed to read initial value");
	else
		error = 0;

	if (error) {
		LOG("GPIO init failed (in: 0x%02X, out: 0x%02X)",
		    i_mask, o_mask);
		gpioex_free(g);
		g = NULL;
	} else {
		g->i_mask = i_mask;
		g->o_mask = o_mask;
		g->o_value = 0;
		g->flags.auto_write = 1;
	}


	return g;
}

void gpioex_free(struct gpioex *g)
{
	assert(g != NULL);

	if (g->i2c != NULL)
		i2cdev_free(g->i2c);

	free(g);
}

int gpioex_get(struct gpioex *g, char *value)
{
	assert(g != NULL);

	if (read_value(g) < 0)
		return -1;

	*value = (g->i_value | g->o_value);

	return 0;
}

int gpioex_set(struct gpioex *g, char value, int set_clear)
{
	char masked;

	assert(g != NULL);

	masked = value & g->o_mask;

	if (set_clear)
		g->o_value |= masked;
	else
		g->o_value &= ~masked;

	if (g->flags.auto_write)
		return write_value(g);

	return 0;
}

void gpioex_set_auto_write(struct gpioex *g, int enable)
{
	assert(g != NULL);

	g->flags.auto_write = enable ? 1 : 0;
}

/* ----------------------------------------------------------------------------
 * static functions
 */

static int read_value(struct gpioex *g)
{
	char value;

	if (i2cdev_read(g->i2c, &value, 1) < 0)
		return -1;

	g->i_value = value & g->i_mask;

	return 0;
}

static int write_value(struct gpioex *g)
{
	char value = g->o_value | g->i_mask;

	if (i2cdev_write(g->i2c, &value, 1) < 0)
		return -1;

	return 0;
}
