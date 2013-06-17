/*
  Plastic Logic hardware library - gpio_signals

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

#ifndef INCLUDE_GPIO_SIGNALS
#define INCLUDE_GPIO_SIGNALS 1

/* ---------------------------------------------------------------------------
 * CPLD JTAG
 */

#define GPIO_CPLDJ_I_MASK 0x80
#define GPIO_CPLDJ_O_MASK 0x70

union gpio_cpldj {
	struct {
		char reserved:4;
		char tck:1;
		char tms:1;
		char tdi:1;
		char tdo:1;
	};
	char byte;
};


/* ---------------------------------------------------------------------------
 * push buttons
 */

#define GPIO_PBTN_I_MASK 0xFF
#define GPIO_PBTN_O_MASK 0x00


/* ---------------------------------------------------------------------------
 * HV relays
 */

#define GPIO_HVRELAY_I_MASK 0x00
#define GPIO_HVRELAY_O_MASK 0xFF


#endif /* INCLUDE_GPIO_SIGNALS */
