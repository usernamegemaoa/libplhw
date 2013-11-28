/*
  Plastic Logic hardware library - cpld

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

#include "cpld.h"
#include "i2cdev.h"
#include <libplhw.h>
#include <plsdk/plconfig.h>
#include <assert.h>
#include <unistd.h>

#define LOG_TAG "cpld"
#include <plsdk/log.h>

#define ARRAY_SIZE(array, type) (sizeof (array) / sizeof (type))

struct cpld {
	struct i2cdev *i2c;
	struct plconfig *config;
	union {
		struct {
			struct cpld_byte_0 b0;
			struct cpld_byte_1 b1;
			struct cpld_byte_2 b2;
		};
		char data[CPLD_NB_BYTES];
	};
};

struct cpld_supported_switches {
	char v_min;
	char v_max;
	const enum cpld_switch *switches;
	size_t n_switches;
};

static const enum cpld_switch CPLD_SWITCHES_V1[] = {
	CPLD_HVEN,
	CPLD_COM_SW_EN,
	CPLD_COM_SW_CLOSE,
	CPLD_COM_PSU,
	CPLD_BPCOM_CLAMP,
};

#define SET_SWITCHES(min, max, s) {				\
	.v_min = min,						\
	.v_max = max,						\
	.switches = s,						\
	.n_switches = ARRAY_SIZE(s, enum cpld_switch)		\
 }

static const struct cpld_supported_switches CPLD_SUPPORTED_SWITCHES[] = {
	SET_SWITCHES(0, 1, CPLD_SWITCHES_V1),
};

#undef SET_SWITCHES

static const size_t CPLD_SWITCHES_TABLE_LEN =
	ARRAY_SIZE(CPLD_SUPPORTED_SWITCHES, struct cpld_supported_switches);

static int is_switch_supported(struct cpld *cpld, enum cpld_switch sw);
static int read_i2c_data(struct cpld *cpld);
static int write_i2c_data(struct cpld *cpld);

struct cpld *cpld_init(const char *i2c_bus, char i2c_address)
{
	struct cpld *cpld;

	cpld = malloc(sizeof (struct cpld));

	if (cpld == NULL)
		return cpld;

	cpld->config = plconfig_init(NULL, "libplhw");

	if (cpld->config == NULL)
		goto err_free_cpld;

	if (i2c_address == PLHW_NO_I2C_ADDR)
		i2c_address = plconfig_get_i2c_addr(
			cpld->config, "CPLD-address", 0x70);

	cpld->i2c = i2cdev_init(i2c_bus, i2c_address);

	if (cpld->i2c == NULL) {
		LOG("failed to initialise I2C");
		goto err_free_plconfig;
	}

	if (read_i2c_data(cpld) < 0) {
		LOG("failed to read the I2C data");
		goto err_free_i2cdev;
	}

	return cpld;

err_free_i2cdev:
	i2cdev_free(cpld->i2c);
err_free_plconfig:
	plconfig_free(cpld->config);
err_free_cpld:
	free(cpld);

	return NULL;
}

void cpld_free(struct cpld *cpld)
{
	assert(cpld != NULL);

	i2cdev_free(cpld->i2c);
	plconfig_free(cpld->config);
	free(cpld);
}

int cpld_get_version(const struct cpld *cpld)
{
	assert(cpld != NULL);

	return cpld->b0.version;
}

int cpld_get_board_id(const struct cpld *cpld)
{
	assert(cpld != NULL);

	return cpld->b2.board_id;
}

size_t cpld_get_data_size(const struct cpld *cpld)
{
	assert(cpld != NULL);

	return CPLD_NB_BYTES;
}

int cpld_dump(const struct cpld *cpld, char *data, size_t size)
{
	unsigned i;
	const char *in;
	char *out;

	assert(cpld != NULL);
	assert(data != NULL);

	for (i = 0, in = cpld->data, out = data;
	     (i < size) && (i < CPLD_NB_BYTES);
	     ++i, ++in, ++out) {
		*out = *in;
	}

	return i;
}

int cpld_set_switch(struct cpld *cpld, enum cpld_switch sw, int on)
{
	struct cpld_byte_0 *b0;
	struct cpld_byte_1 *b1;

	assert(cpld != NULL);

	if (!is_switch_supported(cpld, sw))
		return -1;

	b0 = &cpld->b0;
	b1 = &cpld->b1;

	switch (sw) {
	case CPLD_HVEN:             b0->cpld_hven        = on ? 1 : 0;  break;
	case CPLD_COM_SW_EN:        b1->vcom_sw_en       = on ? 1 : 0;  break;
	case CPLD_COM_SW_CLOSE:     b1->vcom_sw_close    = on ? 1 : 0;  break;
	case CPLD_COM_PSU:          b1->vcom_psu_en      = on ? 1 : 0;  break;
	case CPLD_BPCOM_CLAMP:      b0->bpcom_clamp      = on ? 1 : 0;  break;
	default:
		assert(!"invalid switch identifier");
		break;
	}

	return write_i2c_data(cpld);
}

int cpld_get_switch(struct cpld *cpld, enum cpld_switch sw)
{
	struct cpld_byte_0 *b0;
	struct cpld_byte_1 *b1;
	int ret;

	assert(cpld != NULL);

	if (!is_switch_supported(cpld, sw))
		return -1;

	b0 = &cpld->b0;
	b1 = &cpld->b1;

	switch (sw) {
	case CPLD_HVEN:             ret = b0->cpld_hven     ? 1 : 0;  break;
	case CPLD_COM_SW_EN:        ret = b1->vcom_sw_en    ? 1 : 0;  break;
	case CPLD_COM_SW_CLOSE:     ret = b1->vcom_sw_close ? 1 : 0;  break;
	case CPLD_COM_PSU:          ret = b1->vcom_psu_en   ? 1 : 0;  break;
	case CPLD_BPCOM_CLAMP:      ret = b0->bpcom_clamp   ? 1 : 0;  break;
	default:
		assert(!"invalid switch identifier");
		ret = -1;
		break;
	}

	return ret;
}

/* ----------------------------------------------------------------------------
 * static functions
 */

static int is_switch_supported(struct cpld *cpld, enum cpld_switch sw)
{
	const struct cpld_supported_switches *it;
	const struct cpld_supported_switches *end;
	const char v = cpld->b0.version;

	end = &CPLD_SUPPORTED_SWITCHES[CPLD_SWITCHES_TABLE_LEN];

	for (it = CPLD_SUPPORTED_SWITCHES; it != end; ++it) {
		const enum cpld_switch *s;
		const enum cpld_switch *s_end;

		if ((v < it->v_min) || (v > it->v_max))
			continue;

		s_end = &it->switches[it->n_switches];

		for (s = it->switches; s != s_end; ++s)
			if (*s == sw)
				return 1;
	}

	LOG("switch (%i) not supported with this CPLD version (%i)", sw, v);

	return 0;
}

static int read_i2c_data(struct cpld *cpld)
{
	return i2cdev_read(cpld->i2c, cpld->data, CPLD_NB_BYTES);
}

static int write_i2c_data(struct cpld *cpld)
{
	return i2cdev_write(cpld->i2c, cpld->data, CPLD_NB_BYTES);
}
