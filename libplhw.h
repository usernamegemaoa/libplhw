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

/**
   @file libplhw.h

   This library provides low-level user-side functions to directly control the
   Plastic Logic display hardware.
*/

/** Library version */
#define PLHW_VERSION "1.2"

/** Invalid I2C address value */
#define PLHW_NO_I2C_ADDR 0xFF

#ifndef min
/** Get the smallest relative of two integers */
# define min(a, b) (((a) < (b)) ? (a) : (b))
#endif


/**
   @name CPLD
   @{
 */

/** CLPD hardware switches, wired to various electronic parts */
enum cpld_switch {
	CPLD_HVEN,                   /**< high-voltage PSU enable */
	CPLD_COM_SW_EN,              /**< VCOM switch enable */
	CPLD_COM_SW_CLOSE,           /**< VCOM switch close */
	CPLD_COM_PSU,                /**< VCOM PSU enable */
	CPLD_BPCOM_CLAMP,            /**< back-plane COM clamp enable */
};

/** Opaque structure used in public CPLD interface */
struct cpld;

/** Create an initialised cpld instance
    @param[in] i2c_bus path to the I2C bus device
    @param[in] i2c_address CPLD I2C address
    @return pointer to new cpld instance or NULL if error
 */
extern struct cpld *cpld_init(const char *i2c_bus, char i2c_address);

/** Free the cpld instance
    @param[in] cpld cpld instance as created by cpld_init
 */
extern void cpld_free(struct cpld *cpld);

/** Get the CPLD firmware version
    @param[in] cpld cpld instance
    @return CPLD firmware version number
*/
extern int cpld_get_version(const struct cpld *cpld);

/** Get the board ID stored in CPLD firmware
    @param[in] cpld cpld instance
    @return board ID stored in CPLD firmare
 */
extern int cpld_get_board_id(const struct cpld *cpld);

/** Get the size of the CPLD data (registers)
    @param[in] cpld cpld instance
    @return size of the CPLD data in bytes
*/
extern size_t cpld_get_data_size(const struct cpld *cpld);

/** Dump the CPLD data
    @param[in] cpld cpld instance
    @param[out] data user buffer to receive the CPLD data
    @param[in] size size of the user buffer in bytes
    @return number of bytes copied to the user buffer
 */
extern int cpld_dump(const struct cpld *cpld, char *data, size_t size);

/** Set a CPLD hardware switch
    @param[in] cpld cpld instance
    @param[in] sw switch identifier
    @param[in] on state to set the switch to (1 for on, 0 for off)
    @return 0 if success, -1 if error
*/
extern int cpld_set_switch(struct cpld *cpld, enum cpld_switch sw, int on);

/** Get a CPLD hardware switch
    @param[in] cpld cpld instance
    @param[in] sw switch identifier
    @return 0 if switch is off, 1 if on and -1 if error
*/
extern int cpld_get_switch(struct cpld *cpld, enum cpld_switch sw);

/** @} */


/**
   @name HVPMIC - MAX17135
   @{
*/

#define MAX17135_NB_TIMINGS 8        /**< number of timings */

/** Temperature sensor identifiers */
enum max17135_temp_id {
	MAX17135_TEMP_EXT = 1,       /**< external temperature sensor */
	MAX17135_TEMP_INT,           /**< internal temperature sensor */
};

/** Temperature failure codes */
enum max17135_temp_failure {
	MAX17135_TEMP_OK = 1,        /**< no temperature failure */
	MAX17135_TEMP_OPEN,          /**< open circuit failure */
	MAX17135_TEMP_SHORT,         /**< short circuit failure */
};

/** Fault identifiers */
enum max17135_fault_id {
	MAX17135_FAULT_NONE = 1,     /**< no fault */
	MAX17135_FAULT_FBPG,         /**< GVDD undervoltage fault */
	MAX17135_FAULT_HVINP,        /**< HVINP undervoltage fault */
	MAX17135_FAULT_HVINN,        /**< HVINN undervoltage fault */
	MAX17135_FAULT_FBNG,         /**< GVEE undervoltage fault */
	MAX17135_FAULT_HVINPSC,      /**< HVINP short-circuit fault */
	MAX17135_FAULT_HVINNSC,      /**< HVINN short-circuit fault */
	MAX17135_FAULT_OT,           /**< thermal shutdown */
};

