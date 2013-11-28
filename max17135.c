/*
  Plastic Logic hardware library - max17135

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

#include "max17135.h"
#include "i2cdev.h"
#include <libplhw.h>
#include <plsdk/plconfig.h>
#include <assert.h>
#include <unistd.h>

/* Set to 1 to allow timing registers to be persistently saved in the chip.
 * IMPORTANT: This can be performed only 3 times, further attempts will cause
 * the chip to malfunction.  */
#define MAX17135_ALLOW_SAVE 0

#define LOG_TAG "max17135"
#include <plsdk/log.h>

struct max17135 {
	struct i2cdev *i2c;
	struct plconfig *config;
	char prod_id;
	char prod_rev;
	char timing[MAX17135_NB_TIMINGS];
	struct {
		unsigned timings_read:1;
		unsigned timings_written:1;
	} flags;
	unsigned pok_delay_us;
};

static int read_const_registers(struct max17135 *p);
static int read_timings(struct max17135 *p);
static int write_timings(struct max17135 *p);
static int save_timings(struct max17135 *p);

struct max17135 *max17135_init(const char *i2c_bus, char i2c_address)
{
	struct max17135 *p;

	p = malloc(sizeof(struct max17135));

	if (p == NULL)
		return NULL;

	p->config = plconfig_init(NULL, "libplhw");

	if (p->config == NULL)
		goto err_free_max17135;

	if (i2c_address == PLHW_NO_I2C_ADDR)
		i2c_address = plconfig_get_i2c_addr(
			p->config, "MAX17135-address", 0x48);

	p->i2c = i2cdev_init(i2c_bus, i2c_address);

	if (p->i2c == NULL) {
		LOG("failed to initialise I2C");
		goto err_free_plconfig;
	}

	if (read_const_registers(p)) {
		LOG("failed to read registers");
		goto err_free_i2cdev;
	}

	p->flags.timings_read = 0;
	p->flags.timings_written = 0;
	p->pok_delay_us = 10000;

	return p;

err_free_i2cdev:
	i2cdev_free(p->i2c);
err_free_plconfig:
	plconfig_free(p->config);
err_free_max17135:
	free(p);

	return NULL;
}

void max17135_free(struct max17135 *p)
{
	assert(p != NULL);

	i2cdev_free(p->i2c);
	plconfig_free(p->config);
	free(p);
}

int max17135_get_prod_id(struct max17135 *p)
{
	assert(p != NULL);

	return p->prod_id;
}

int max17135_get_prod_rev(struct max17135 *p)
{
	assert(p != NULL);

	return p->prod_rev;
}

int max17135_get_vcom(struct max17135 *p, char *dvr)
{
	assert(p != NULL);
	assert(dvr != NULL);

	if (i2cdev_read_reg8(p->i2c, MAX17135_REG_DVR, dvr, 1))
		return -1;

	return 0;
}

int max17135_set_vcom(struct max17135 *p, char value)
{
	assert(p != NULL);

	return i2cdev_write_reg8(p->i2c, MAX17135_REG_DVR, &value, 1);
}

int max17135_save_vcom(struct max17135 *p)
{
#if MAX17135_ALLOW_SAVE
	union max17135_prog prog;

	assert(p != NULL);

	prog.byte = 0;
	prog.dvr = 1;

	return i2cdev_write_reg8(p->i2c, MAX17135_REG_PROG, &prog.byte, 1);
#else
	LOG("writing the VCOM value is not allowed");
	return -1;
#endif
}

int max17135_get_timing(struct max17135 *p, unsigned n)
{
	assert(p != NULL);
	assert(n < MAX17135_NB_TIMINGS);

	if (read_timings(p))
		return -1;

	return p->timing[n];
}

int max17135_get_timings(struct max17135 *p, char *data, size_t size)
{
	const int rd_size =
		(size < MAX17135_NB_TIMINGS) ? size : MAX17135_NB_TIMINGS;
	const char *in;
	char *out;
	int i;

	assert(p != NULL);
	assert(data != NULL);

	if (read_timings(p))
		return -1;

	in = p->timing;
	out = data;

	for (i = 0; i < rd_size; ++i)
		*out++ = *in++;

	return i;
}

