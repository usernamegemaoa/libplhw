/*
  Plastic Logic hardware library - libplhw

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

#ifndef INCLUDE_LIBPLHW_H
#define INCLUDE_LIBPLHW_H 1

#include <stdlib.h>
#include <stdint.h>

#define PLHW_VERSION "0.4"

#define PLHW_DEF_I2C_BUS "/dev/i2c-0"


/* ----------------------------------------------------------------------------
 * CPLD
 */

#define CPLD_DEF_I2C_ADDR 0x70

enum cpld_switch {
	CPLD_HVEN,
	CPLD_COM_SW_EN,
	CPLD_COM_SW_CLOSE,
	CPLD_COM_PSU,
	CPLD_BPCOM_CLAMP,
};

struct cpld;

extern struct cpld *cpld_init(const char *i2c_bus, char i2c_address);
extern void cpld_free(struct cpld *cpld);

extern int cpld_get_version(const struct cpld *cpld);
extern int cpld_get_board_id(const struct cpld *cpld);
extern size_t cpld_get_data_size(const struct cpld *cpld);
extern int cpld_dump(const struct cpld *cpld, char *data, size_t size);

extern int cpld_set_switch(struct cpld *cpld, enum cpld_switch sw, int on);
extern int cpld_get_switch(struct cpld *cpld, enum cpld_switch sw);


/* ----------------------------------------------------------------------------
 * HV PMIC (MAX17135AETJ+)
 */

#define HVPMIC_DEF_I2C_ADDR 0x48
#define HVPMIC_NB_TIMINGS 8

enum hvpmic_temp_id {
	HVPMIC_TEMP_EXT = 1,
	HVPMIC_TEMP_INT,
};

enum hvpmic_temp_failure {
	HVPMIC_TEMP_OK = 1,
	HVPMIC_TEMP_OPEN,
	HVPMIC_TEMP_SHORT,
};

enum hvpmic_fault_id {
	HVPMIC_FAULT_NONE = 1,
	HVPMIC_FAULT_FPBG,
	HVPMIC_FAULT_HVINP,
	HVPMIC_FAULT_HVINN,
	HVPMIC_FAULT_FBNG,
	HVPMIC_FAULT_HVINPSC,
	HVPMIC_FAULT_HVINNSC,
	HVPMIC_FAULT_OT,
};

enum hvpmic_en_id {
	HVPMIC_EN_EN = 1,
	HVPMIC_EN_CEN,
	HVPMIC_EN_CEN2,
};

struct hvpmic;

extern struct hvpmic *hvpmic_init(const char *i2c_device, char i2c_address);
extern void hvpmic_free(struct hvpmic *hvpmic);

extern int hvpmic_get_prod_id(struct hvpmic *hvpmic);
extern int hvpmic_get_prod_rev(struct hvpmic *hvpmic);

extern int hvpmic_get_vcom(struct hvpmic *hvpmic, char *value);
extern int hvpmic_set_vcom(struct hvpmic *hvpmic, char value);
extern int hvpmic_save_vcom(struct hvpmic *hvpmic);

extern int hvpmic_get_timing(struct hvpmic *hvpmic, unsigned n);
extern int hvpmic_get_timings(struct hvpmic *hvpmic, char *data, size_t size);
extern int hvpmic_set_timing(struct hvpmic *hvpmic, unsigned n, char value);
extern int hvpmic_set_timings(struct hvpmic *hvpmic, const char *data,
                              size_t size);
extern int hvpmic_save_timings(struct hvpmic *hvpimc);

extern int hvpmic_get_temp_sensor_en(struct hvpmic *hvpmic);
extern int hvpmic_set_temp_sensor_en(struct hvpmic *hvpmic, int en);
extern int hvpmic_get_temperature(struct hvpmic *hvpmic, short *temp,
                                  enum hvpmic_temp_id id);
extern int hvpmic_get_temp_failure(struct hvpmic *hvpmic);
extern float hvpmic_convert_temperature(struct hvpmic *hvpmic, short temp);

extern int hvpmic_get_pok(struct hvpmic *hvpmic);
extern void hvpmic_set_pok_delay(struct hvpmic *hvpmic, unsigned delay_us);
extern int hvpmic_wait_for_pok(struct hvpmic *hvpmic);

