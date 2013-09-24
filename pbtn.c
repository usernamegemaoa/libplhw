/*
  Plastic Logic hardware library - pbtn

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

#include "gpio_signals.h"
#include "gpioex.h"
#include "i2cdev.h"
#include <libplhw.h>
#include <plsdk/plconfig.h>
#include <assert.h>
#include <unistd.h>

#define LOG_TAG "pbtn"
#include <plsdk/log.h>

#define PBTN_DEF_POLL_SLEEP_US 100000

struct pbtn {
	struct gpioex *gpio;
	struct plconfig *config;
	unsigned poll_sleep_us;
	char btns;
	pbtn_abort_t abort;
};

static int wait_btn(struct pbtn *b, enum pbtn_id mask, int state, int any);

struct pbtn *pbtn_init(const char *i2c_bus, int i2c_address)
{
	struct pbtn *b;

	b = malloc(sizeof (struct pbtn));

	if (b == NULL)
		return NULL;

	b->config = plconfig_init(NULL, "libplhw");

	if (b->config == NULL)
		goto err_free_pbtn;

	if (i2c_address == PLHW_NO_I2C_ADDR)
		i2c_address = i2cdev_get_config_addr(
			b->config, "pbtn-address", 0x21);

	b->gpio = gpioex_init(i2c_bus, i2c_address, GPIO_PBTN_I_MASK,
			      GPIO_PBTN_O_MASK);

	if (b->gpio == NULL) {
		LOG("failed to initialise GPIO expander");
		goto err_free_plconfig;
	}

	b->btns = 0;
	b->poll_sleep_us = PBTN_DEF_POLL_SLEEP_US;

	return b;

err_free_plconfig:
	plconfig_free(b->config);
err_free_pbtn:
	free(b);

	return NULL;
}

void pbtn_free(struct pbtn *b)
{
	assert(b != NULL);

	gpioex_free(b->gpio);
	plconfig_free(b->config);
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
