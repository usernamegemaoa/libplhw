/*
  Plastic Logic hardware library - i2cdev

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
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef ANDROID /* Bionic libc */
struct i2c_rdwr_ioctl_data {
	struct i2c_msg *msgs;
	__u32 nmsgs;
};
#else /* GNU libc */
# include <linux/i2c-dev.h>
#endif

#define LOG_TAG "i2cdev"
#include "log.h"

#define BLOCK_SIZE_STEP 64

struct i2cdev {
	int fd;
	char addr;
	struct {
		uint8_t verbose_log:1;
		uint8_t ignore_read_nak:1;
		uint8_t ignore_write_nak:1;
	} flags;
	uint8_t *block;
	size_t block_size;
};

static int rdwr_data(struct i2cdev *d, __u16 flags, void *data, size_t size);
static int read_reg_data(struct i2cdev *d, uint8_t *reg, size_t reg_sz,
			 void *buffer, size_t buffer_sz);
static int write_reg_data(struct i2cdev *d, uint8_t *reg, size_t reg_sz,
			  const void *buf, size_t buf_sz);
static void print_data(const void *data, size_t size);
static void print_reg_io(const char *label, uint8_t addr, const uint8_t *reg,
			 size_t reg_sz, const void *buf, size_t buf_sz,
			 int local_errno);

struct i2cdev *i2cdev_init(const char *bus_device, char address)
{
	struct i2cdev *d;
	int error = 1;

	assert(bus_device != NULL);
	assert(!(address & 0x80));

	d = malloc(sizeof (struct i2cdev));
	assert(d != NULL);

	d->addr = address;
	d->fd = open(bus_device, O_RDWR);

	if (d->fd < 0)
		LOG("failed to open I2C bus device (%s)", bus_device);
	else if (ioctl(d->fd, I2C_SLAVE, d->addr) < 0)
		LOG("failed to set I2C address (0x%02X)", d->addr);
	else
		error = 0;

	if (error) {
		i2cdev_free(d);
		d = NULL;
	} else {
		d->flags.verbose_log = 0;
		d->flags.ignore_read_nak = 0;
		d->flags.ignore_write_nak = 0;
		d->block = NULL;
		d->block_size = 0;
	}

	return d;
}

void i2cdev_free(struct i2cdev *d)
{
	assert(d != NULL);

	if (d->fd >= 0)
		close(d->fd);

	if (d->block != NULL)
		free(d->block);

	free(d);
}

void i2cdev_set_flag(struct i2cdev *d, enum i2cdev_flag f, int enable)
{
	const int on = enable ? 1 : 0;

	assert(d != NULL);

	switch (f) {
	case I2CDEV_IGNORE_READ_NAK:   d->flags.ignore_read_nak = on;   break;
	case I2CDEV_IGNORE_WRITE_NAK:  d->flags.ignore_write_nak = on;  break;
	case I2CDEV_VERBOSE_LOG:       d->flags.verbose_log = on;       break;
	default: assert(!"Invalid flag id"); break;
	}
}

int i2cdev_read(struct i2cdev *d, void *data, size_t size)
{
        __u16 i2c_flags = I2C_M_RD;

	assert(d != NULL);
	assert(data != NULL);

	if (d->flags.ignore_read_nak)
		i2c_flags |= I2C_M_IGNORE_NAK;

	return rdwr_data(d, i2c_flags, data, size);
}

int i2cdev_write(struct i2cdev *d, const void *data, size_t size)
{
	__u16 i2c_flags = 0;

	assert(d != NULL);
	assert(data != NULL);

	if (d->flags.ignore_write_nak)
		i2c_flags |= I2C_M_IGNORE_NAK;

	return rdwr_data(d, i2c_flags, (void *) data, size);
}

int i2cdev_read_reg(struct i2cdev *d, const void *reg, size_t reg_sz,
		    void *data, size_t data_sz)
{
	assert(d != NULL);
	assert(reg != NULL);
	assert(data != NULL);

	return read_reg_data(d, (uint8_t *) reg, reg_sz, data, data_sz);
}

int i2cdev_read_reg8(struct i2cdev *d, char reg, void *data, size_t sz)
{
	assert(d != NULL);
	assert(data != NULL);

	return read_reg_data(d, (uint8_t *) &reg, 1, data, sz);
}

int i2cdev_write_reg(struct i2cdev *d, const void *reg, size_t reg_sz,
		     const void *data, size_t data_sz)
{
	assert(d != NULL);
	assert(reg != NULL);
	assert(data != NULL);

	return write_reg_data(d, (uint8_t *) reg, reg_sz, data, data_sz);
}

