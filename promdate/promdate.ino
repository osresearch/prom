/**
 * \file PROM dancer
 *
 * Read up to 40 pin DIP PROMs using a Teensy++ 2.0
 */

//#include <avr/io.h>
//#include <avr/pgmspace.h>
//#include <avr/interrupt.h>
#include <stdint.h>
//#include <string.h>
#include <util/delay.h>
#include "xmodem.h"
#include "bits.h"
#include "chips.h"

uint8_t recv_str(char *buf, uint8_t size);
void parse_and_execute_command(const char *buf, uint8_t num);

static uint8_t hexdigit(
	uint8_t x
)
{
	x &= 0xF;
	if (x < 0xA)
		return x + '0' - 0x0;
	else
		return x + 'A' - 0xA;
}


static uint8_t
printable(
	uint8_t x
)
{
	if ('A' <= x && x <= 'Z')
		return 1;
	if ('a' <= x && x <= 'z')
		return 1;
	if ('0' <= x && x <= '9')
		return 1;
	if (x == ' ')
		return 1;

	return 0;
}
	

/** Total number of mapped pins.
 * This is unlikely to change without a significant hardware redesign.
 */
#define ZIF_PINS 40

/** Mapping of AVR IO ports to the ZIF socket pins */
static const uint8_t ports[ZIF_PINS+1] = {
	0,
	0xB6, // 1
	0xB5,
	0xB4,
	0xB3,
	0xB2,
	0xB1,
	0xB0,
	0xE7,
	0xE6,
	0xA2,
	0xA1,
	0xF0,
	0xF1,
	0xF2,
	0xF3,
	0xF4,
	0xF5,
	0xF6,
	0xF7,
	0xA3, // 20

	0xA7, // 21
	0xC7,
	0xC6,
	0xC5,
	0xC4,
	0xC3,
	0xC2,
	0xC1,
	0xC0,
	0xE1,
	0xE0,
	0xD7,
	0xD6,
	0xD5,
	0xD4,
	0xD3,
	0xD2,
	0xD1,
	0xD0,
	0xB7, // 40
};

/** Select one of the chips */
static const prom_t * prom = &proms[0];


/** Translate PROM pin numbers into ZIF pin numbers */
static inline uint8_t
prom_pin(
	const uint8_t pin
)
{
	if (pin <= prom->pins / 2)
		return ports[pin];
	else
		return ports[pin + ZIF_PINS - prom->pins];
}


/** Generate a 0.5 MHz clock on the XTAL pin to drive the chip
 * if it does not have a built in oscillator enabled.
 */
static void
isp_clock(
	uint8_t cycles
)
{
	const uint8_t xtal = prom_pin(prom->addr_pins[ISP_XTAL]);
	for (uint8_t i = 0 ; i < cycles ; i++)
	{
		out(xtal, 1);
		_delay_us(1);
		out(xtal, 0);
		_delay_us(1);
	}
}


/** Send a byte to an AVR ISP enabled chip and read a result.
 * Since the AVR ISP is bidirectional, every byte out is also a byte in.
 */
static uint8_t
isp_write(
	uint8_t byte
)
{
	const uint8_t mosi = prom_pin(prom->addr_pins[ISP_MOSI]);
	const uint8_t sck = prom_pin(prom->addr_pins[ISP_SCK]);
	const uint8_t miso = prom_pin(prom->data_pins[ISP_MISO]);
	uint8_t rc = 0;

	for (uint8_t i = 0 ; i < 8 ; i++, byte <<= 1)
	{
		out(mosi, (byte & 0x80) ? 1 : 0);
		isp_clock(4);

		out(sck, 1);
		isp_clock(4);

		rc = (rc << 1) | (in(miso) ? 1 : 0);
		out(sck, 0);
	}

	return rc;
}


/** Enter programming mode for an ISP chip.
 * \return 1 on success, 0 on failure.
 */
