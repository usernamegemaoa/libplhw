/*
  Plastic Logic hardware library - tps65185

  Copyright (C) 2013 Plastic Logic Limited

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

#include "tps65185.h"
#include "i2cdev.h"
#include <libplhw.h>
#include <plsdk/plconfig.h>
#include <assert.h>
#include <unistd.h>

#define LOG_TAG "tps65185"
#include <plsdk/log.h>

struct tps65185 {
	struct i2cdev *i2c;
	struct plconfig *config;
	struct tps65185_version version;
};

struct regval {
	uint8_t reg;
	uint8_t val;
};

struct tps65185 *tps65185_init(const char *i2c_bus, char i2c_address)
{
	struct tps65185 *p;

	p = malloc(sizeof(struct tps65185));

	if (p == NULL)
		return NULL;

	p->config = plconfig_init(NULL, "libplhw");

	if (p->config == NULL)
		goto err_free_tps65185;

	if (i2c_address == PLHW_NO_I2C_ADDR)
		i2c_address = i2cdev_get_config_addr(
			p->config, "TPS65185-address", 0x68);

	p->i2c = i2cdev_init(i2c_bus, i2c_address);

	if (p->i2c == NULL) {
		LOG("failed to initialise I2C");
		goto err_free_plconfig;
	}

	if (i2cdev_read_reg8(p->i2c, TPS65185_REG_REV_ID, &p->version, 1)) {
		LOG("failed to read version register");
		goto err_free_i2cdev;
	}

	return p;

err_free_i2cdev:
	i2cdev_free(p->i2c);
err_free_plconfig:
	plconfig_free(p->config);
err_free_tps65185:
	free(p);

	return NULL;
}

void tps65185_free(struct tps65185 *p)
{
	assert(p != NULL);

	i2cdev_free(p->i2c);
	plconfig_free(p->config);
	free(p);
}

void tps65185_get_info(struct tps65185 *p, struct tps65185_info *info)
{
	assert(p != NULL);
	assert(info != NULL);

	info->version = p->version.version;
	info->major = p->version.major;
	info->minor = p->version.minor;
}

int tps65185_set_vcom(struct tps65185 *p, uint16_t value)
{
	const uint8_t val1 = value & 0xFF;
	uint8_t val2;

	assert(p != NULL);
	assert(value < 0x200);

	if (i2cdev_read_reg8(p->i2c, TPS65185_REG_VCOM2, &val2, 1)) {
		LOG("failed to read VCOM2 register");
		return -1;
	}

	val2 &= 0xFE;
	val2 |= (value >> 8) & 0x01;

	if (i2cdev_write_reg8(p->i2c, TPS65185_REG_VCOM1, &val1, 1) ||
	    i2cdev_write_reg8(p->i2c, TPS65185_REG_VCOM2, &val2, 1)) {
		LOG("failed to write to the VCOM registers");
		return -1;
	}

	return 0;
}

int tps65185_get_vcom(struct tps65185 *p, uint16_t *value)
{
	uint8_t val1;
	uint8_t val2;

	assert(p != NULL);
	assert(value != NULL);

	if (i2cdev_read_reg8(p->i2c, TPS65185_REG_VCOM1, &val1, 1) ||
	    i2cdev_read_reg8(p->i2c, TPS65185_REG_VCOM2, &val2, 1)) {
		LOG("failed to read the VCOM registers");
		return -1;
	}

	*value = ((val2 & 0x01) << 8) | val1;

	return 0;
}

int tps65185_set_seq(struct tps65185 *p, const struct tps65185_seq *seq,
		     int up)
{
	uint8_t reg_val;
	uint8_t reg_addr;

	assert(p != NULL);
	assert(seq != NULL);

	reg_val = seq->vddh;
	reg_val |= seq->vpos << 2;
	reg_val |= seq->vee << 4;
	reg_val |= seq->vneg << 6;

	reg_addr = up ? TPS65185_REG_UPSEQ0 : TPS65185_REG_DWNSEQ0;

	if (i2cdev_write_reg8(p->i2c, reg_addr, &reg_val, 1))
		return -1;

	reg_val = seq->strobe1;
	reg_val |= seq->strobe2 << 2;
	reg_val |= seq->strobe3 << 4;
	reg_val |= seq->strobe4 << 6;

	reg_addr = up ? TPS65185_REG_UPSEQ1 : TPS65185_REG_DWNSEQ1;

	if (i2cdev_write_reg8(p->i2c, reg_addr, &reg_val, 1))
		return -1;

	return 0;
}

int tps65185_get_seq(struct tps65185 *p, struct tps65185_seq *seq, int up)
{
	uint8_t reg_val;
	uint8_t reg_addr;

	assert(p != NULL);
	assert(seq != NULL);

	reg_addr = up ? TPS65185_REG_UPSEQ0 : TPS65185_REG_DWNSEQ0;

	if (i2cdev_read_reg8(p->i2c, reg_addr, &reg_val, 1))
		return -1;

	seq->vddh = reg_val & 0x3;
	seq->vpos = (reg_val >> 2) & 0x3;
	seq->vee = (reg_val >> 4) & 0x3;
	seq->vneg = (reg_val >> 6) & 0x3;

	reg_addr = up ? TPS65185_REG_UPSEQ1 : TPS65185_REG_DWNSEQ1;

	if (i2cdev_read_reg8(p->i2c, reg_addr, &reg_val, 1))
		return -1;

	seq->strobe1 = reg_val & 0x3;
	seq->strobe2 = (reg_val >> 2) & 0x3;
	seq->strobe3 = (reg_val >> 4) & 0x3;
	seq->strobe4 = (reg_val >> 6) & 0x3;

	return 0;
}

int tps65185_set_power(struct tps65185 *p, enum tps65185_power power)
{
	static const unsigned POLL_SLEEP_US = 5000;
	static const unsigned POLL_LOOPS = 100000 / POLL_SLEEP_US;
	uint8_t val;
	uint8_t flag;
	unsigned loop;

	assert(p != NULL);
	assert((power == TPS65185_ACTIVE) || (power == TPS65185_STANDBY));

	if (i2cdev_read_reg8(p->i2c, TPS65185_REG_ENABLE, &val, 1))
		return -1;

	flag = 1 << power;
	val |= flag;

	if (i2cdev_write_reg8(p->i2c, TPS65185_REG_ENABLE, &val, 1))
		return -1;

	loop = POLL_LOOPS;

	while (val & flag) {
		if (i2cdev_read_reg8(p->i2c, TPS65185_REG_ENABLE, &val, 1))
			return -1;

		if (!loop--) {
			LOG("TIMEOUT waiting for power transition");
			return -1;
		}

		usleep(10000);
	}

	return 0;
}

int tps65185_set_en(struct tps65185 *p, enum tps65185_en_id id, int on)
{
	uint8_t val;
	uint8_t flag;

	assert(p != NULL);
	assert((id >= 0) && (id < 6));

	if (i2cdev_read_reg8(p->i2c, TPS65185_REG_ENABLE, &val, 1))
		return -1;

	flag = 1 << id;

	if (on)
		val |= flag;
	else
		val &= ~flag;

	return i2cdev_write_reg8(p->i2c, TPS65185_REG_ENABLE, &val, 1);
}

int tps65185_get_en(struct tps65185 *p, enum tps65185_en_id id)
{
	uint8_t val;
	uint8_t flag;

	assert(p != NULL);
	assert((id >= 0) && (id < 6));

	if (i2cdev_read_reg8(p->i2c, TPS65185_REG_ENABLE, &val, 1))
		return -1;

	flag = 1 << id;

	return (val & flag) ? 1 : 0;
}

/* ----------------------------------------------------------------------------
 * static functions
 */

#if 0
		{ TPS65185_REG_ENABLE,     0x00 },
		{ TPS65185_REG_VADJ,       0x03 },
		{ TPS65185_REG_TMST1,      0x00 },
		{ TPS65185_REG_TMST2,      0x78 },
#endif