int max17135_set_timing(struct max17135 *p, unsigned n, char value)
{
	assert(p != NULL);
	assert(n < MAX17135_NB_TIMINGS);

	if (read_timings(p))
		return -1;

	p->timing[n] = value;

	return write_timings(p);
}

int max17135_set_timings(struct max17135 *p, const char *data, size_t size)
{
	const size_t wr_size =
		(size < MAX17135_NB_TIMINGS) ? size : MAX17135_NB_TIMINGS;
	const char *in;
	char *out;
	unsigned i;

	assert(p != NULL);

	if (read_timings(p))
		return -1;

	in = data;
	out = p->timing;

	for (i = 0; i < wr_size; ++i)
		*out++ = *in++;

	return write_timings(p);
}

int max17135_save_timings(struct max17135 *p)
{
	assert(p != NULL);

	if (!p->flags.timings_written)
		return 0;

	return save_timings(p);
}

int max17135_get_temp_sensor_en(struct max17135 *p)
{
	union max17135_conf conf;
	int ret;

	assert(p != NULL);

	ret = i2cdev_read_reg8(p->i2c, MAX17135_REG_CONF, &conf.byte, 1);

	if (!ret)
		ret = conf.shutdown ? 0 : 1;

	return ret;
}

int max17135_set_temp_sensor_en(struct max17135 *p, int en)
{
	union max17135_conf conf;
	int ret;

	assert(p != NULL);

	ret = i2cdev_read_reg8(p->i2c, MAX17135_REG_CONF, &conf.byte, 1);

	if (!ret) {
		conf.shutdown = en ? 0 : 1;
		ret = i2cdev_write_reg8(p->i2c, MAX17135_REG_CONF,
					&conf.byte,1);
	}

	return ret;
}

int max17135_get_temperature(struct max17135 *p, short *temp,
			     enum max17135_temp_id id)
{
	char reg;
	char value[2];

	assert(p != NULL);
	assert(temp != NULL);

	switch (id) {
	case MAX17135_TEMP_EXT:
		reg = MAX17135_REG_EXT_TEMP;
		break;

	case MAX17135_TEMP_INT:
		reg = MAX17135_REG_INT_TEMP;
		break;

	default:
		assert(!"invalid temperature identifier");
		return -1;
	}

	if (i2cdev_read_reg8(p->i2c, reg, value, 2))
		return -1;

	*temp = (value[0] << 8) | (value[1]);

	return 0;
}

int max17135_get_temp_failure(struct max17135 *p)
{
	union max17135_temp_stat stat;

	assert(p != NULL);

	if (i2cdev_read_reg8(p->i2c, MAX17135_REG_TEMP_STAT, &stat.byte, 1))
		return -1;

	if (stat.open)
		return MAX17135_TEMP_OPEN;

	if (stat.shrt)
		return MAX17135_TEMP_SHORT;

	return MAX17135_TEMP_OK;
}

float max17135_convert_temperature(struct max17135 *p, short temp)
{
	assert(p != NULL);

	return (float) (((temp >> 7) & 0x1FF)) / 2;
}

int max17135_get_pok(struct max17135 *p)
{
	union max17135_fault fault;

	assert(p != NULL);

	if (i2cdev_read_reg8(p->i2c, MAX17135_REG_FAULT, &fault.byte, 1))
		return -1;

	return fault.pok;
}

void max17135_set_pok_delay(struct max17135 *p, unsigned delay_us)
{
	assert(p != NULL);

	p->pok_delay_us = delay_us;
}

int max17135_wait_for_pok(struct max17135 *p)
{
	static const int POLL_SLEEP_US = 5000;
	static const int POLL_LOOPS = 1000000 / POLL_SLEEP_US;
	int pok;
	int i;

	assert(p != NULL);

	usleep(p->pok_delay_us);

	for (i = POLL_LOOPS, pok = 0; i && !pok; --i) {
		pok = max17135_get_pok(p);

		if (pok < 0)
			LOG("failed to get POK status");
		else if (!pok)
			usleep(POLL_SLEEP_US);
	}

	if (!pok) {
		LOG("time out waiting for POK");
		return -1;
	}

	return 0;
}