/** High-voltage power supply identifiers */
enum max17135_en_id {
	MAX17135_EN_EN = 1,          /**< main HV PSU */
	MAX17135_EN_CEN,             /**< first VCOM HV PSU */
	MAX17135_EN_CEN2,            /**< second VCOM HV PSU */
};

/** Opaque structure used in public MAX17135 interface */
struct max17135;

/** Create an initialised max17135 instance
    @param[in] i2c_bus path to the I2C bus device
    @param[in] i2c_address MAX17135 I2C address or PLHW_NO_I2C_ADDR for default
    @return pointer to new max17135 instance or NULL if error
 */
extern struct max17135 *max17135_init(const char *i2c_bus, char i2c_address);

/** Free max17135 instance
    @param[in] p max17135 instance as created by max17135_init
 */
extern void max17135_free(struct max17135 *p);

/** Get product identifier code
    @param[in] p max17135 instance
    @return product identifier code
*/
extern int max17135_get_prod_id(struct max17135 *p);

/** Get product revision number
    @param[in] p max17135 instance
    @return product revision number
 */
extern int max17135_get_prod_rev(struct max17135 *p);

/** Get VCOM value
    @param[in] p max17135 instance
    @param[out] value VCOM value
    @return 0 if success, -1 if error
 */
extern int max17135_get_vcom(struct max17135 *p, char *value);

/** Set VCOM value
    @param[in] p max17135 instance
    @param[in] value VCOM value to set
    @return 0 if success, -1 if error
 */
extern int max17135_set_vcom(struct max17135 *p, char value);

/** Save VCOM value into persistent memory
    @param[in] p max17135 instance
    @return 0 if success, -1 if error
 */
extern int max17135_save_vcom(struct max17135 *p);

/** Get a timing value
    @param[in] p max17135 instance
    @param[in] n timing number
    @return timing value in ms (>= 0) if success, -1 if error
 */
extern int max17135_get_timing(struct max17135 *p, unsigned n);

/** Get all timing values
    @param[in] p max17135 instance
    @param[out] data user buffer to store the timings data
    @param[in] size size of the user buffer in bytes
    @return 0 if success, -1 if error
 */
extern int max17135_get_timings(struct max17135 *p, char *data, size_t size);

/** Set a timing value
    @param[in] p max17135 instance
    @param[in] n timing number
    @param[in] value timing value in ms
    @return 0 if success, -1 if error
 */
extern int max17135_set_timing(struct max17135 *p, unsigned n, char value);

/** Set all timing values
    @param[in] p max17135 instance
    @param[in] data user buffer with timings data
    @param[in] size size of the user buffer in bytes
    @return 0 if success, -1 if error
 */
extern int max17135_set_timings(struct max17135 *p, const char *data,
				size_t size);

/** Save timings into persistent memory (DANGER: ONLY 3 WRITE CYCLES)
    @param[in] p max17135 instance
    @return 0 if success, -1 if error
 */
extern int max17135_save_timings(struct max17135 *p);

/** Get status of temperature sensor
    @param[in] p max17135 instance
    @return 0 if disabled, 1 if enabled or -1 if error
 */
extern int max17135_get_temp_sensor_en(struct max17135 *p);

/** Set status of temperature sensor
    @param[in] p max17135 instance
    @param[in] en status to set the temperature sensor to (0 or 1)
    @return 0 if success, -1 if error
 */
extern int max17135_set_temp_sensor_en(struct max17135 *p, int en);

/** Get temperature measurement
    @param[in] p max17135 instance
    @param[out] temp temperature measurement
    @param[in] id temperature sensor identifier
    @return 0 if success, -1 if error
 */
extern int max17135_get_temperature(struct max17135 *p, short *temp,
				    enum max17135_temp_id id);

/** Get temperature failure code
    @param[in] p max17135 instance
    @return temperature failure code or -1 if error
 */
extern int max17135_get_temp_failure(struct max17135 *p);

/** Convert temperature value into degrees as floating point value
    @param[in] p max17135 instance
    @param[in] temp temperature measurement value
    @return temperature in Celsius degrees or -1 if error
 */
extern float max17135_convert_temperature(struct max17135 *p, short temp);

/** Get POK signal status
    @param[in] p max17135 instance
    @return 1 if POK set, 0 if cleared or -1 if error
 */
extern int max17135_get_pok(struct max17135 *p);

/** Set POK delay
    @param[in] p max17135 instance
    @param delay_us delay in micro-seconds to wait before polling POK status
 */
