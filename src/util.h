#ifndef CHIP8_UTILS_H
#define CHIP8_UTILS_H

#include <cstdint>
#include <stdio.h>
#include <iostream>

typedef uint8_t u08;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f16;
typedef float f32;
typedef double f64;

inline void dump_bytes(const u08 *bytes, u64 count, std::ostream &stream) {
	for (u64 i = 0; i < count; i++) {
		stream << std::hex << bytes[i];
	}
}

#include <Windows.h>
inline void beep() {
	Beep(440, 100);
}

#endif // CHIP8_UTILS_H