#ifndef CHIPS_H
#define CHIPS_H

#include <stdint.h>

// Turn on pullups on input pins
#define OPTIONS_PULLUPS 0x01
// Needs a latch pulse
#define OPTIONS_LATCH   0x02

// Output pin for OPTIONS_LATCH
#define LATCH_PIN 0

// For AVR chips
#define ISP_MOSI 0
#define ISP_MISO 0
#define ISP_SCK 1
#define ISP_RESET 2
#define ISP_XTAL 3

typedef struct
{
	/** Name of the chip type */
	char name[16];

	/** Options for chip, including pullup */
	uint8_t options;

	/** Total number of pins on the chip */
	uint8_t pins;

	/** Total number of address pins.
	 * The download will retrieve 2^addr_width bytes.
 	 */
	uint8_t addr_width;

	/** Total number of data pins.
	 * If data_pins == 0, the chip is assumed to be in AVR ISP mode.
	 */
	uint8_t data_width;

	/** Address pins.
	 * An array of up to 24 pins in package numbering.
	 * Any that are 0 will be ignored.  Other than ISP mode chips,
	 * there should be addr_width pins defined here.
	 * These will be configured as outputs from the reader
	 * and initially driven low.
	 */
	uint8_t addr_pins[24];

	/** Data pins.
	 * An array of up to 24 pins in package numbering.
	 * Any that are 0 will be ignored.  Other than ISP mode chips,
	 * there should be data_width pins defined here.
	 * These will be configured as inputs from the reader,
	 * with no pull ups.
	 */
	uint8_t data_pins[24];

	/** Pins to be driven high at all times.
	 * These will be configured as outputs and drive hi.
	 * Typical power limits are sourcing 50 mA per pin.
	 */
	uint8_t hi_pins[8];

	/** Pins to be driven low at all times.
	 * These will be configured as outputs and driven low.
	 * Typical power limits are sinking 50 mA per pin.
	 */
	uint8_t lo_pins[8];

  /** Pin to use for VCC. This is primarily a documentation
   * field for the user, so they know where to hook the VCC
   * line.
   */
  uint8_t vcc;

  /** Pin to use for ground. This is primarily a documentation
   * field for the user, so they know where to hook the ground
   * line.
   */
  uint8_t gnd;

} prom_t;

extern const prom_t proms[];
extern const uint16_t proms_count;

#endif // CHIPS_H
