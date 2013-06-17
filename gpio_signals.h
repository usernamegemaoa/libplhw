/*
 * Copyright (C) 2011, 2012 Plastic Logic Limited
 * All rights reserved.
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
