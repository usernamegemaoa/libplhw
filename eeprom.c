/*
  Plastic Logic hardware library - eeprom

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

#include "i2cdev.h"
#include <libplhw.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#define LOG_TAG "eeprom"
#include "log.h"

#define WRITE_TIME_US 5000
#define DEFAULT_I2C_BLOCK_SIZE 96

struct eeprom_config {
	const char *mode;
	size_t data_size;
	size_t page_size;
	size_t offset_size;
};

struct eeprom {
	struct i2cdev *i2c;
	struct eeprom_config cfg;
	size_t offset;
	size_t block_size;
	uint8_t *packet;
	struct {
		char offset_written:1;
	} flags;
};

#define EEPROM_SIZE(n) (1024 * (1 << n) / 8)

static const struct eeprom_config eeprom_config_table[] = {
	{ "24c01",   EEPROM_SIZE(0),  16, 1 }, /*  128 Bytes */
	{ "24c02",   EEPROM_SIZE(1),  16, 1 }, /*  256 Bytes */
	{ "24c04",   EEPROM_SIZE(2),  16, 1 }, /*  512 Bytes */
	{ "24c08",   EEPROM_SIZE(3),  16, 1 }, /*   1K Bytes */
	{ "24c16",   EEPROM_SIZE(4),  16, 1 }, /*   2K Bytes */
	{ "24c32",   EEPROM_SIZE(5),  16, 2 }, /*   4K Bytes */
	{ "24c64",   EEPROM_SIZE(6),  16, 2 }, /*   8K Bytes */
	{ "24c128",  EEPROM_SIZE(7),  16, 2 }, /*  16K Bytes */
	{ "24c256",  EEPROM_SIZE(8),  64, 2 }, /*  32K Bytes */
	{ "24c512",  EEPROM_SIZE(9),  64, 2 }, /*  64K Bytes */
	{ "24c1024", EEPROM_SIZE(10), 64, 2 }, /* 128K Bytes */
	{ NULL, 0, 0, 0 }
};

static void set_offset(struct eeprom *e);
static int sync_offset(struct eeprom *e);
static int write_page(struct eeprom *e, const char *data, size_t size);

struct eeprom *eeprom_init(const char *i2c_bus, char i2c_address,
			   const char *mode)
{
	const struct eeprom_config *config;
	struct eeprom *e;

	assert(mode != NULL);

	if (i2c_address == PLHW_NO_I2C_ADDR) {
		LOG("no I2C address specified");
		return NULL;
	}

	e = malloc(sizeof (struct eeprom));

	if (e == NULL)
		return NULL;

	for (config = eeprom_config_table; config->mode != NULL; ++config) {
		if (!strcmp(mode, config->mode)) {
			memcpy(&e->cfg, config, sizeof e->cfg);
			break;
		}
	}

	if (config->mode == NULL) {
		LOG("unsupported mode: %s", mode);
		goto err_free_e;
	}

	LOG("mode: %s, data_size: %zu, page_size: %zu, offset_size: %zu",
	    e->cfg.mode, e->cfg.data_size, e->cfg.page_size,
	    e->cfg.offset_size);

	e->offset = 0;
	e->block_size = DEFAULT_I2C_BLOCK_SIZE;
	e->packet = malloc(e->cfg.page_size + e->cfg.offset_size);
	assert(e->packet != NULL);

	e->i2c = i2cdev_init(i2c_bus, i2c_address);

	if (e->i2c == NULL)
		goto err_free_packet;

	if (sync_offset(e) < 0) {
		LOG("failed to set the initial offset");
		goto err_free_i2c;
	}

	return e;

err_free_i2c:
	i2cdev_free(e->i2c);
err_free_packet:
	free(e->packet);
err_free_e:
	free(e);

	return NULL;
}

void eeprom_free(struct eeprom *e)
{
	assert(e != NULL);

	i2cdev_free(e->i2c);
	free(e->packet);
	free(e);
}

const char *eeprom_get_mode(struct eeprom *e)
{
	assert(e != NULL);

	return e->cfg.mode;
}

size_t eeprom_get_size(struct eeprom *e)
{
	assert(e != NULL);

	return e->cfg.data_size;
}

