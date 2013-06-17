/*
 * Copyright (C) 2011, 2012 Plastic Logic Limited
 * All rights reserved.
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
