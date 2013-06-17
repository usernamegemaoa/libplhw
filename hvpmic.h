/*
 * Copyright (C) 2011, 2012 Plastic Logic Limited
 * All rights reserved.
 */

#ifndef INCLUDE_HVPMIC_H
#define INCLUDE_HVPMIC_H 1

enum hvpmic_register {
	HVPMIC_REG_EXT_TEMP   = 0x00,
	HVPMIC_REG_CONF       = 0x01,
	HVPMIC_REG_INT_TEMP   = 0x04,
	HVPMIC_REG_TEMP_STAT  = 0x05,
	HVPMIC_REG_PROD_REV   = 0x06,
	HVPMIC_REG_PROD_ID    = 0x07,
	HVPMIC_REG_DVR        = 0x08,
	HVPMIC_REG_ENABLE     = 0x09,
	HVPMIC_REG_FAULT      = 0x0A,
	HVPMIC_REG_PROG       = 0x0C,
	HVPMIC_REG_TIMING_1   = 0x10,
	HVPMIC_REG_TIMING_2   = 0x11,
	HVPMIC_REG_TIMING_3   = 0x12,
	HVPMIC_REG_TIMING_4   = 0x13,
	HVPMIC_REG_TIMING_5   = 0x14,
	HVPMIC_REG_TIMING_6   = 0x15,
	HVPMIC_REG_TIMING_7   = 0x16,
	HVPMIC_REG_TIMING_8   = 0x17,
};

/* HVPMIC_REG_CONF */
union hvpmic_conf {
	struct {
		char shutdown:1;
	};
	char byte;
};

/* HVPMIC_REG_TEMP_STAT */
union hvpmic_temp_stat {
	struct {
		char busy:1;
		char open:1;
		char shrt:1;
		char reserved:5;
	};
	char byte;
};

/* HVPMIC_REG_ENABLE */
union hvpmic_enable {
	struct {
		char en:1;
		char cen:1;
		char cen2:1;
		char reserved:5;
	};
	char byte;
};

/* HVPMIC_REG_FAULT */
union hvpmic_fault {
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

/* HVPMIC_REG_PROG */
union hvpmic_prog {
	struct {
		char dvr:1;
		char timing:1;
	};
	char byte;
};

#endif /* INCLUDE_HVPMIC_H */