extern void max17135_set_pok_delay(struct max17135 *p, unsigned delay_us);

/** Wait for POK signal (block until set or timeout or I/O error)
    @param[in] p max17135 instance
    @return 0 if success, -1 if error
 */
extern int max17135_wait_for_pok(struct max17135 *p);

/** Enable a given HV PSU
    @param[in] p max17135 instance
    @param[in] id HV PSU identifier
    @param[in] on state to set the HV PSU to (1 to enable, 0 to disable)
    @return 0 if success, -1 if error
 */
extern int max17135_set_en(struct max17135 *p, enum max17135_en_id id, int on);

/** Get the status of a given HV PSU
    @param[in] p max17135 instance
    @param[in] id HV PSU identifier
    @return 1 if enabled, 0 if disabled or -1 if error
 */
extern int max17135_get_en(struct max17135 *p, enum max17135_en_id id);

/** Get fault identifier
    @param[in] p max17135 instance
    @return fault identifier (>= 0) or -1 if error
 */
extern int max17135_get_fault(struct max17135 *p);

/** @} */


/**
   @name HVPMIC - TPS65185
   @{
*/

/** Opaque structure used in public TPS65185 interface */
struct tps65185;

/** Constant chip information */
struct tps65185_info {
	unsigned version;            /**< version number */
	unsigned major;              /**< major version number */
	unsigned minor;              /**< minor version number */
};

/** Power modes */
enum tps65185_power {
	TPS65185_ACTIVE = 7,         /**< active (HV is turned on) */
	TPS65185_STANDBY = 6,        /**< standby (HV is turned off) */
};

/** Power rail identifiers */
enum tps65185_en_id {
	TPS65185_V3P3_EN = 5,        /**< V3P3 (3.3V power supply) */
	TPS65185_VCOM_EN = 4,        /**< VCOM */
	TPS65185_VDDH_EN = 3,        /**< VDDH (postive gate voltage) */
	TPS65185_VPOS_EN = 2,        /**< VPOS (positive source voltage) */
	TPS65185_VEE_EN = 1,         /**< VEE (negative gate voltage) */
	TPS65185_VNEG_EN = 0,        /**< VNEG (negative source voltage) */
};

/** Power rail sequence strobe identifiers */
enum tps65185_strobe {
	TPS65185_STROBE1 = 0,        /**< STROBE1 identifier */
	TPS65185_STROBE2 = 1,        /**< STROBE2 identifier */
	TPS65185_STROBE3 = 2,        /**< STROBE3 identifier */
	TPS65185_STROBE4 = 3,        /**< STROBE4 identifier */
};

/** Power rail sequence strobe delay values */
enum tps65185_delay {
	TPS65185_STROBE_3_MS = 0,    /**< 3ms strobe delay */
	TPS65185_STROBE_6_MS = 1,    /**< 6ms strobe delay */
	TPS65185_STROBE_9_MS = 2,    /**< 9ms strobe delay */
	TPS65185_STROBE_12_MS = 3,   /**< 12ms strobe delay */
};

/** Power rail up/down sequence data */
struct tps65185_seq {
	enum tps65185_strobe vddh;   /**< strobe value for VDDH */
	enum tps65185_strobe vpos;   /**< strobe value for VPOS */
	enum tps65185_strobe vee;    /**< strobe value for VEE */
	enum tps65185_strobe vneg;   /**< strobe value for VNEG */
	enum tps65185_delay strobe1; /**< delay of STROBE1 */
	enum tps65185_delay strobe2; /**< delay of STROBE2 */
	enum tps65185_delay strobe3; /**< delay of STROBE3 */
	enum tps65185_delay strobe4; /**< delay of STROBE4 */
};

/** Create an initialised tps65185 instance
    @param[in] i2c_bus path to the I2C bus device
    @param[in] i2c_address TPS65185 I2C address or PLHW_NO_I2C_ADDR for default
    @return pointer to new tps17135 instance or NULL if error
 */
extern struct tps65185 *tps65185_init(const char *i2c_bus, char i2c_address);

/** Free tps65185 instance
    @param[in] p max17135 instance as created by tps65185_init
 */
extern void tps65185_free(struct tps65185 *p);

/** Get constant chip information
    @param[in] p tps65185 instance
    @param[out] info information structure
*/
extern void tps65185_get_info(struct tps65185 *p, struct tps65185_info *info);

