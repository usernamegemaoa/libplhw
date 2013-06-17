/*
  Plastic Logic hardware library - hvpmic

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

#include "hvpmic.h"
#include "libplhw.h"
#include "i2cdev.h"
#include <assert.h>
#include <unistd.h>

#define HVPMIC_ALLOW_SAVE 0

#define LOG_TAG "hvpmic"
#include "log.h"

struct hvpmic {
	struct i2cdev *i2c;
	char prod_id;
	char prod_rev;
	char timing[HVPMIC_NB_TIMINGS];
	struct {
		unsigned timings_read:1;
		unsigned timings_written:1;
	} flags;
	unsigned pok_delay_us;
};

static int read_const_registers(struct hvpmic *p);
static int read_timings(struct hvpmic *p);
static int write_timings(struct hvpmic *p);
static int save_timings(struct hvpmic *p);

struct hvpmic *hvpmic_init(const char *i2c_bus, char i2c_address)
{
	struct hvpmic *p;
	int error = 1;

	p = malloc(sizeof (struct hvpmic));
	assert(p != NULL);

	p->i2c = i2cdev_init(i2c_bus, i2c_address);

	if (p->i2c == NULL)
		LOG("failed to initialise I2C");
	else if (read_const_registers(p) < 0)
		LOG("failed to read registers");
	else
		error = 0;

	if (error) {
		hvpmic_free(p);
		p = NULL;
	} else {
		p->flags.timings_read = 0;
		p->flags.timings_written = 0;
		p->pok_delay_us = 10000;
	}

	return p;
}

void hvpmic_free(struct hvpmic *p)
{
	assert(p != NULL);

	if (p->i2c != NULL)
		i2cdev_free(p->i2c);

	free (p);
}

int hvpmic_get_prod_id(struct hvpmic *p)
{
	assert(p != NULL);

	return p->prod_id;
}

int hvpmic_get_prod_rev(struct hvpmic *p)
{
	assert(p != NULL);

	return p->prod_rev;
}

int hvpmic_get_vcom(struct hvpmic *p, char *dvr)
{
	assert(p != NULL);
	assert(dvr != NULL);

	if (i2cdev_read_reg8(p->i2c, HVPMIC_REG_DVR, dvr, 1) < 0)
		return -1;

	return 0;
}

int hvpmic_set_vcom(struct hvpmic *p, char value)
{
	assert(p != NULL);

	return i2cdev_write_reg8(p->i2c, HVPMIC_REG_DVR, &value, 1);
}

int hvpmic_save_vcom(struct hvpmic *p)
{
#if HVPMIC_ALLOW_SAVE
	union hvpmic_prog prog;

	assert(p != NULL);

	prog.byte = 0;
	prog.dvr = 1;

	return i2cdev_write_reg8(p->i2c, HVPMIC_REG_PROG, &prog.byte, 1);
#else
	LOG("writing the VCOM value is not allowed");
	return -1;
#endif
}

int hvpmic_get_timing(struct hvpmic *p, unsigned n)
{
	assert(p != NULL);
	assert(n < HVPMIC_NB_TIMINGS);

	if (read_timings(p) < 0)
		return -1;

	return p->timing[n];
}

int hvpmic_get_timings(struct hvpmic *p, char *data, size_t size)
{
	const int rd_size =
		(size < HVPMIC_NB_TIMINGS) ? size : HVPMIC_NB_TIMINGS;
	const char *in;
	char *out;
	int i;

	assert(p != NULL);
	assert(data != NULL);

	if (read_timings(p) < 0)
		return -1;

	in = p->timing;
	out = data;

	for (i = 0; i < rd_size; ++i)
		*out++ = *in++;

	return i;
}

int hvpmic_set_timing(struct hvpmic *p, unsigned n, char value)
{
	assert(p != NULL);
	assert(n < HVPMIC_NB_TIMINGS);

	if (read_timings(p) < 0)
		return -1;

	p->timing[n] = value;

	return write_timings(p);
}

int hvpmic_set_timings(struct hvpmic *p, const char *data, size_t size)
{
	const size_t wr_size =
		(size < HVPMIC_NB_TIMINGS) ? size : HVPMIC_NB_TIMINGS;
	const char *in;
	char *out;
	unsigned i;

	assert(p != NULL);

	if (read_timings(p) < 0)
		return -1;

	in = data;
	out = p->timing;

	for (i = 0; i < wr_size; ++i)
		*out++ = *in++;

	return write_timings(p);
}

int hvpmic_save_timings(struct hvpmic *p)
{
	assert(p != NULL);

	if (!p->flags.timings_written)
		return 0;

	return save_timings(p);
}

int hvpmic_get_temp_sensor_en(struct hvpmic *p)
{
	union hvpmic_conf conf;
	int ret;

	assert(p != NULL);

	ret = i2cdev_read_reg8(p->i2c, HVPMIC_REG_CONF, &conf.byte, 1);

	if (!ret)
		ret = conf.shutdown ? 0 : 1;

	return ret;
}

int hvpmic_set_temp_sensor_en(struct hvpmic *p, int en)
{
	union hvpmic_conf conf;
	int ret;

	assert(p != NULL);

	ret = i2cdev_read_reg8(p->i2c, HVPMIC_REG_CONF, &conf.byte, 1);

	if (!ret) {
		conf.shutdown = en ? 0 : 1;
		ret = i2cdev_write_reg8(p->i2c, HVPMIC_REG_CONF, &conf.byte,1);
	}

	return ret;
}

int hvpmic_get_temperature(struct hvpmic *p, short *temp,
                           enum hvpmic_temp_id id)
{
	char reg;
	char value[2];
	int ret = 0;

	assert(p != NULL);
	assert(temp != NULL);

	switch (id) {
	case HVPMIC_TEMP_EXT:
		reg = HVPMIC_REG_EXT_TEMP;
		break;

	case HVPMIC_TEMP_INT:
		reg = HVPMIC_REG_INT_TEMP;
		break;

	default:
		assert(!"invalid temperature identifier");
		ret = -1;
		break;
	}

	if (ret < 0)
		return -1;

	if (i2cdev_read_reg8(p->i2c, reg, value, 2) < 0)
		return -1;

	*temp = (value[0] << 8) | (value[1]);

	return 0;
}

int hvpmic_get_temp_failure(struct hvpmic *p)
{
	union hvpmic_temp_stat stat;

	assert(p != NULL);

	if (i2cdev_read_reg8(p->i2c, HVPMIC_REG_TEMP_STAT, &stat.byte, 1) < 0)
		return -1;

	if (stat.open)
		return HVPMIC_TEMP_OPEN;

	if (stat.shrt)
		return HVPMIC_TEMP_SHORT;

	return HVPMIC_TEMP_OK;
}

float hvpmic_convert_temperature(struct hvpmic *p, short temp)
{
	assert(p != NULL);

	return (float) (((temp >> 7) & 0x1FF)) / 2;
}

int hvpmic_get_pok(struct hvpmic *p)
{
	union hvpmic_fault fault;

	assert(p != NULL);

	if (i2cdev_read_reg8(p->i2c, HVPMIC_REG_FAULT, &fault.byte, 1) < 0)
		return -1;

	return fault.pok;
}

void hvpmic_set_pok_delay(struct hvpmic *p, unsigned delay_us)
{
	assert(p != NULL);

	p->pok_delay_us = delay_us;
}

int hvpmic_wait_for_pok(struct hvpmic *p)
{
	static const int POLL_SLEEP_US = 5000;
	static const int POLL_LOOPS = 1000000 / POLL_SLEEP_US;
	int pok;
	int i;

	assert(p != NULL);

	usleep(p->pok_delay_us);

	for (i = POLL_LOOPS, pok = 0; i && !pok; --i) {
		pok = hvpmic_get_pok(p);

		if (!pok < 0)
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

int hvpmic_set_en(struct hvpmic *p, enum hvpmic_en_id id, int on)
{
	union hvpmic_enable enable;
	int ret = 0;

	assert(p != NULL);

	if (i2cdev_read_reg8(p->i2c, HVPMIC_REG_ENABLE, &enable.byte, 1) < 0)
		return -1;

	switch (id) {
	case HVPMIC_EN_EN:      enable.en     = on ? 1 : 0;    break;
	case HVPMIC_EN_CEN:     enable.cen    = on ? 1 : 0;    break;
	case HVPMIC_EN_CEN2:    enable.cen2   = on ? 1 : 0;    break;
	default:
		assert(!"invalid HV enable identifier");
		ret = -1;
		break;
	}

	return i2cdev_write_reg8(p->i2c, HVPMIC_REG_ENABLE, &enable.byte, 1);
}

int hvpmic_get_en(struct hvpmic *p, enum hvpmic_en_id id)
{
	union hvpmic_enable enable;
	int ret;

	assert(p != NULL);

	if (i2cdev_read_reg8(p->i2c, HVPMIC_REG_ENABLE, &enable.byte, 1) < 0)
		return -1;

	switch (id) {
	case HVPMIC_EN_EN:      ret = enable.en     ? 1 : 0;    break;
	case HVPMIC_EN_CEN:     ret = enable.cen    ? 1 : 0;    break;
	case HVPMIC_EN_CEN2:    ret = enable.cen2   ? 1 : 0;    break;
	default:
		assert(!"invalid HV enable identifier");
		ret = -1;
		break;
	}

	return ret;
}

int hvpmic_get_fault(struct hvpmic *p)
{
	union hvpmic_fault fault;

	assert(p != NULL);

	if (i2cdev_read_reg8(p->i2c, HVPMIC_REG_FAULT, &fault.byte, 1) < 0)
		return -1;

	if (fault.fbpg)
		return HVPMIC_FAULT_FPBG;

	if (fault.hvinp)
		return HVPMIC_FAULT_HVINN;

	if (fault.hvinp)
		return HVPMIC_FAULT_HVINP;

	if (fault.fbng)
		return HVPMIC_FAULT_FBNG;

	if (fault.hvinpsc)
		return HVPMIC_FAULT_HVINPSC;

	if (fault.hvinnsc)
		return HVPMIC_FAULT_HVINNSC;

	if (fault.ot)
		return HVPMIC_FAULT_OT;

	return HVPMIC_FAULT_NONE;
}

/* ----------------------------------------------------------------------------
 * static functions
 */