void eeprom_set_page_size(struct eeprom *e, size_t page_size)
{
	assert(e != NULL);

	e->cfg.page_size = page_size;
}

size_t eeprom_get_page_size(struct eeprom *e)
{
	assert(e != NULL);

	return e->cfg.page_size;
}

void eeprom_set_block_size(struct eeprom *e, size_t block_size)
{
	assert(e != NULL);
	assert(block_size != 0);

	e->block_size = block_size;
}

size_t eeprom_get_block_size(struct eeprom *e)
{
	assert(e != NULL);

	return e->block_size;
}

void eeprom_seek(struct eeprom *e, size_t offset)
{
	assert(e != NULL);
	assert(offset < e->cfg.data_size);

	e->offset = offset;
	e->flags.offset_written = 0;
}

size_t eeprom_get_offset(struct eeprom *e)
{
	assert(e != NULL);

	return e->offset;
}

int eeprom_read(struct eeprom *e, char *data, size_t size)
{
	size_t read_size;
	int blocks;
	int overrun;
	int block;
	char *p;

	assert(e != NULL);

	if (e->offset == INVALID_OFFSET)
		return -1;

	if (sync_offset(e) < 0)
		return -1;

	overrun = e->offset + size - e->cfg.data_size;

	if (overrun > 0)
		read_size = size - overrun;
	else
		read_size = size;

	if (!read_size)
		return 0;

	blocks = read_size / e->block_size;

	if (read_size % e->block_size)
		++blocks;

	p = data;

	for (block = 0; block < blocks; ++block) {
		const size_t b_size = min(read_size, e->block_size);

		if (i2cdev_read(e->i2c, p, b_size) < 0) {
			e->offset = INVALID_OFFSET;
			return -1;
		}

		read_size -= b_size;
		p += b_size;
	}

	e->offset += size;

	return 0;
}

int eeprom_write(struct eeprom *e, const char *data, size_t size)
{
	unsigned page_offset;
	unsigned first_len;
	unsigned n_pages;
	unsigned page;
	unsigned last_len;
	const char *p;

	assert(e != NULL);
	assert(data != NULL);

	page_offset = e->offset % e->cfg.page_size;

	if ((page_offset + size) < e->cfg.page_size) {
		first_len = size;
		n_pages = 0;
		last_len = 0;
	} else {
		if (page_offset == 0)
			first_len = 0;
		else
			first_len = e->cfg.page_size - page_offset;

		last_len = (e->offset + size) % e->cfg.page_size;
		n_pages = (size - first_len - last_len) / e->cfg.page_size;
	}

	p = data;

	if (first_len) {
		if (write_page(e, p, first_len) < 0)
			return -1;

		p += first_len;
	}

	for (page = 0; page < n_pages; ++page) {
		if (write_page(e, p, e->cfg.page_size) < 0)
			return -1;

		p += e->cfg.page_size;
	}

	if (last_len) {
		if (write_page(e, p, last_len) < 0)
			return -1;
	}

	return 0;
}

/* ----------------------------------------------------------------------------
 * static functions
 */

static void set_offset(struct eeprom *e)
{
	if (e->cfg.offset_size == 1) {
		e->packet[0] = e->offset;
	} else {
		e->packet[0] = (e->offset >> 8) & 0x7F;
		e->packet[1] = e->offset & 0xFF;
	}
}

static int sync_offset(struct eeprom *e)
{
	if (e->flags.offset_written)
		return 0;

	set_offset(e);

	if (i2cdev_write(e->i2c, e->packet, e->cfg.offset_size) < 0) {
		e->offset = INVALID_OFFSET;
		return -1;
	}

	e->flags.offset_written = 1;

	return 0;
}

static int write_page(struct eeprom *e, const char *data, size_t size)
{
	assert(size <= e->cfg.page_size);

	set_offset(e);
	memcpy(&e->packet[e->cfg.offset_size], data, size);

	if (i2cdev_write(e->i2c, e->packet, (size + e->cfg.offset_size)) < 0)
		return -1;

	usleep(WRITE_TIME_US);
	e->offset += size;

	return 0;
}