static int
isp_setup(void)
{
	// Pulse the RESET pin, while holding SCK low.
	const uint8_t sck = prom_pin(prom->addr_pins[ISP_SCK]);
	const uint8_t reset = prom_pin(prom->addr_pins[ISP_RESET]);
	const uint8_t miso = prom_pin(prom->data_pins[ISP_MISO]);
	out(sck, 0);
	out(reset, 1);
	isp_clock(4);
	out(reset, 0);
	isp_clock(255);

	// Now delay at least 20 ms
	_delay_ms(20);

	uint8_t rc1, rc2, rc3, rc4;

	// Enter programming mode; enable pull up on the MISO pin
	out(miso, 1);

	rc1 = isp_write(0xAC);
	rc2 = isp_write(0x53);
	rc3 = isp_write(0x12);
	rc4 = isp_write(0x34);

	// Disable pull up
	out(miso, 0);

	if (rc3 == 0x53)
		return 1;

	// Now show what we read
	char buf[11];
	buf[0] = hexdigit(rc1 >> 4);
	buf[1] = hexdigit(rc1 >> 0);
	buf[2] = hexdigit(rc2 >> 4);
	buf[3] = hexdigit(rc2 >> 0);
	buf[4] = hexdigit(rc3 >> 4);
	buf[5] = hexdigit(rc3 >> 0);
	buf[6] = hexdigit(rc4 >> 4);
	buf[7] = hexdigit(rc4 >> 0);
	buf[8] = '\0';

	Serial.println(buf);
	return 0;
}


/** Read a byte using the AVRISP, instead of the normal PROM format.
 */
static uint8_t
isp_read(
	uint32_t addr
)
{
	uint8_t h = (addr >> 12) & 0x01;
	uint8_t a = (addr >>  8) & 0x0F;
	uint8_t b = (addr >>  0) & 0xFF;
	isp_write(0x20 | (h ? 0x8 : 0));
	isp_write(a);
	isp_write(b);
	return isp_write(0);
}



/** Configure all of the IO pins for the new PROM type */
static void
prom_setup(void)
{
	// Configure all of the address pins as outputs,
	// pulled low for now
	for (uint8_t i = 0 ; i < array_count(prom->addr_pins) ; i++)
	{
		uint8_t pin = prom_pin(prom->addr_pins[i]);
		if (pin == 0)
			continue;
		out(pin, 0);
		ddr(pin, 1);
	}

	// Configure all of the data pins as inputs,
	// Enable pullups if specified.
	for (uint8_t i = 0 ; i < array_count(prom->data_pins) ; i++)
	{
		uint8_t pin = prom_pin(prom->data_pins[i]);
		if (pin == 0)
			continue;
		if ((prom->options & OPTIONS_PULLUPS) != 0) {
			out(pin, 1);
		} else {
			out(pin, 0);
		}
		ddr(pin, 0);
	}

	// Configure all of the hi and low pins as outputs.
	// Do the low pins first to bring them to ground potential,
	// then the high pins.
	for (uint8_t i = 0 ; i < array_count(prom->lo_pins) ; i++)
	{
		uint8_t pin = prom_pin(prom->lo_pins[i]);
		if (pin == 0)
			continue;
		out(pin, 0);
		ddr(pin, 1);
	}

	for (uint8_t i = 0 ; i < array_count(prom->hi_pins) ; i++)
	{
		uint8_t pin = prom_pin(prom->hi_pins[i]);
		if (pin == 0)
			continue;
		out(pin, 1);
		ddr(pin, 1);
	}


	// Let things stabilize for a little while
	_delay_ms(250);

	// If this is an AVR ISP chip, try to go into programming mode
	if (prom->data_width == 0)
		isp_setup();
}


/** Switch all of the ZIF pins back to tri-state to make it safe.
 * Doesn't matter what PROM is inserted.
 */
static void
prom_tristate(void)
{
	for (uint8_t i = 1 ; i <= ZIF_PINS ; i++)
	{
		ddr(ports[i], 0);
		out(ports[i], 0);
	}
}


/** Select a 32-bit address for the current PROM */
static void
prom_set_address(
	uint32_t addr
)
{
	for (uint8_t i = 0 ; i < prom->addr_width ; i++)
	{
		out(prom_pin(prom->addr_pins[i]), addr & 1);
		addr >>= 1;
	}
}


static uint8_t
_prom_read(void)
{
	uint8_t b = 0;
	for (uint8_t i = 0 ; i < prom->data_width  ; i++)
	{
		uint8_t bit = in(prom_pin(prom->data_pins[i])) ? 0x80 : 0;
		b = (b >> 1) | bit;
	}

	return b;
}


/** Read a byte from the PROM at the specified address..
 * \todo Update this to handle wider than 8-bit PROM chips.
 */
static uint8_t
prom_read(
	uint32_t addr
)
{
	if (prom->data_width == 0)
		return isp_read(addr);

	uint8_t latch = (prom->options & OPTIONS_LATCH) != 0;
	uint8_t latch_pin = prom_pin(prom->lo_pins[LATCH_PIN]);
	if (latch) {
		out(latch_pin,1);
	}
	prom_set_address(addr);
	if (latch) {
		out(latch_pin,0);
	}
	for(uint8_t i = 0 ; i < 255; i++)
	{
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
	}

	uint8_t old_r = _prom_read();

	// Try reading a few times to be sure,
	// or until the values converge
	for (uint8_t i = 0 ; i < 8 ; i++)
	{
		uint8_t r = _prom_read();
		if (r == old_r)
			break;
		old_r = r;
	}

	return old_r;
}