static int read_const_registers(struct hvpmic *p)
{
	if (i2cdev_read_reg8(p->i2c, HVPMIC_REG_PROD_REV, &p->prod_rev, 1) < 0)
		return -1;

	if (i2cdev_read_reg8(p->i2c, HVPMIC_REG_PROD_ID, &p->prod_id, 1) < 0)
		return -1;

	return 0;
}

static int read_timings(struct hvpmic *p)
{
	char reg;
	char *timing;
	int ret;
	int i;

	if (p->flags.timings_read)
		return 0;

	reg = HVPMIC_REG_TIMING_1;
	timing = p->timing;
	ret = 0;

	for (i = 0; (i < HVPMIC_NB_TIMINGS) && !ret; ++i)
		ret = i2cdev_read_reg8(p->i2c, reg++, timing++, 1);

	if (!ret)
		p->flags.timings_read = 1;

	p->flags.timings_written = 0;

	return ret;
}

static int write_timings(struct hvpmic *p)
{
	char reg = HVPMIC_REG_TIMING_1;
	char *timing = p->timing;
	int ret = 0;
	int i;

	if (p->flags.timings_written)
		return 0;

	for (i = 0; (i < HVPMIC_NB_TIMINGS) && !ret; ++i)
		ret = i2cdev_write_reg8(p->i2c, reg++, timing++, 1);

	if (!ret)
		p->flags.timings_written = 1;

	p->flags.timings_read = 0;

	return ret;
}

static int save_timings(struct hvpmic *p)
{
#if HVPMIC_ALLOW_SAVE
	int ret = write_timings(p);

	if (!ret) {
		union hvpmic_prog prog;

		prog.byte = 0;
		prog.timing = 1;

		ret = i2cdev_write_reg8(p->i2c, HVPMIC_REG_PROG, &prog.byte,1);
	}

	return ret;
#else
	LOG("saving the timings is not allowed");
	return -1;
#endif
}