extern int hvpmic_set_en(struct hvpmic *hvpmic, enum hvpmic_en_id id, int on);
extern int hvpmic_get_en(struct hvpmic *hvpmic, enum hvpmic_en_id id);
extern int hvpmic_get_fault(struct hvpmic *hvpmic);


/* ---------------------------------------------------------------------------
 * EEPROM
 */

#define EEPROM_DEF_I2C_ADDR 0x50
#define INVALID_OFFSET ((size_t) -1)

struct eeprom;

extern struct eeprom *eeprom_init(const char *i2c_bus, char i2c_addr);
extern void eeprom_free(struct eeprom *eeprom);

extern size_t eeprom_get_size(struct eeprom *eeprom);
extern void eeprom_seek(struct eeprom *eeprom, size_t offset);
extern size_t eeprom_get_offset(struct eeprom *eeprom);
extern int eeprom_read(struct eeprom *eeprom, char *data, size_t size);
extern int eeprom_write(struct eeprom *eeprom, const char *data, size_t size);


/* ----------------------------------------------------------------------------
 * DAC - MAX5820
 */

#define DAC5820_DEF_I2C_ADDR 0x39

enum dac5820_channel_id {
	DAC5820_CH_A = 1,
	DAC5820_CH_B,
};

enum dac5820_power_id {
	DAC5820_POW_ON = 1,
	DAC5820_POW_OFF_FLOAT,
	DAC5820_POW_OFF_1K,
	DAC5820_POW_OFF_100K,
};

struct dac5820;

extern struct dac5820 *dac5820_init(const char *i2c_bus, int i2c_addr);
extern void dac5820_free(struct dac5820 *dac);

extern int dac5820_set_power(struct dac5820 *dac,
			     enum dac5820_channel_id channel,
			     enum dac5820_power_id power);
extern int dac5820_output(struct dac5820 *dac, enum dac5820_channel_id channel,
			  char value);


/* ----------------------------------------------------------------------------
 * ADC - MAX11607
 */

#define ADC11607_DEF_I2C_ADDR 0x34

enum adc11607_ref_id {
	ADC11607_REF_VDD = 1,
	ADC11607_REF_INTERNAL,
	ADC11607_REF_EXTERNAL,
};

typedef unsigned short adc11607_result_t;
#define ADC11607_INVALID_RESULT ((adc11607_result_t) -1)

struct adc11607;

extern struct adc11607 *adc11607_init(const char *i2c_bus, int i2c_addr);
extern void adc11607_free(struct adc11607 *adc);

extern void adc11607_set_ext_ref_value(struct adc11607 *adc, float value);
extern int adc11607_set_ref(struct adc11607 *adc, enum adc11607_ref_id ref_id);
extern enum adc11607_ref_id adc11607_get_ref(struct adc11607 *adc);

extern unsigned adc11607_get_nb_channels(struct adc11607 *adc);
extern int adc11607_select_channel_range(struct adc11607 *adc, unsigned range);
extern int adc11607_read_results(struct adc11607 *adc);
extern adc11607_result_t adc11607_get_result(struct adc11607 *adc,
					     unsigned channel);
extern float adc11607_get_volts(struct adc11607 *adc, adc11607_result_t value);
extern unsigned adc11607_get_millivolts(struct adc11607 *adc,
					adc11607_result_t value);

/* ----------------------------------------------------------------------------
 * Push buttons
 */

#define PBTN_DEF_I2C_ADDR 0x21

enum pbtn_id {
	PBTN_2 = 0x01,
	PBTN_3 = 0x02,
	PBTN_4 = 0x04,
	PBTN_5 = 0x08,
	PBTN_6 = 0x10,
	PBTN_7 = 0x20,
	PBTN_8 = 0x40,
	PBTN_9 = 0x80,
	PBTN_ALL = 0xFF,
};

struct pbtn;

typedef int (*pbtn_abort_t)(void);

extern struct pbtn *pbtn_init(const char *i2c_bus, int i2c_addr);
extern void pbtn_free(struct pbtn *pbtn);

extern int pbtn_probe(struct pbtn *pbtn, enum pbtn_id mask);
extern void pbtn_set_abort_cb(struct pbtn *pbtn, pbtn_abort_t abort);
extern int pbtn_wait(struct pbtn *pbtn, enum pbtn_id mask, int state);
extern int pbtn_wait_any(struct pbtn *pbtn, enum pbtn_id mask, int state);

#endif /* INCLUDE_LIBPLHW_H */