/** Set VCOM register value
    @param[in] p tps65185 instance
    @param[in] value 9-bit VCOM register value to use
    @return 0 if success, -1 if error
*/
extern int tps65185_set_vcom(struct tps65185 *p, uint16_t value);

/** Get the VCOM register value
    @param[in] p tps65185 instance
    @param[out] value 9-bit VCOM register value read back
    @return 0 if success, -1 if error
*/
extern int tps65185_get_vcom(struct tps65185 *p, uint16_t *value);

/** Set power up/down sequence configuration
    @param[in] p tps65185 instance
    @param[in] seq sequence information structure to use
    @param[in] up 1 for power up sequence, 0 for power down
    @return 0 if success, -1 if error
*/
extern int tps65185_set_seq(struct tps65185 *p, const struct tps65185_seq *seq,
			    int up);

/** Get power up/down sequence configuration
    @param[in] p tps65185 instance
    @param[out] seq structure to store the sequence information read back
    @param[in] up 1 for power up sequence, 0 for power down
    @return 0 if success, -1 if error
*/
extern int tps65185_get_seq(struct tps65185 *p, struct tps65185_seq *seq,
			    int up);

/** Set the power mode and state of HV outputs
    @param[in] p tps65185 instance
    @param[in] power power mode (active for HV on, standby for HV off)
    @return 0 if success, -1 if error
*/
extern int tps65185_set_power(struct tps65185 *p, enum tps65185_power power);

/** Enable or disable a specific output power rail and wait for completion
    @param[in] p tps65185 instance
    @param[in] id power rail identifier
    @param[in] on 1 to enable, 0 to disable
    @return 0 if success, -1 if error
*/
extern int tps65185_set_en(struct tps65185 *p, enum tps65185_en_id id, int on);

/** Get the enable state of a spcific output power rail
    @param[in] p tps65185 instance
    @param[in] id power rail identifier
    @return 0 if disabled, 1 if enabled or -1 if error
*/
extern int tps65185_get_en(struct tps65185 *p, enum tps65185_en_id id);

/** @} */


/**
   @name EEPROM
   @{
*/

#define INVALID_OFFSET ((size_t) -1) /**< invalid offset code */

/** Opaque structure used in public EEPROM interface */
struct eeprom;

/** Create an initialised eeprom instance
    @param[in] i2c_bus path to the I2C bus device
    @param[in] i2c_address EEPROM I2C address
    @param[in] mode EEPROM mode (24c01, 24c256 etc...)
    @return pointer to new eeprom instance or NULL if error
 */
extern struct eeprom *eeprom_init(const char *i2c_bus, char i2c_address,
				  const char *mode);

/** Free an eeprom instance
    @param[in] eeprom eeprom instance as created by eeprom_init
 */
extern void eeprom_free(struct eeprom *eeprom);

/** Get the EEPROM mode identifier
    @param[in] eeprom eeprom instance
    @return static string with EEPROM mode
*/
extern const char *eeprom_get_mode(struct eeprom *eeprom);

/** Get the size of the EEPROM
    @param[in] eeprom eeprom instance
    @return EEPROM size in bytes
 */
extern size_t eeprom_get_size(struct eeprom *eeprom);

/** Set the EEPROM page size to override default value
    @param[in] eeprom eeprom instance
    @param[in] page_size size of the EEPROM page
*/
extern void eeprom_set_page_size(struct eeprom *eeprom, size_t page_size);

/** Get the EEPROM page size
    @param[in] eeprom eeprom instance
    @return EEPROM page size
*/
extern size_t eeprom_get_page_size(struct eeprom *eeprom);

/** Set the EEPROM block size
    @param[in] eeprom eeprom instance
    @param[in] block_size size of the data block in bytes
 */
extern void eeprom_set_block_size(struct eeprom *eeprom, size_t block_size);

/** Get the EEPROM block size
    @param[in] eeprom eeprom instance
    @return data block size in bytes
 */
extern size_t eeprom_get_block_size(struct eeprom *eeprom);

/** Seek into the EEPROM data
    @param[in] eeprom eeprom instance
    @param[in] offset offset in bytes
 */
extern void eeprom_seek(struct eeprom *eeprom, size_t offset);

/** Get the current EEPROM data cursor offset
    @param[in] eeprom eeprom instance
    @return current offset in bytes
 */
extern size_t eeprom_get_offset(struct eeprom *eeprom);

