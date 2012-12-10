#include "chips.h"
#include "bits.h"

const prom_t proms[] = {
{
	// Default is to leave everything in tristate mode
	.name		= "NONE",
	.pins		= 28,

	.addr_width	= 0,
	.addr_pins	= {
	},

	.data_width	= 0,
	.data_pins	= {
	},

	.hi_pins	= { },
	.lo_pins	= { },
},
{
	.name		= "M27C512",
	.pins		= 28,

	.addr_width	= 16,
	.addr_pins	= {
		10, 9, 8, 7, 6, 5, 4, 3, 25, 24, 21, 23, 2, 26, 27, 1,
	},

	.data_width	= 8,
	.data_pins	= {
		11, 12, 13, 15, 16, 17, 18, 19,
	},

	.hi_pins	= { 28, },
	.lo_pins	= { 22, 20, 14, },
	.vcc		= 28,
	.gnd		= 14,
},
{
	.name		= "M27C256",
	.pins		= 28,
	.addr_width	= 15,
	.addr_pins	= {
		10, 9, 8, 7, 6, 5, 4, 3, 25, 24, 21, 23, 2, 26, 27,
	},

	.data_width	= 8,
	.data_pins	= {
		11, 12, 13, 15, 16, 17, 18, 19,
	},
	.hi_pins	= { 28, 1 },
	.lo_pins	= { 22, 20, 14, },

	.vcc		= 28,
	.gnd		= 14,
},
{
	.name		= "M27C128",
	.pins		= 28,
	.addr_width	= 14,
	.addr_pins	= {
		10, 9, 8, 7, 6, 5, 4, 3, 25, 24, 21, 23, 2, 26,
	},

	.data_width	= 8,
	.data_pins	= {
		11, 12, 13, 15, 16, 17, 18, 19,
	},
	.hi_pins	= { 28, 1, 27, },
	.lo_pins	= { 22, 20, 14, },

	.vcc		= 28,
	.gnd		= 14,
},
{
	.name		= "LH-535618",
    .options 	= OPTIONS_PULLUPS | OPTIONS_LATCH,
	.pins		= 28,
	.addr_width	= 15,
	.addr_pins	= {
    10, 9, 8, 7, 6, 5, 4, 3, 26, 25, 2, 20, 24, 22, 28,
	},

	.data_width	= 8,
	.data_pins	= {
		11, 12, 13, 15, 16, 17, 18, 19,
	},
	.hi_pins	= { 1, },
	.lo_pins	= { 
    [LATCH_PIN] = 23,
    21, 27, 14, },

	.vcc		= 1,
	.gnd		= 14,
},
{
	.name		= "M27C64",
	.pins		= 28,
	.addr_width	= 13,
	.addr_pins	= {
		10, 9, 8, 7, 6, 5, 4, 3, 25, 24, 21, 23, 2,
	},

	.data_width	= 8,
	.data_pins	= {
		11, 12, 13, 15, 16, 17, 18, 19,
	},
	.hi_pins	= { 28, 1, 27 },
	.lo_pins	= { 22, 20, 14, },

	.vcc		= 28,
	.gnd		= 14,
},
{
	.name		= "87C64",
	.options 	= OPTIONS_PULLUPS | OPTIONS_LATCH,
	.pins		= 28,
	.addr_width	= 13,
	.addr_pins	= {
		10, 9, 8, 7, 6, 5, 4, 3, 25, 24, 21, 23, 2,
	},

	.data_width	= 8,
	.data_pins	= {
		11, 12, 13, 15, 16, 17, 18, 19,
	},
	.hi_pins	= { 28, 1, 27 },
	.lo_pins	= { 
		[LATCH_PIN] = 20,
		22, 14, 
	},

	.vcc		= 28,
	.gnd		= 14,
},
{
	.name		= "C64-2732",
	.pins		= 24,
	.addr_width	= 12,
	.addr_pins	= {
		8, 7, 6, 5, 4, 3, 2, 1, 23, 22, 19, 18,
	},

	.data_width	= 8,
	.data_pins	= {
		9, 10, 11, 13, 14, 15, 16, 17,
	},
	.hi_pins	= { 24, 21 },
	.lo_pins	= { 12, 20, },

	.vcc		= 24,
	.gnd		= 12,
},
{
	/** 512x8 PROM -- UNTESTED */	
	.name		= "TBP28S42",
	.pins		= 20,
	.addr_width	= 9,
	.addr_pins	= {
		1, 2, 3, 4, 5, 16, 17, 18, 19,
	},
	.data_width	= 8,
	.data_pins	= {
		6, 7, 8, 9, 11, 12, 13, 14,
	},
	.hi_pins	= { 20 },
	.lo_pins	= { 10, 15 },
},

{
	/** 8192x8 UV EEPROM, found in DX synth */
	.name		= "MBM2764-30",
	.pins		= 28,
	.addr_width	= 13,
	.addr_pins	= {
		10, 9, 8, 7, 6, 5, 4, 3, 25, 24, 21, 23, 2,
	},

	.data_width	= 8,
	.data_pins	= {
		11, 12, 13, 15, 16, 17, 18, 19,
	},
	.hi_pins	= {
		//28, // vdd, disabled since external power must be used
		27, // pgm
		1,  // vpp
	},
	.lo_pins	= { 22, 20, 14, }, // !oe, !cs, gnd
},
{
	// C64 kernel and basic ROMs
	.name		= "27C210",
	.pins		= 40,
	.addr_width	= 16,
	.addr_pins	= {
    21, 22, 23, 24, 25, 26, 27, 28, 29, 31, 32, 33, 34, 35, 36, 37,
	},
	.data_width	= 16,
	.data_pins	= {
    19, 18, 17, 16, 15, 14, 13, 12, 10,  9,  8,  7,  6,  5,  4,  3,
	},
	.hi_pins	= {
    40, // VCC
    1,  // VPP
    39, // PGM'
	},
	.lo_pins	= {
    2, // E'
    11, 30, // GND
    20, // G'
	},

	.vcc		= 40,
	.gnd		= 11,
},
{
	/** Apple Mac SE PROM chips
	 * Similar to a M27C512, but with the 17th address line
	 * on 22 instead of Vpp, allowing 128 KB of data.
	 */
	.name		= "APPLE PROM",
	.pins		= 28,
	.addr_width	= 17,
	.addr_pins	= {
		10, 9, 8, 7, 6, 5, 4, 3, 25, 24, 21, 23, 2, 26, 27, 1, 22
	},

	.data_width	= 8,
	.data_pins	= {
		11, 12, 13, 15, 16, 17, 18, 19,
	},
	.hi_pins	= { 28, },
	.lo_pins	= { 20, 14, },
},
{
	.name		= "28F512 (untstd)",
	.pins		= 32,
	.addr_width	= 16,
	.addr_pins	= {
		12, 11, 10, 9, 8, 7, 6, 5, 27, 26, 23, 25, 4, 28, 29, 15
	},
	.data_width	= 8,
	.data_pins	= {
		13, 14, 15, 17, 18, 19, 20, 21,
	},
	.hi_pins	= {
		32, // vcc
		31, // !we
		1, // Vpp
	},
	.lo_pins	= {
		16, // gnd
		24, // !oe
		22, // !ce
	},

	.vcc		= 32,
	.gnd		= 16,
},
{
	// C64 kernel and basic ROMs
	.name		= "2364A",
	.pins		= 24,
	.addr_width	= 13,
	.addr_pins	= {
		8, 7, 6, 5, 4, 3, 2, 1, 23, 22, 19, 18, 21,
	},
	.data_width	= 8,
	.data_pins	= {
		9, 10, 11, 13, 14, 15, 16, 17,
	},
	.hi_pins	= {
		24,
	},
	.lo_pins	= {
		12, // gnd
		20, // !cs
	},

	.vcc		= 24,
	.gnd		= 12,
},
{
	/** 2716 mask ROM used in video games.
	 * \note: Not tested yet.
	 */
	.name		= "2716 (untested)",
	.pins		= 24,
	.addr_width	= 11,
	.addr_pins	= {
		8, 7, 6, 5, 4, 3, 2, 1, 23, 22, 19,
	},

	.data_width	= 8,
	.data_pins	= {
		9, 10, 11, 13, 14, 15, 16, 17
	},

	.hi_pins	= { 24, 21, },
	.lo_pins	= { 12, 20, 18 },

	.vcc		= 24,
	.gnd		= 12,
},
{
	/** 9316 mask ROM used in video games.
	 * \note: Not tested yet.
	 */
	.name		= "9316 (untested)",
	.pins		= 24,
	.addr_width	= 11,
	.addr_pins	= {
		8, 7, 6, 5, 4, 3, 2, 1, 23, 22, 19,
	},

	.data_width	= 8,
	.data_pins	= {
		9, 10, 11, 13, 14, 15, 16, 17
	},

	.hi_pins	= { 24, 18, },
	.lo_pins	= { 12, 21, 20 },

	.vcc		= 24,
	.gnd		= 12,
},
{
	.name		= "HN462732",
	.pins		= 24,
	.addr_width	= 12,
	.addr_pins	= {
		8, 7, 6, 5, 4, 3, 2, 1, 23, 22, 19, 21,
	},
	.data_width	= 8,
	.data_pins	= {
		9, 10, 11, 13, 14, 15, 16, 17,
	},
	.hi_pins	= {
		24, // vcc
	},
	.lo_pins	= {
		12, // gnd
		20, // !oe
		18, // !ce
	},

	.vcc		= 24,
	.gnd		= 12,
},
{
	/** atmega8.
	 * Not an EEPROM, but a chip to read via ISP.
	 * data_width / addr_width == 0 to indicate that this is not eeprom
	 */
	.name		= "ATMega8",
	.pins		= 28,
	.addr_width	= 13,
	.addr_pins	= {
		[ISP_MOSI] = 17, // from the reader to the chip
		[ISP_SCK] = 19, // SCK,
		[ISP_RESET] = 1, // reset
		[ISP_XTAL] = 9, // xtal
	},
	.data_pins	= {
		[ISP_MISO] = 18, // from the chip back to the reader
	},
	.lo_pins	= {
		8, // gnd
		22, // gnd
	},
	.hi_pins	= {
		7, // vcc
		20, // avcc
	},

	.vcc		= 7,
	.gnd		= 8,
},
};

const uint16_t proms_count = array_count(proms);
