/*
 * Copyright (C) 2011, 2012 Plastic Logic Limited
 * All rights reserved.
 */

#include "gpio_signals.h"
#include "gpioex.h"
#include "libplhw.h"
#include <assert.h>
#include <unistd.h>

#define LOG_TAG "pbtn"
#include "log.h"

#define PBTN_DEF_POLL_SLEEP_US 100000

struct pbtn {
	struct gpioex *gpio;
	unsigned poll_sleep_us;
	char btns;
	pbtn_abort_t abort;
};

static int wait_btn(struct pbtn *b, enum pbtn_id mask, int state, int any);

struct pbtn *pbtn_init(const char *i2c_bus, int i2c_addr)
{
	struct pbtn *b;

	b = malloc(sizeof (struct pbtn));
	assert(b != NULL);

	b->gpio = gpioex_init(i2c_bus, i2c_addr, GPIO_PBTN_I_MASK,
			      GPIO_PBTN_O_MASK);

	if (b->gpio == NULL) {
		LOG("failed to initialise GPIO expander");
		pbtn_free(b);
		b = NULL;
	} else {
		b->btns = 0;
		b->poll_sleep_us = PBTN_DEF_POLL_SLEEP_US;
	}

	return b;
}

void pbtn_free(struct pbtn *b)
{
	assert(b != NULL);

	if (b->gpio != NULL)
		gpioex_free(b->gpio);

	free(b);
}

int pbtn_probe(struct pbtn *b, enum pbtn_id mask)
{
	char port;

	assert(b != NULL);

	if (gpioex_get(b->gpio, &port) < 0)
		return -1;

	return (port & mask) ? 0 : 1;
}

void pbtn_set_abort_cb(struct pbtn *pbtn, pbtn_abort_t abort)
{
	assert(pbtn != NULL);

	pbtn->abort = abort;
}

int pbtn_wait(struct pbtn *b, enum pbtn_id mask, int state)
{
	assert(b != NULL);

	return wait_btn(b, mask, state, 0);
}

int pbtn_wait_any(struct pbtn *b, enum pbtn_id mask, int state)
{
	assert(b != NULL);

	return wait_btn(b, mask, state, 1);
}

/* ----------------------------------------------------------------------------
 * static functions
 */

static int wait_btn(struct pbtn *b, enum pbtn_id mask, int state, int any)
{
	char port;
	int ret = 0;

	assert(b != NULL);

	while (!ret) {
		const int abort_ret = (b->abort == NULL) ? 0 : b->abort();

		if (abort_ret) {
			ret = abort_ret;
			break;
		}

		ret = gpioex_get(b->gpio, &port);

		if (ret < 0)
			break;

		if (any) {
			if (state)
				port = ~port;

			ret = port & mask;
		} else {
			if (!state)
				port = ~port;

			ret = (port & mask) ? 0 : 1;
		}

		if (!ret)
			usleep(b->poll_sleep_us);
	}

	return ret;
}
