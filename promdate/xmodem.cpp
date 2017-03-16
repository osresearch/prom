/**
 * \file xmodem protocol
 *
 * Using USB serial
 */

#include <Arduino.h>
#include "WProgram.h"
//#include <avr/io.h>
//#include <avr/pgmspace.h>
//#include <avr/interrupt.h>
#include <stdint.h>
#include <string.h>
//#include <util/delay.h>
#include "xmodem.h"



/** Send a block.
 * Compute the checksum and complement.
 *
 * \return 0 if all is ok, -1 if a cancel is requested or more
 * than 10 retries occur.
 */
int
xmodem_send(
	xmodem_block_t * const block
)
{
	// Compute the checksum and complement
	uint8_t cksum = 0;
	for (uint8_t i = 0 ; i < sizeof(block->data) ; i++)
		cksum += block->data[i];

	block->cksum = cksum;
	block->block_num++;
	block->block_num_complement = 0xFF - block->block_num;

	// Send the block, and wait for an ACK
	uint8_t retry_count = 0;

	while (retry_count++ < 10)
	{
		Serial.write((const uint8_t*) block, sizeof(*block));

		// Wait for an ACK (done), CAN (abort) or NAK (retry)
		while (1)
		{
			uint16_t c = Serial.read();
			if (c == -1)
				continue;
			if (c == XMODEM_ACK)
				return 0;
			if (c == XMODEM_CAN)
				return -1;
			if (c == XMODEM_NAK)
				break;
		}
	}

	// Failure or cancel
	return -1;
}


int
xmodem_init(
	xmodem_block_t * const block
)
{
	block->soh = 0x01;
	block->block_num = 0x00;

	// wait for initial nak
	while (1)
	{
		uint16_t c = Serial.read();
		if (c == -1)
			continue;
		if (c == XMODEM_NAK)
			return 0;
		if (c == XMODEM_CAN)
			return -1;
	}
}


int
xmodem_fini(
	xmodem_block_t * const block
)
{
#if 0
/* Don't send EOF?  rx adds it to the file? */
	block->block_num++;
	memset(block->data, XMODEM_EOF, sizeof(block->data));
	if (xmodem_send_block(block) < 0)
		return;
#endif

	// File transmission complete.  send an EOT
	// wait for an ACK or CAN
	while (1)
	{
		Serial.print((char) XMODEM_EOT);

		while (1)
		{
			uint16_t c = Serial.read();
			if (c == -1)
				continue;
			if (c == XMODEM_ACK)
				return 0;
			if (c == XMODEM_CAN)
				return -1;
		}
	}
}