static uint8_t
usb_serial_getchar_echo(void)
{
	while (1)
	{
		uint16_t c = Serial.read();
		if (c == -1)
			continue;
		Serial.print((char) c);
		return c;
	}
}

static uint8_t
hexdigit_parse(
	uint8_t c
)
{
	if ('0' <= c && c <= '9')
		return c - '0';
	if ('A' <= c && c <= 'F')
		return c - 'A' + 0xA;
	if ('a' <= c && c <= 'f')
		return c - 'a' + 0xA;
	return 0xFF;
}

static void
hex32(
	uint8_t * buf,
	uint32_t addr
)
{
	buf[7] = hexdigit(addr & 0xF); addr >>= 4;
	buf[6] = hexdigit(addr & 0xF); addr >>= 4;
	buf[5] = hexdigit(addr & 0xF); addr >>= 4;
	buf[4] = hexdigit(addr & 0xF); addr >>= 4;
	buf[3] = hexdigit(addr & 0xF); addr >>= 4;
	buf[2] = hexdigit(addr & 0xF); addr >>= 4;
	buf[1] = hexdigit(addr & 0xF); addr >>= 4;
	buf[0] = hexdigit(addr & 0xF); addr >>= 4;
}


static void
hexdump(
	uint32_t addr
)
{
	char buf[80];
	hex32(buf, addr);

	for (int i = 0 ; i < 16 ; i++)
	{
		uint8_t w = prom_read(addr++);
		uint8_t x = 8 + i * 3;
		buf[x+0] = ' ';
		buf[x+1] = hexdigit(w >> 4);
		buf[x+2] = hexdigit(w >> 0);

		buf[8 + 16*3 + i + 2] = printable(w) ? w : '.';
	}

	buf[8 + 16 * 3 + 0] = ' ';
	buf[8 + 16 * 3 + 1] = ' ';
	buf[8 + 16 * 3 + 18] = '\r';
	buf[8 + 16 * 3 + 19] = '\n';
	buf[8 + 16 * 3 + 20] = '\0';

	Serial.print(buf);
}


/** Read an address from the serial port, then read that from the PROM */
static void
read_addr(char* buffer)
{
	uint32_t addr = 0;
	uint8_t buf_idx = 0;
	while (1)
	{
	  uint8_t c = buffer[buf_idx++];
	  if (c == '\0')
	    break;
	  uint8_t n = hexdigit_parse(c);
	  if (n == 0xFF)
	    goto error;
	  
	  addr = (addr << 4) | n;
	}

	Serial.println();

	prom_setup();

	for (uint8_t line = 0 ; line < 4 ; line++)
	{
		hexdump(addr);
		addr += 16;
	}
	return;

error:
	Serial.println("?");
}


/** Send a single prom name to the serial port */
static void
prom_list_send(
	int mode,
	const prom_t * const prom,
	int selected
)
{
	uint8_t buf[32];
	uint8_t off = 0;
	if (selected)
	{
		buf[off++] = '*';
		buf[off++] = '*';
		buf[off++] = '*';
		buf[off++] = ' ';
	}
	if (mode >= 16) {
	  buf[off++] = hexdigit(mode / 16);
	}
	buf[off++] = hexdigit(mode % 16);
	buf[off++] = ' ';
	memcpy(buf+off, prom->name, sizeof(prom->name));
	off += sizeof(prom->name);
	buf[off++] = '\r';
	buf[off++] = '\n';
	buf[off++] = '\0';

	Serial.print(buf);
}


/** List all of the PROM models supported */
static void
prom_list(void)
{
	for (unsigned i = 0 ; i < proms_count ; i++)
	{
		const prom_t * const p = &proms[i];
		prom_list_send(i, p, p == prom );
	}
}

static void
prom_mode(char* buffer)
{
  for (int i = 0; i < proms_count; i++) {
    const char* a = proms[i].name;
    const char* b = buffer;
    char match = 1;
    while (*a != '\0' && *b != '\0') {      
      if (*a != *b) { match = 0; break; }
      a++; b++;
    }
    if (match) {
	prom = &proms[i];
	prom_list_send(i, prom, 1);
	return;
    }
  }    
  Serial.println("- No such chip");
}


