/*
  Plastic Logic hardware library - dac5820

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

#include "dac5820.h"
#include "i2cdev.h"
#include "libplhw.h"
#include <assert.h>

#define LOG_TAG "dac5820"
#include "log.h"

struct dac5820 {
	struct i2cdev *i2c;
};

struct dac5820 *dac5820_init(const char *i2c_bus, int i2c_addr)
{
	struct dac5820 *dac;

	dac = malloc(sizeof (struct dac5820));
	assert(dac != NULL);

	dac->i2c = i2cdev_init(i2c_bus, i2c_addr);

	if (dac->i2c == NULL) {
		LOG("failed to initialise I2C");
		dac5820_free(dac);
		dac = NULL;
	}

	return dac;
}

void dac5820_free(struct dac5820 *dac)
{
	assert(dac != NULL);

	if (dac->i2c != NULL)
		i2cdev_free(dac->i2c);

	free(dac);
}

int dac5820_set_power(struct dac5820 *dac, enum dac5820_channel_id channel,
		      enum dac5820_power_id power)
{
	union dac5820_ext_payload payload;
	int ret = 0;

	assert(dac != NULL);

	payload.cmd_byte.cmd = DAC5820_CMD_EXT__DATA_0;
	payload.cmd_byte.data_high = 0;
	payload.ext_byte.reserved = 0;

	switch (channel) {
	case DAC5820_CH_A:
		payload.ext_byte.a = 1;
		payload.ext_byte.b = 0;
		break;

	case DAC5820_CH_B:
		payload.ext_byte.a = 0;
		payload.ext_byte.b = 1;
		break;

	default:
		assert(!"invalid channel identifier");
		ret = -1;
		break;
	}

	switch (power) {
	case DAC5820_POW_ON:
		payload.ext_byte.pd = DAC5820_PD_ON;
		break;

	case DAC5820_POW_OFF_FLOAT:
		payload.ext_byte.pd = DAC5820_PD_OFF_FLOAT;
		break;

	case DAC5820_POW_OFF_1K:
		payload.ext_byte.pd = DAC5820_PD_OFF_1K;
		break;

	case DAC5820_POW_OFF_100K:
		payload.ext_byte.pd = DAC5820_PD_OFF_100K;
		break;

	default:
		assert(!"invalid power mode indentifier");
		ret = -1;
		break;
	}

	if (!ret)
		ret = i2cdev_write(dac->i2c, payload.bytes, sizeof payload);

	return ret;
}

int dac5820_output(struct dac5820 *dac, enum dac5820_channel_id channel,
		   char value)
{
	enum dac5820_write_cmd_id cmd;
	int ret = 0;

	assert(dac != NULL);

	switch (channel) {
	case DAC5820_CH_A:
		cmd = DAC5820_CMD_LOAD_IN_DAC_A__UP_DAC_B__OUT_AB;
		break;

	case DAC5820_CH_B:
		cmd = DAC5820_CMD_LOAD_IN_DAC_B__UP_DAC_A__OUT_AB;
		break;

	default:
		assert(!"invalid channel identifier");
		ret = -1;
		break;
	}

	if (!ret) {
		union dac5820_write_payload payload;

		payload.cmd_byte.cmd = cmd;
		payload.cmd_byte.data_high = (value >> 4) & 0xF;
		payload.data_byte.data_low = value & 0xF;
		payload.data_byte.reserved = 0;

		ret = i2cdev_write(dac->i2c, payload.bytes, sizeof payload);
	}

	return ret;
}
