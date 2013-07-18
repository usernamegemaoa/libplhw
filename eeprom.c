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
#include "libplhw.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>

#define LOG_TAG "eeprom"
#include "log.h"

#define ADDR_RANGE (32 * 1024)
#define PAGE_LENGTH 64
#define WRITE_TIME_US 5000
#define DEFAULT_I2C_BLOCK_SIZE 96

struct eeprom {
	struct i2cdev *i2c;
	size_t offset;
	size_t block_size;
	struct {
		char offset_written:1;
	} flags;
};

static int sync_offset(struct eeprom *e);
static int write_page(struct eeprom *e, const char *data, size_t size);

struct eeprom *eeprom_init(const char *i2c_bus, char i2c_address)
{
	struct eeprom *e;
	int error = 1;

	e = malloc(sizeof (struct eeprom));
	assert(e != NULL);

	e->offset = 0;
	e->block_size = DEFAULT_I2C_BLOCK_SIZE;
	e->i2c = i2cdev_init(i2c_bus, i2c_address);

	if (e->i2c == NULL)
		LOG("failed to initialise I2C");
	else if (sync_offset(e) < 0)
		LOG("failed to set the initial offset");
	else
		error = 0;

	if (error) {
		eeprom_free(e);
		e = NULL;
	}

	return e;
}

void eeprom_free(struct eeprom *e)
{
	assert(e != NULL);

	if (e->i2c != NULL)
		i2cdev_free(e->i2c);

	free(e);
}

size_t eeprom_get_size(struct eeprom *e)
{
	assert(e != NULL);

	return ADDR_RANGE;
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
	assert(offset < ADDR_RANGE);

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

	overrun = e->offset + size - ADDR_RANGE;

	if (overrun > 0)
		read_size = size - overrun;
	else
		read_size = size;

	if (!size)
		return 0;

	blocks = read_size / e->block_size;

	if (read_size % e->block_size)
		++blocks;

	p = data;

	for (block = 0; block < blocks; ++block) {
		const size_t b_size =
			(read_size < e->block_size)
			? read_size : e->block_size;

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

	page_offset = e->offset % PAGE_LENGTH;

	if ((page_offset + size) < PAGE_LENGTH) {
		first_len = size;
		n_pages = 0;
		last_len = 0;
	} else {
		if (page_offset == 0)
			first_len = 0;
		else
			first_len = PAGE_LENGTH - page_offset;

		last_len = (e->offset + size) % PAGE_LENGTH;
		n_pages = (size - first_len - last_len) / PAGE_LENGTH;
	}

	p = data;

	if (first_len) {
		if (write_page(e, p, first_len) < 0)
			return -1;

		p += first_len;
	}

	for (page = 0; page < n_pages; ++page) {
		if (write_page(e, p, PAGE_LENGTH) < 0)
			return -1;

		p += PAGE_LENGTH;
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

static int sync_offset(struct eeprom *e)
{
	char offset[2];

	if (e->flags.offset_written)
		return 0;

	offset[0] = (e->offset >> 8) & 0x7F;
	offset[1] = e->offset & 0xFF;

	if (i2cdev_write(e->i2c, offset, 2) < 0) {
		e->offset = INVALID_OFFSET;
		return -1;
	}

	e->flags.offset_written = 1;

	return 0;
}

static int write_page(struct eeprom *e, const char *data, size_t size)
{
	struct {
		char offset[2];
		char page[PAGE_LENGTH];
	} packet;

	assert(size <= PAGE_LENGTH);

	packet.offset[0] = (e->offset >> 8) & 0x7F;
	packet.offset[1] = e->offset & 0xFF;
	memcpy(&packet.page, data, size);

	if (i2cdev_write(e->i2c, &packet, size + 2) < 0)
		return -1;

	usleep(WRITE_TIME_US);
	e->offset += size;

	return 0;
}
