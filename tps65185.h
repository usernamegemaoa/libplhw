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

#ifndef INCLUDE_TPS65185_H
#define INCLUDE_TPS65185_H 1

#include <stdint.h>

enum tps65185_register {
	TPS65185_REG_TMST_VALUE = 0x00,
	TPS65185_REG_ENABLE     = 0x01,
	TPS65185_REG_VADJ       = 0x02,
	TPS65185_REG_VCOM1      = 0x03,
	TPS65185_REG_VCOM2      = 0x04,
	TPS65185_REG_INT_EN1    = 0x05,
	TPS65185_REG_INT_EN2    = 0x06,
	TPS65185_REG_INT1       = 0x07,
	TPS65185_REG_INT2       = 0x08,
	TPS65185_REG_UPSEQ0     = 0x09,
	TPS65185_REG_UPSEQ1     = 0x0A,
	TPS65185_REG_DWNSEQ0    = 0x0B,
	TPS65185_REG_DWNSEQ1    = 0x0C,
	TPS65185_REG_TMST1      = 0x0D,
	TPS65185_REG_TMST2      = 0x0E,
	TPS65185_REG_PG_STAT    = 0x0F,
	TPS65185_REG_REV_ID     = 0x10,
};

struct tps65185_version {
	uint8_t version:4;
	uint8_t minor:2;
	uint8_t major:2;
};

#endif /* INCLUDE_TPS65185_H */
