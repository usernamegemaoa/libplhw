/*
 * Copyright (C) 2011, 2012 Plastic Logic Limited
 * All rights reserved.
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