/** Read some data from the EEPROM
    @param[in] eeprom eeprom instance
    @param[out] data user buffer to store the EEPROM data
    @param[in] size size of the user buffer
    @return 0 if success, -1 if error
 */
extern int eeprom_read(struct eeprom *eeprom, char *data, size_t size);

/** Write some data into the EEPROM
    @param[in] eeprom eeprom instance
    @param[in] data user data with data to be written into the EEPROM
    @param[in] size size of the user data buffer in bytes
    @return 0 if success, -1 if error
 */
extern int eeprom_write(struct eeprom *eeprom, const char *data, size_t size);

/** @} */


/**
   @name DAC - MAX5820
   @{

   This DAC is used on Plastic Logic hardware to generate the VCOM voltage.
*/

/** Channel identifiers */
enum dac5820_channel_id {
	DAC5820_CH_A = 1,            /**< Identifier for channel A */
	DAC5820_CH_B,                /**< Identifier for channel B */
};

/** Output power mode identifiers */
enum dac5820_power_id {
	DAC5820_POW_ON = 1,          /**< enabled with low impendence */
	DAC5820_POW_OFF_FLOAT,       /**< disabled with high impedence */
	DAC5820_POW_OFF_1K,          /**< disabled with 1K pull-down */
	DAC5820_POW_OFF_100K,        /**< disabled with 100K pull-down */
};

/** Opaque structure used in public DAC interface */
struct dac5820;

/** Create an initialised dac5820 instance
    @param[in] i2c_bus path to the I2C bus device
    @param[in] i2c_address DAC I2C address or PLHW_NO_I2C_ADDR for default
    @return pointer to new dac5820 instance or NULL if error
 */
extern struct dac5820 *dac5820_init(const char *i2c_bus, int i2c_address);

/** Free a dac5820 instance
    @param[in] dac dac5820 instance as created by dac5820_init
 */
extern void dac5820_free(struct dac5820 *dac);

/** Set the output power mode for a given channel
    @param[in] dac dac5820 instance
    @param[in] channel output channel identifier
    @param[in] power power mode identifier
    @return 0 if success, -1 if error
 */
extern int dac5820_set_power(struct dac5820 *dac,
			     enum dac5820_channel_id channel,
			     enum dac5820_power_id power);

/** Set the output value for a given channel
    @param[in] dac dac5820 instance
    @param[in] channel output channel identifier
    @param[in] value output value to set on the given channel
    @return 0 if success, -1 if error
 */
extern int dac5820_output(struct dac5820 *dac, enum dac5820_channel_id channel,
			  char value);

/** @} */


/**
   @name ADC - MAX11607
   @{

   This ADC is used on Plastic Logic hardware to perform automatic VCOM
   calibration.
*/

/** Voltage reference identifiers */
enum adc11607_ref_id {
	ADC11607_REF_VDD = 1,        /**< VDD reference voltage */
	ADC11607_REF_INTERNAL,       /**< Internal reference voltage */
	ADC11607_REF_EXTERNAL,       /**< External reference voltage */
};

/** ADC result type */
typedef unsigned short adc11607_result_t;

/** Invalid ADC result code */
#define ADC11607_INVALID_RESULT ((adc11607_result_t) -1)

/** Opaque structure used in public ADC interface */
struct adc11607;

/** Create an initialised adc11607 instance
    @param[in] i2c_bus path to the I2C bus device
    @param[in] i2c_address ADC I2C address or PLHW_NO_I2C_ADDR for default
    @return pointer to new adc11607 instance or NULL if error
 */
extern struct adc11607 *adc11607_init(const char *i2c_bus, int i2c_address);

/** Free a adc11607 instance
    @param[in] adc adc11607 instance as created by adc11607_init
 */
extern void adc11607_free(struct adc11607 *adc);

/** Set external reference value in volts
    @param[in] adc adc11607 instance
    @param[in] value reference voltage in volts
 */
extern void adc11607_set_ext_ref_value(struct adc11607 *adc, float value);

/** Select voltage reference
    @param[in] adc adc11607 instance
    @param[in] ref_id voltage reference identifier
    @return 0 if success, -1 if error
 */
extern int adc11607_set_ref(struct adc11607 *adc, enum adc11607_ref_id ref_id);

/** Get current voltage reference identifier
    @param[in] adc adc11607 instance
    @return voltage reference identifier
 */
extern enum adc11607_ref_id adc11607_get_ref(struct adc11607 *adc);

/** Get the number of ADC channels
    @param[in] adc adc11607 instance
    @return number of ADC channels
 */
