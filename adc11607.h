/*
  Plastic Logic hardware library - adc11607

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

#ifndef INCLUDE_ADC11607_H
#define INCLUDE_ADC11607_H 1

#define ADC11607_NB_RESULTS 4

struct adc11607_setup {
	char reserved:1;
	char reset:1;
	char bip_uni:1;
	char clk_sel:1;
	char sel:3;
	char setup_1:1;
};

struct adc11607_config {
	char se_diff:1;
	char cs:4;
	char scan:2;
	char config_0:1;
};

union adc11607_setup_config {
	struct {
		struct adc11607_setup setup;
		struct adc11607_config config;
	};
	char bytes[2];
};

#define ADC11607_SEL_INT_REF_ON   0x1
#define ADC11607_SEL_EXT_REF      0x2
#define ADC11607_SEL_INT_REF      0x4
#define ADC11607_SEL_AIN__REF_OUT 0x2

#define ADC11607_MAX_VALUE 0x3FF

#endif /* INCLUDE_ADC11607_H */
