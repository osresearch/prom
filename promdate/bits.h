#ifndef _prom_bits_h_
#define _prom_bits_h_

#include <stdint.h>

#define sbi(PORT, PIN) ((PORT) |=  (1 << (PIN)))
#define cbi(PORT, PIN) ((PORT) &= ~(1 << (PIN)))
#define array_count(ARRAY) (sizeof(ARRAY) / sizeof(*(ARRAY)))
#define _STR(X) #X
#define STR(X) _STR(X)

#ifdef __cplusplus
extern "C" {
#endif

void ddr(
	const uint8_t port,
	const uint8_t value
);


void out(
	const uint8_t port,
	uint8_t value
);


uint8_t in(
	uint8_t port
);

#ifdef __cplusplus
}
#endif

#endif
