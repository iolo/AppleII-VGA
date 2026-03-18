#pragma once

#include <stdint.h>

void set_asc_font(uint8_t *font);

uint8_t* get_asc_glyph(uint8_t code);

void set_han_font(uint8_t *font);

uint8_t* get_han_glyph(uint16_t code);
