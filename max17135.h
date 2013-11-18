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

#ifndef INCLUDE_MAX17135_H
#define INCLUDE_MAX17135_H 1

enum max17135_register {
	MAX17135_REG_EXT_TEMP   = 0x00,
	MAX17135_REG_CONF       = 0x01,
	MAX17135_REG_INT_TEMP   = 0x04,
	MAX17135_REG_TEMP_STAT  = 0x05,
	MAX17135_REG_PROD_REV   = 0x06,
	MAX17135_REG_PROD_ID    = 0x07,
	MAX17135_REG_DVR        = 0x08,
	MAX17135_REG_ENABLE     = 0x09,
	MAX17135_REG_FAULT      = 0x0A,
	MAX17135_REG_PROG       = 0x0C,
	MAX17135_REG_TIMING_1   = 0x10,
	MAX17135_REG_TIMING_2   = 0x11,
	MAX17135_REG_TIMING_3   = 0x12,
	MAX17135_REG_TIMING_4   = 0x13,
	MAX17135_REG_TIMING_5   = 0x14,
	MAX17135_REG_TIMING_6   = 0x15,
	MAX17135_REG_TIMING_7   = 0x16,
	MAX17135_REG_TIMING_8   = 0x17,
};

/* MAX17135_REG_CONF */
union max17135_conf {
	struct {
		char shutdown:1;
	};
	char byte;
};

/* MAX17135_REG_TEMP_STAT */
union max17135_temp_stat {
	struct {
		char busy:1;
		char open:1;
		char shrt:1;
		char reserved:5;
	};
	char byte;
};

/* MAX17135_REG_ENABLE */
union max17135_enable {
	struct {
		char en:1;
		char cen:1;
		char cen2:1;
		char reserved:5;
	};
	char byte;
};

/* MAX17135_REG_FAULT */
union max17135_fault {
	struct {
		char fbpg:1;
		char hvinp:1;
		char hvinn:1;
		char fbng:1;
		char hvinpsc:1;
		char hvinnsc:1;
		char ot:1;
		char pok:1;
	};
	char byte;
};

/* MAX17135_REG_PROG */
union max17135_prog {
	struct {
		char dvr:1;
		char timing:1;
	};
	char byte;
};

#endif /* INCLUDE_MAX17135_H */
