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

#ifndef INCLUDE_DAC5820_H
#define INCLUDE_DAC5820_H 1

/*
 * A and B are the names of the 2 channels
 * IN is the input register (one per channel)
 * DAC is the dac register used on the output (one per channel)
 * LOAD means the I2C data is copied to IN
 * UP means the IN is copied to DAC
 * OUT means the contents of DAC is used on the output
 */

/* write commands (4 bits in dac5820_byte_write_cmd) */
enum dac5820_write_cmd_id {
	DAC5820_CMD_LOAD_IN_DAC_A__UP_DAC_B__OUT_AB = 0x0,
	DAC5820_CMD_LOAD_IN_DAC_B__UP_DAC_A__OUT_AB = 0x1,
	DAC5820_CMD_LOAD_IN_A                       = 0x4,
	DAC5820_CMD_LOAD_IN_B                       = 0x5,
	DAC5820_CMD_UP_DAC_AB__OUT_AB__LOAD_IN_A    = 0x8,
	DAC5820_CMD_UP_DAC_AB__OUT_AB__LOAD_IN_B    = 0x9,
	DAC5820_CMD_LOAD_IN_DAC_AB__OUT_AB          = 0xC,
	DAC5820_CMD_LOAD_IN_AB                      = 0xD,
	DAC5820_CMD_OUT_AB__NO_DATA                 = 0xE,
	DAC5820_CMD_EXT__DATA_0                     = 0xF,
};

enum dac5820_pd {
	DAC5820_PD_ON         = 0x0,
	DAC5820_PD_OFF_FLOAT  = 0x1,
	DAC5820_PD_OFF_1K     = 0x2,
	DAC5820_PD_OFF_100K   = 0x3
};

struct dac5820_byte_write_cmd {
	char data_high:4;
	char cmd:4;
};

struct dac5820_byte_cmd_ext {
	char pd:2;
	char a:1;
	char b:1;
	char reserved:4;
};

struct dac5820_byte_write_data {
	char reserved:4;
	char data_low:4;
};

union dac5820_write_payload {
	struct {
		struct dac5820_byte_write_cmd cmd_byte;
		struct dac5820_byte_write_data data_byte;
	};
	char bytes[2];
};

union dac5820_ext_payload {
	struct {
		struct dac5820_byte_write_cmd cmd_byte;
		struct dac5820_byte_cmd_ext ext_byte;
	};
	char bytes[2];
};

/* read commands (8 bits) */
enum dac5820_read_cmd_id {
	DAC5820_CMD_READ_A = 0xF1,
	DAC5820_CMD_READ_B = 0xF2,
};

struct dac5820_byte_read_cmd {
	char data:4;
	char pd:2;
	char reserved:2;
};

#endif /* INCLUDE_DAC5820_H */
