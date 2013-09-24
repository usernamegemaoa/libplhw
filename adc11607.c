/*
  Plastic Logic hardware library - adc11607

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

#include "adc11607.h"
#include "i2cdev.h"
#include <libplhw.h>
#include <plsdk/plconfig.h>
#include <assert.h>

#define LOG_TAG "adc11607"
#include <plsdk/log.h>

struct adc11607 {
	struct i2cdev *i2c;
	struct plconfig *config;
	union adc11607_setup_config cmd;
	unsigned nb_channels;
	enum adc11607_ref_id ref_id;
	float ext_ref;
	float ref;
	adc11607_result_t results[ADC11607_NB_RESULTS];
};

static int set_init_config(struct adc11607 *adc);
static void set_nb_channels(struct adc11607 *adc);
static void set_ref(struct adc11607 *adc, struct adc11607_setup *setup,
		    enum adc11607_ref_id ref_id);
static void set_invalid_results(struct adc11607 *adc);

struct adc11607 *adc11607_init(const char *i2c_bus, int i2c_address)
{
	struct adc11607 *adc;
	int error = 1;

	adc = malloc(sizeof (struct adc11607));

	if (adc == NULL)
		return NULL;

	adc->config = plconfig_init(NULL, "libplhw");

	if (adc->config == NULL)
		goto err_free_adc;

	if (i2c_address == PLHW_NO_I2C_ADDR)
		i2c_address = i2cdev_get_config_addr(
			adc->config, "ADC11607-address", 0x34);

	adc->i2c = i2cdev_init(i2c_bus, i2c_address);

	if (adc->i2c == NULL)
		LOG("failed to initialise I2C");
	else if (set_init_config(adc) < 0)
		LOG("failed to set the initial configuration");
	else
		error = 0;

	if (error) {
		adc11607_free(adc);
		adc = NULL;
	}

	return adc;

err_free_adc:
	free(adc);

	return NULL;
}

void adc11607_free(struct adc11607 *adc)
{
	assert(adc != NULL);

	plconfig_free(adc->config);

	if (adc->i2c != NULL)
		i2cdev_free(adc->i2c);

	free(adc);
}

void adc11607_set_ext_ref_value(struct adc11607 *adc, float value)
{
	assert(adc != NULL);

	adc->ext_ref = value;
}

int adc11607_set_ref(struct adc11607 *adc, enum adc11607_ref_id ref_id)
{
	struct adc11607_setup *setup;

	assert(adc != NULL);

	setup = &adc->cmd.setup;
	set_ref(adc, setup, ref_id);

	return i2cdev_write(adc->i2c, setup, 1);
}

enum adc11607_ref_id adc11607_get_ref(struct adc11607 *adc)
{
	assert(adc != NULL);

	return adc->ref_id;
}

unsigned adc11607_get_nb_channels(struct adc11607 *adc)
{
	assert(adc != NULL);

	return adc->nb_channels;
}

int adc11607_select_channel_range(struct adc11607 *adc, unsigned range)
{
	assert(adc != NULL);

	set_invalid_results(adc);

	if (range >= adc->nb_channels) {
		LOG("invalid channel range number");
		return -1;
	}

	adc->cmd.config.cs = range;

	return i2cdev_write(adc->i2c, &adc->cmd.config, 1);
}

int adc11607_read_results(struct adc11607 *adc)
{
	char data[8];
	size_t n;
	size_t read_n;
	const char *it;
	unsigned i;

	assert(adc != NULL);

	n = adc->cmd.config.cs + 1;
	assert(n <= ADC11607_NB_RESULTS);

	read_n = n * 2;

	if (i2cdev_read(adc->i2c, data, read_n) < 0) {
		LOG("failed to read the results");
		set_invalid_results(adc);
		return -1;
	}

	for (it = data, i = 0; i < n; ++i, it += 2)
		adc->results[i] = ((it[0] & 0x03) << 8) | it[1];

	for (i = n; i < ADC11607_NB_RESULTS; ++i)
		adc->results[i] = ADC11607_INVALID_RESULT;

	return 0;
}

adc11607_result_t adc11607_get_result(struct adc11607 *adc, unsigned channel)
{
	assert(adc != NULL);
	assert(channel < adc->nb_channels);

	return adc->results[channel];
}

float adc11607_get_volts(struct adc11607 *adc, adc11607_result_t value)
{
	assert(adc != NULL);
	assert(value != ADC11607_INVALID_RESULT);
	assert(value <= ADC11607_MAX_VALUE);

	return (value * adc->ref) / ADC11607_MAX_VALUE;
}

unsigned adc11607_get_millivolts(struct adc11607 *adc, adc11607_result_t value)
{
	assert(adc != NULL);
	assert(value != ADC11607_INVALID_RESULT);
	assert(value <= ADC11607_MAX_VALUE);

	return (value * adc->ref * 1000) / ADC11607_MAX_VALUE;
}

/* ----------------------------------------------------------------------------
 * static functions
 */

static int set_init_config(struct adc11607 *adc)
{
	struct adc11607_setup * const setup = &adc->cmd.setup;
	struct adc11607_config * const config = &adc->cmd.config;

	setup->reset = 1;
	setup->bip_uni = 0;
	setup->clk_sel = 0;
	setup->setup_1 = 1;

	adc->ext_ref = 0.0;
	set_ref(adc, setup, ADC11607_REF_INTERNAL);

	config->se_diff = 1;
	config->cs = 3;
	config->scan = 0;
	config->config_0 = 0;

	if (i2cdev_write(adc->i2c, adc->cmd.bytes, sizeof adc->cmd) < 0)
		return -1;

	set_nb_channels(adc);
	set_invalid_results(adc);

	return 0;
}

static void set_nb_channels(struct adc11607 *adc)
{
	unsigned n;

	if (!adc->cmd.config.se_diff)
		n = 2;
	else if ((adc->cmd.setup.sel == 2) || (adc->cmd.setup.sel == 3))
		n = 3;
	else
		n = 4;

	adc->nb_channels = n;
}

static void set_ref(struct adc11607 *adc, struct adc11607_setup *setup,
		    enum adc11607_ref_id ref_id)
{
	switch (ref_id) {
	case ADC11607_REF_VDD:
		setup->sel = 0;
		adc->ref = 3.3;
		break;

	case ADC11607_REF_INTERNAL:
		setup->sel = ADC11607_SEL_INT_REF | ADC11607_SEL_INT_REF_ON;
		adc->ref = 2.048;
		break;

	case ADC11607_REF_EXTERNAL:
		setup->sel = ADC11607_SEL_EXT_REF;
		adc->ref = adc->ext_ref;
		break;

	default:
		assert(!"invalid reference voltage identifier");
		break;
	}

	adc->ref_id = ref_id;
}

static void set_invalid_results(struct adc11607 *adc)
{
	int i;

	for (i = 0; i < ADC11607_NB_RESULTS; ++i)
		adc->results[i] = ADC11607_INVALID_RESULT;
}