int i2cdev_write_reg8(struct i2cdev *d, char reg, const void *data, size_t sz)
{
	assert(d != NULL);
	assert(data != NULL);

	return write_reg_data(d, (uint8_t *) &reg, 1, data, sz);
}

/* ----------------------------------------------------------------------------
 * static functions
 */

static int rdwr_data(struct i2cdev *d, __u16 flags, void *data, size_t size)
{
	struct i2c_msg msgs[1] = {
		{
			.addr = d->addr,
			.flags = flags,
			.len = size,
			.buf = (__u8 *) data
		}
	};

	struct i2c_rdwr_ioctl_data i2c_data = {
		.msgs = msgs,
		.nmsgs = 1
	};

	int ret = (ioctl(d->fd, I2C_RDWR, &i2c_data) < 0) ? -1 : 0;

	if (ret || d->flags.verbose_log) {
		if (ret)
			ret = -errno;

		LOG_N("%s (addr: 0x%02X, size: %zu:",
		      flags & I2C_M_RD ? "read" : "write", d->addr, size);
		print_data(data, size);
		LOG_PRINT(") -> %s\n", ret ? strerror(-ret) : "OK");
	}

	return ret;
}

static int read_reg_data(struct i2cdev *d, uint8_t *reg, size_t reg_sz,
			 void *buf, size_t buf_sz)
{
	struct i2c_msg msgs[2] = {
		{
			.addr = d->addr,
			.flags = 0,
			.len = reg_sz,
			.buf = reg
		},
		{
			.addr = d->addr,
			.flags = I2C_M_RD
			 | (d->flags.ignore_read_nak ? I2C_M_IGNORE_NAK : 0),
			.len = buf_sz,
			.buf = (__u8 *) buf
		}
	};

	struct i2c_rdwr_ioctl_data i2c_data = {
		.msgs = msgs,
		.nmsgs = 2
	};

	int ret = (ioctl(d->fd, I2C_RDWR, &i2c_data) < 0) ? -1 : 0;

	if (ret || d->flags.verbose_log) {
		if (ret)
			ret = -errno;

		print_reg_io("read reg data", d->addr, reg, reg_sz, buf,
			     buf_sz, -ret);
	}

	return ret;
}

static int write_reg_data(struct i2cdev *d, uint8_t *reg, size_t reg_sz,
			  const void *buf, size_t buf_sz)
{
	const size_t w_size = buf_sz + reg_sz;

	struct i2c_msg msg = {
		.addr = d->addr,
		.flags = d->flags.ignore_write_nak ? I2C_M_IGNORE_NAK : 0,
		.len = w_size,
	};

	struct i2c_rdwr_ioctl_data i2c_data = {
		.msgs = &msg,
		.nmsgs = 1
	};

	size_t block_size;
	int ret;

	block_size = w_size;

	if (!(block_size % BLOCK_SIZE_STEP))
		--block_size;

	block_size /= BLOCK_SIZE_STEP;
	++block_size;
	block_size *= BLOCK_SIZE_STEP;

	if (d->block == NULL)
		d->block = malloc(block_size);
	else if (d->block_size < block_size)
		d->block = realloc(d->block, block_size);

	if (d->block == NULL)
		return -1;

	d->block_size = block_size;
	memcpy(d->block, reg, reg_sz);
	memcpy(d->block + reg_sz, buf, buf_sz);
	msg.buf = (__u8 *) d->block;

	ret = (ioctl(d->fd, I2C_RDWR, &i2c_data) < 0) ? -1 : 0;

	if (ret || d->flags.verbose_log) {
		if (ret)
			ret = -errno;

		print_reg_io("write reg data", d->addr, reg, reg_sz, buf,
			     buf_sz, -ret);
	}

	return ret;
}

static void print_data(const void *data, size_t size)
{
	static const size_t MAX_DUMP = 8;
	const unsigned n = (size < MAX_DUMP) ? size : MAX_DUMP;
	const uint8_t * const data8 = (const uint8_t *) data;
	const uint8_t * const end = &data8[n];
	const uint8_t *it = &data8[0];

	while (it != end)
		LOG_PRINT(" %02X", *it++);
}

static void print_reg_io(const char *label, uint8_t addr, const uint8_t *reg,
			 size_t reg_sz, const void *buf, size_t buf_sz,
			 int local_errno)
{
	LOG_N("%s (addr: 0x%02X, reg:", label, addr);
	print_data(reg, reg_sz);
	LOG_PRINT(", size: %zu:", buf_sz);
	print_data(buf, buf_sz);
	LOG_PRINT(") -> %s\n", local_errno ? strerror(local_errno) : "OK");
}
