/*
  Plastic Logic hardware library - cpld

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

#ifndef INCLUDE_CPLD_H
#define INCLUDE_CPLD_H 1

#define CPLD_NB_BYTES 3

/* ----------------------------------------------------------------------------
   CPLD API v1
*/

/*
  RO = Read Only
  WO = Write Only
  RW = Read / Write
*/

/* Byte 0

   +---+----+---------------------+
   | 0 | RW | CPLD_HVEN
   +---+----+---------------------+
   | 1 | RW | BPCOM_CLAMP
   +---+----+---------------------+
   | 2 | RO | CPLD API version
   | 3 |    |
   | 4 |    |
   | 5 |    |
   | 6 |    |
   | 7 |    |
   +---+----+---------------------+

*/

struct cpld_byte_0 {
	char cpld_hven:1;
	char bpcom_clamp:1;
	char version:6;
};

/* Byte 1

   +---+----+---------------------+
   | 0 | RW | VCOM_SW_CLOSE
   +---+----+---------------------+
   | 1 | RW | VCOM_SW_EN
   +---+----+---------------------+
   | 2 | RW | VCOM_PSU_EN
   +---+----+---------------------+
   | 3 | RW | VGPOS_CLAMP
   +---+----+---------------------+
   | 4 | RO | CPLD build version
   | 5 |    |
   | 6 |    |
   | 7 |    |
   +---+--------------------------+

*/

struct cpld_byte_1 {
	char vcom_sw_close:1;
	char vcom_sw_en:1;
	char vcom_psu_en:1;
	char vgpos_clamp:1;
	char build_version:4;
};

/* Byte 2

   +---+----+---------------------+
   | 0 | RO | Board identifier
   | 1 |    |
   | 2 |    |
   | 3 |    |
   +---+----+---------------------+
   | 4 | RO | reserved
   | 5 |    |
   | 6 |    |
   | 7 |    |
   +---+----+---------------------+

*/

struct cpld_byte_2 {
	char board_id:4;
	char reserved:4;
};

#endif /* INCLUDE_CPLD_H */
