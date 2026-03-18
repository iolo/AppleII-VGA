#include "render.h"

#include <pico/stdlib.h>
#include <string.h>
#include "buffers.h"
#include "colors.h"
#include "htextfont/htextfont.h"
#include "vga.h"

// virtual hi-resolution color text mode
//
// 80x25 characters
// 2 bytes per character: first byte for character code, second byte for attribute
// continuous 80*25*2 = 4000 bytes for character data
// similar to [vga text mode](https://en.wikipedia.org/wiki/VGA_text_mode)
// MSB of character code byte is used to determine whether it's an ascii character or a hangul character
// when MSB is 0, it's an ascii character
// when MSB is 1, it's a first byte of hangul character; the next byte is the second byte of the hangul character
// ascii character is 8x16 pixels(single width)
// hangul character is 16x16 pixels(double width); 1 hangul character occupies the space of 2 ascii characters
// attribute byte: foreground color, background color, and some bits for intensity/blink/underline
// framebuffer starts from 0x2000..(80*25*2=4000 bytes; 0x2000..0x2FFF)
//
// for example,
// 0x2000: 0x41 0x0F  // ascii 'A' with attribute 0x0F (white on black)
// 0x2002: 0xAC 0xF0 0x00 0xF0 // unicode '가' with attribute 0xF0 (black on white)
#define VTEXT_BANK main_memory
#define VTEXT_BASE 0x2000
#define VTEXT_WIDTH 640
#define VTEXT_HEIGHT 400
#define VTEXT_COLS 580
#define VTEXT_ROWS 25
#define VTEXT_GLYPH_WIDTH 8
#define VTEXT_GLYPH_HEIGHT 16
#define VTEXT_GLYPH_SIZE ((VTEXT_GLYPH_WIDTH/8)*VTEXT_GLYPH_HEIGHT) // 16 for ascii, 32 for hangul glyph
#define VTEXT_VIEWPORT_WIDTH (VTEXT_COLS*VTEXT_GLYPH_WIDTH) // 80*8=640
#define VTEXT_VIEWPORT_HEIGHT (VTEXT_ROWS*VTEXT_GLYPH_HEIGHT) // 25*16=400 line doubling
#define VTEXT_TOP_PADDING ((VTEXT_HEIGHT-VTEXT_VIEWPORT_HEIGHT)/2) // 0
#define VTEXT_LEFT_PADDING ((VTEXT_WIDTH-VTEXT_VIEWPORT_WIDTH)/2) // 80/40

// color index(high/low nibble of attribute byte) to rgb333 values
static const uint16_t* colors = ntsc_palette;

static uint8_t* vtext_line_buf[VTEXT_COLS];

void render_vtext_row(uint row) {
    uint addr = VTEXT_BASE + row * VTEXT_COLS;
    for (uint col = 0; col < VTEXT_COLS;) {
      uint8_t code = VTEXT_BANK[addr + col*2];
      if (code & 0x80) {
        // hangul (double width)
        uint8_t code2 = VTEXT_BANK[addr + col*2 + 2];
        uint8_t* glyph = get_han_glyph((code << 8) | code2);
        vtext_line_buf[col++] = glyph; // left half
        vtext_line_buf[col++] = glyph + VTEXT_GLYPH_SIZE; // right half
      } else {
        // ascii (single width)
        vtext_line_buf[col++] = get_asc_glyph(code);
      }
    }

    for(int glyph_line = 0; glyph_line < VTEXT_GLYPH_HEIGHT; glyph_line++) {
      struct vga_scanline* sl = vga_prepare_scanline();

      uint16_t* sl_data = (uint16_t*)(sl->data);
      for (int col = 0; col < VTEXT_COLS; col++) {
        uint_fast8_t attr = VTEXT_BANK[addr + col*2 + 1];
        uint_fast8_t fg = colors[attr & 0x0f];
        uint_fast8_t bg = colors[attr >> 4];
        uint_fast8_t bits = vtext_line_buf[col][glyph_line];
        *sl_data++ = (bits & 0b10000000) ? fg : bg;
        *sl_data++ = (bits & 0b01000000) ? fg : bg;
        *sl_data++ = (bits & 0b00100000) ? fg : bg;
        *sl_data++ = (bits & 0b00010000) ? fg : bg;
        *sl_data++ = (bits & 0b00001000) ? fg : bg;
        *sl_data++ = (bits & 0b00001000) ? fg : bg;
        *sl_data++ = (bits & 0b00000100) ? fg : bg;
        *sl_data++ = (bits & 0b00000010) ? fg : bg;
        *sl_data++ = (bits & 0b00000001) ? fg : bg;
      }

      sl->length = VTEXT_COLS * (VTEXT_GLYPH_WIDTH >> 1); // 640 pixels = 320 uint32
      //sl->repeat_count = 1; // line doubling!
      vga_submit_scanline(sl);
    }
}

void render_vtext() {
    vga_prepare_frame();

    if (VTEXT_TOP_PADDING > 0) {
      vga_skip_lines(VTEXT_TOP_PADDING);
    }

    for(uint row = 0; row < VTEXT_ROWS; row++) {
      render_vtext_row(row);
    }
}