int max17135_set_en(struct max17135 *p, enum max17135_en_id id, int on)
{
	union max17135_enable enable;

	assert(p != NULL);

	if (i2cdev_read_reg8(p->i2c, MAX17135_REG_ENABLE, &enable.byte, 1))
		return -1;

	switch (id) {
	case MAX17135_EN_EN:      enable.en     = on ? 1 : 0;    break;
	case MAX17135_EN_CEN:     enable.cen    = on ? 1 : 0;    break;
	case MAX17135_EN_CEN2:    enable.cen2   = on ? 1 : 0;    break;
	default:
		assert(!"invalid HV enable identifier");
		return -1;
	}

	return i2cdev_write_reg8(p->i2c, MAX17135_REG_ENABLE, &enable.byte, 1);
}

int max17135_get_en(struct max17135 *p, enum max17135_en_id id)
{
	union max17135_enable enable;
	int ret;

	assert(p != NULL);

	if (i2cdev_read_reg8(p->i2c, MAX17135_REG_ENABLE, &enable.byte, 1))
		return -1;

	switch (id) {
	case MAX17135_EN_EN:      ret = enable.en     ? 1 : 0;    break;
	case MAX17135_EN_CEN:     ret = enable.cen    ? 1 : 0;    break;
	case MAX17135_EN_CEN2:    ret = enable.cen2   ? 1 : 0;    break;
	default:
		assert(!"invalid HV enable identifier");
		return -1;
	}

	return ret;
}

int max17135_get_fault(struct max17135 *p)
{
	union max17135_fault fault;

	assert(p != NULL);

	if (i2cdev_read_reg8(p->i2c, MAX17135_REG_FAULT, &fault.byte, 1))
		return -1;

	if (fault.fbpg)
		return MAX17135_FAULT_FBPG;

	if (fault.hvinp)
		return MAX17135_FAULT_HVINN;

	if (fault.hvinp)
		return MAX17135_FAULT_HVINP;

	if (fault.fbng)
		return MAX17135_FAULT_FBNG;

	if (fault.hvinpsc)
		return MAX17135_FAULT_HVINPSC;

	if (fault.hvinnsc)
		return MAX17135_FAULT_HVINNSC;

	if (fault.ot)
		return MAX17135_FAULT_OT;

	return MAX17135_FAULT_NONE;
}

/* ----------------------------------------------------------------------------
 * static functions
 */

static int read_const_registers(struct max17135 *p)
{
	if (i2cdev_read_reg8(p->i2c, MAX17135_REG_PROD_REV, &p->prod_rev, 1))
		return -1;

	if (i2cdev_read_reg8(p->i2c, MAX17135_REG_PROD_ID, &p->prod_id, 1))
		return -1;

	return 0;
}

static int read_timings(struct max17135 *p)
{
	char reg;
	char *timing;
	int ret;
	int i;

	if (p->flags.timings_read)
		return 0;

	reg = MAX17135_REG_TIMING_1;
	timing = p->timing;
	ret = 0;

	for (i = 0; (i < MAX17135_NB_TIMINGS) && !ret; ++i)
		ret = i2cdev_read_reg8(p->i2c, reg++, timing++, 1);

	if (!ret)
		p->flags.timings_read = 1;

	p->flags.timings_written = 0;

	return ret;
}

static int write_timings(struct max17135 *p)
{
	char reg = MAX17135_REG_TIMING_1;
	char *timing = p->timing;
	int ret = 0;
	int i;

	if (p->flags.timings_written)
		return 0;

	for (i = 0; (i < MAX17135_NB_TIMINGS) && !ret; ++i)
		ret = i2cdev_write_reg8(p->i2c, reg++, timing++, 1);

	if (!ret)
		p->flags.timings_written = 1;

	p->flags.timings_read = 0;

	return ret;
}

static int save_timings(struct max17135 *p)
{
#if MAX17135_ALLOW_SAVE
	int ret = write_timings(p);

	if (!ret) {
		union max17135_prog prog;

		prog.byte = 0;
		prog.timing = 1;

		ret = i2cdev_write_reg8(p->i2c, MAX17135_REG_PROG,
					&prog.byte,1);
	}

	return ret;
#else
	LOG("saving the timings is not allowed");
	return -1;
#endif
}