extern unsigned adc11607_get_nb_channels(struct adc11607 *adc);

/** Select a channel range (number of channels to use)
    @param[in] adc adc11607 instance
    @param[in] range range of channels to use
    @return 0 if success, -1 if error
 */
extern int adc11607_select_channel_range(struct adc11607 *adc, unsigned range);

/** Read the results of the last ADC conversion
    @param[in] adc adc11607 instance
    @return 0 if success, -1 if error
 */
extern int adc11607_read_results(struct adc11607 *adc);

/** Get the result for a given channel
    @param[in] adc adc11607 instance
    @param[in] channel channel number to get the result from
    @return ADC conversion result
 */
extern adc11607_result_t adc11607_get_result(struct adc11607 *adc,
					     unsigned channel);

/** Convert an ADC result into volts as a floating point value
    @param[in] adc adc11607 instance
    @param[in] value ADC value to convert into volts
    @return voltage for the given ADC value as floating point in volts
 */
extern float adc11607_get_volts(struct adc11607 *adc, adc11607_result_t value);

/** Convert an ADC result into millivolts as an integer
    @param[in] adc adc11607 instance
    @param[in] value ADC value to convert into millivolts
    @return voltage for the given ADC value as integer in millivolts
 */
extern unsigned adc11607_get_millivolts(struct adc11607 *adc,
					adc11607_result_t value);

/** @} */


/**
   @name Push buttons - using PCF8574 I2C-GPIO expander
   @{
*/

#define PBTN_DEF_I2C_ADDR 0x21       /**< default push buttons I2C address */

/** Push button identifiers */
enum pbtn_id {
	PBTN_2 = 0x01,               /**< push button #2 */
	PBTN_3 = 0x02,               /**< push button #3 */
	PBTN_4 = 0x04,               /**< push button #4 */
	PBTN_5 = 0x08,               /**< push button #5 */
	PBTN_6 = 0x10,               /**< push button #6 */
	PBTN_7 = 0x20,               /**< push button #7 */
	PBTN_8 = 0x40,               /**< push button #8 */
	PBTN_9 = 0x80,               /**< push button #9 */
	PBTN_ALL = 0xFF,             /**< all push buttons shorthand */
};

/** Opaque structure used in public push button interface */
struct pbtn;

/** Abort callback type

    The abort callback is called when polling the push buttons status.
    Returning 1 aborts the polling to cleanly exit or implement a timeout.
 */
typedef int (*pbtn_abort_t)(void);

/** Create an initialised push buttons instance
    @param[in] i2c_bus path to the I2C bus device
    @param[in] i2c_address ADC I2C address or PLHW_NO_I2C_ADDR for default
    @return pointer to new push buttons instance or NULL if error
 */
extern struct pbtn *pbtn_init(const char *i2c_bus, int i2c_address);

/** Free a push buttons instance
    @param[in] pbtn push buttons instance as created by pbtn_init
 */
extern void pbtn_free(struct pbtn *pbtn);

/** Probe the state of the push buttons
    @param[in] pbtn pbtn instance as created by pbtn_init
    @param[in] mask binary mask to select a set of push buttons
    @return 1 if any selected button is pressed, 0 otherwise or -1 if error
 */
extern int pbtn_probe(struct pbtn *pbtn, enum pbtn_id mask);

/** Set the abort callback function
    @param[in] pbtn pbtn instance as created by pbtn_init
    @param[in] abort abort callback function
 */
extern void pbtn_set_abort_cb(struct pbtn *pbtn, pbtn_abort_t abort);

/** Wait for a given push button combination to enter a given state
    @param[in] pbtn pbtn instance as created by pbtn_init
    @param[in] mask binary mask to select a set of push buttons
    @param[in] state state all the push buttons need to be in before returning
    @return 0 if success, 1 if abort or -1 if error
 */
extern int pbtn_wait(struct pbtn *pbtn, enum pbtn_id mask, int state);

/** Wait for any of a set of push buttons to enter a given state
    @param[in] pbtn pbtn instance as created by pbtn_init
    @param[in] mask binary mask to select a set of push buttons
    @param[in] state state any of the byttons needs to be before returning
    @return 0 if success, 1 if abort or -1 if error
 */
extern int pbtn_wait_any(struct pbtn *pbtn, enum pbtn_id mask, int state);

/** @} */

#endif /* INCLUDE_LIBPLHW_H */