/**
 * Scan the current chip against the EPROM definition given.
 * A "successful" scan should yield:
 * - Different data on each data pin
 * - Consistent data across multiple scans
 * - A check on high memory to ensure to disambiguate different
 *   memory grades on the same/similar pinouts
 * Return 1 on success, 0 otherwise.
 */
static unsigned scan(const prom_t* use_prom) {
  prom = use_prom;
  prom_setup();
  // scan first 256 bytes for varying data
  uint8_t zeros = 0;
  uint8_t ones = 0;
  uint8_t block[16];
  for (uint32_t addr = 0; addr < 256; addr += 16) {
    for (uint8_t i = 0; i < 16; i++) {
      block[i] = prom_read(addr+i);
      zeros |= ~block[i];
      ones |= block[i];
    }
    // reread and confirm
    for (uint8_t i = 0; i < 16; i++) {
      if (block[i] != prom_read(addr+i)) {
	return 0;
      }
    }
  }
  // ensure that we're not just getting the same bits again and again
  if (ones != 0xff || zeros != 0xff) { return 0; }
  // check top half of memory. If first 256 bytes mirrors low memory
  // or is the same byte, consider it a failure.
  const uint32_t top_half_addr = (((uint32_t) 1) << prom->addr_width) >> 1;
  uint8_t single_byte = prom_read(top_half_addr);
  uint8_t same_byte_check = 1;
  uint8_t same_data_check = 1;
  for (uint8_t i = 0; i < 256; i++) {
    uint8_t low = prom_read(i);
    uint8_t high = prom_read(top_half_addr+i);
    if (high != single_byte) { same_byte_check = 0; }
    if (low != high) { same_data_check = 0; }
  }
  if (same_data_check || same_byte_check) { return 0; }
  return 1;
}

/**
 * Automatically scan all known EPROM types and attempt to construct a list of candidates.
 */
static void autoscan(void) {
  prom_tristate();
  for (int i = 0; i < proms_count; i++) {
    if (scan(proms+i)) {
      prom_list_send(i, prom, 1);
    }
    prom_tristate();
  }
}

static xmodem_block_t xmodem_block;

/** Send the entire PROM memory via xmodem */
static void
prom_send(void)
{
	if (xmodem_init(&xmodem_block) < 0)
		return;

	// Ending address
	const uint32_t end_addr = (((uint32_t) 1) << prom->addr_width) - 1;

	// Bring the pins up to level
	prom_setup();

	// Start sending!
	uint32_t addr = 0;
	while (1)
	{
		for (uint8_t off = 0 ; off < sizeof(xmodem_block.data) ; off++)
			xmodem_block.data[off] = prom_read(addr++);

		if (xmodem_send(&xmodem_block) < 0)
			return;

		// If we have wrapped the address, we are done
		if (addr >= end_addr)
			break;
	}

	xmodem_fini(&xmodem_block);
}




int main(void)
{
	// Disable the ADC
	ADMUX = 0;

	Serial.begin(115200);

	#define MAX_CMD 64
	char buffer[MAX_CMD];
	uint8_t buf_idx = 0;
	while (1)
	{
		// always put the PROM into tristate so that it is safe
		// to swap the chips in between readings, and 
		prom_tristate();
		Serial.print("> ");

		buf_idx = 0;
		buffer[buf_idx] = 0;
		while (1)
		{
		  // read in a line, processing on a newline, return, or
		  // xmodem transfer nak
		  char c = usb_serial_getchar_echo();
		  if (c == XMODEM_NAK) { buffer[0] = XMODEM_NAK; buf_idx=1; break; }
		  if (c == '\n') { Serial.print("\r"); break; }
		  if (c == '\r') { Serial.print("\n"); break; }
		  if (buf_idx < (MAX_CMD-1)) buffer[buf_idx++] = c;
		}
		buffer[buf_idx] = 0;
		// process command
		switch(buffer[0]) {
		case XMODEM_NAK: prom_send(); break;
		case 'r': read_addr(buffer+1); break;
		case 'l': prom_list(); break;
		case 'm': prom_mode(buffer+1); break;
		case 'i': isp_read(0); break;
		case 's': autoscan(); break;
		case '\n': break;
		case '\r': break;
		default:
			Serial.print(
"r000000 Read a hex word from address\r\n"
"l       List chip modes\r\n"
"mTYPE   Select chip TYPE\r\n"
"s       Autoscan for chip type (POTENTIALLY DANGEROUS)\r\n"
			);
			break;
		}
	}
}
