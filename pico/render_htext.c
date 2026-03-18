#include "render.h"

#include <pico/stdlib.h>
#include "buffers.h"
#include "colors.h"
#include "htextfont/htextfont.h"
#include "vga.h"

#define HTEXT_WIDTH 640
#define HTEXT_HEIGHT 480
#define HTEXT_COLS 40 // 80 in soft_80col mode
#define HTEXT_ROWS 24
#define HTEXT_GLYPH_WIDTH 7 // pixel doubling in 40 column mode,
#define HTEXT_GLYPH_HEIGHT 16
#define HTEXT_GLYPH_SIZE ((HTEXT_GLYPH_WIDTH/8)*HTEXT_GLYPH_HEIGHT) // 16 for ascii, 32 for hangul glyph
#define HTEXT_VIEWPORT_WIDTH (HTEXT_COLS * HTEXT_GLYPH_WIDTH)
#define HTEXT_VIEWPORT_HEIGHT (HTEXT_ROWS * HTEXT_GLYPH_ROWS)
#define HTEXT_LEFT_PADDING ((HTEXT_WIDTH - HTEXT_VIEWPORT_WIDTH)/2) // (640-40*7*2)/2=(640-80*7)/2=40
#define HTEXT_TOP_PADDING ((HTEXT_HEIGHT - HTEXT_VIEWPORT_HEIGHT)/2) // (480-24*16)/2=48

const uint16_t htext_base_addr[] = {
	0x0000,0x0080,0x0100,0x0180,0x0200,0x0280,0x0300,0x0380,
	0x0028,0x00A8,0x0128,0x01A8,0x0228,0x02A8,0x0328,0x03A8,
	0x0050,0x00D0,0x0150,0x01D0,0x0250,0x02D0,0x0350,0x03D0,
};

// pointer to glyph data(16bytes for 7x16) glyph for each column in the current text line
uint8_t* htext_line_buf[80]; // 40 or 80

// character codes -> glyph data pointers
void prepare_htext_line_buf(uint num_cols, const uint8_t* line_main, const uint8_t* line_aux)
{
    for (uint col = 0, col_offset = 0; col < num_cols; col++) {
      uint_fast8_t char_a = ((col & 1) || !line_aux) ? line_main[col_offset++] : line_aux[col_offset];

      // apple ii text mode
      // https://en.wikipedia.org/wiki/Apple_II_character_set
      // 0x00~0x3f 0b00xxxxxx inverse @ABC...012...
      // 0x40~0x7f 0b01xxxxxx flash @ABC...012... or mousetext and `abc...012...(//e alt chr)
      // 0x80~0xff 0b1xxxxxxx normal
      if (char_a >= 0x4c && char_a <= 0x78) {
        // 0x40..0x4b: reserved
        // 0x4c..0x77 -> +0x60 -> 0xac..0xd7 : unicode hangul syllables
        // 0x78 -> 0x11 : unicode hangul jamo
        // 0x79..0x7e: reserved
        if (char_a == 0x78) {
          char_a = 0x11;
        } else {
          char_a += 0x60;
        }
        // get right half char
        col++;
        uint_fast8_t char_b = ((col & 1) || !line_aux) ? line_main[col_offset++] : line_aux[col_offset];
        // glyph must be column-major
        uint8_t* glyph = get_han_glyph((char_a << 8) | char_b);
        htext_line_buf[col-1] = glyph; // left half glyph
        htext_line_buf[col] = glyph + 16; // right half glyph
      } else if (char_a & 0x80) {
        // 0x80..0xff -> msb off! -> 0x00..0x7f
        htext_line_buf[col] = get_asc_glyph(char_a & 0x7f);
      } else {
        // 0x00..0x3f: inverse characters
        htext_line_buf[col] = get_asc_glyph(char_a < 0x20 ? char_a + 'A' : char_a);
      }
    }
}

// for 40/80 text only mode
void render_htext_line_buf(uint16_t* sl_data16, uint num_cols, uint glyph_line, uint_fast16_t fg, uint_fast16_t bg) {
    for(uint col = 0; col < num_cols; col++) {
        const uint_fast8_t bits = htext_line_buf[col][glyph_line];
        *sl_data16++ = bits & 0b01000000 ? fg : bg;
        *sl_data16++ = bits & 0b00100000 ? fg : bg;
        *sl_data16++ = bits & 0b00010000 ? fg : bg;
        *sl_data16++ = bits & 0b00001000 ? fg : bg;
        *sl_data16++ = bits & 0b00000100 ? fg : bg;
        *sl_data16++ = bits & 0b00000010 ? fg : bg;
        *sl_data16++ = bits & 0b00000001 ? fg : bg;
    }
}

// for 40col text overlay mode
// 1 byte -> 7pixels -> 7 uint32_t(16bpp * pixel doubling) * 40 = 280 uint32_t per line
void render_htext40_line_buf_overlay(uint32_t* sl_data32, uint glyph_line, uint_fast32_t fgfg)
{
    for(uint col = 0; col < 40; col++) {
        const uint_fast8_t bits = htext_line_buf[col][glyph_line];
        if (bits & 0b01000000) { *sl_data32++ = fgfg; } else { sl_data32++; }
        if (bits & 0b00100000) { *sl_data32++ = fgfg; } else { sl_data32++; }
        if (bits & 0b00010000) { *sl_data32++ = fgfg; } else { sl_data32++; }
        if (bits & 0b00001000) { *sl_data32++ = fgfg; } else { sl_data32++; }
        if (bits & 0b00000100) { *sl_data32++ = fgfg; } else { sl_data32++; }
        if (bits & 0b00000010) { *sl_data32++ = fgfg; } else { sl_data32++; }
        if (bits & 0b00000001) { *sl_data32++ = fgfg; } else { sl_data32++; }
    }
}

// for 80col text overlay mode
// 1 byte -> 7pixels -> 7 uint16_t(16bpp) * 80 = 280 uint32_t per line
void render_htext80_line_buf_overlay(uint16_t* sl_data16, uint glyph_line, uint_fast16_t fg)
{
    for(uint col = 0; col < 80; col++) {
        const uint_fast8_t bits = htext_line_buf[col][glyph_line];
        if (bits & 0b01000000) { *sl_data16++ = fg; } else { sl_data16++; }
        if (bits & 0b00100000) { *sl_data16++ = fg; } else { sl_data16++; }
        if (bits & 0b00010000) { *sl_data16++ = fg; } else { sl_data16++; }
        if (bits & 0b00001000) { *sl_data16++ = fg; } else { sl_data16++; }
        if (bits & 0b00000100) { *sl_data16++ = fg; } else { sl_data16++; }
        if (bits & 0b00000010) { *sl_data16++ = fg; } else { sl_data16++; }
        if (bits & 0b00000001) { *sl_data16++ = fg; } else { sl_data16++; }
    }
}

void render_htext_line(unsigned int line, bool force_monocolor) {
    uint_fast16_t bg_color = (soft_monochrom | force_monocolor) ? mono_bg_color : ntsc_palette[0];
    uint_fast16_t fg_color = (soft_monochrom | force_monocolor) ? mono_fg_color : ntsc_palette[15];

    const uint line_offset = htext_base_addr[line];

    const uint8_t *page_main = is_page2_display_enabled() ? text_mainmem_page2 : text_mainmem_page1;
    const uint8_t *line_main = page_main + line_offset;

    const uint8_t *line_aux = 0;
    uint num_cols;
    if(soft_80col) {
        // Read every other character from the aux memory bank in 80 column mode
        const uint8_t *page_aux = is_page2_display_enabled() ? text_auxmem_page2 : text_auxmem_page1;
        line_aux = page_aux + line_offset;
        num_cols = 80;
    } else {
        // pixel doubling
        fg_color |= THEN_EXTEND_1;
        bg_color |= THEN_EXTEND_1;
        num_cols = 40;
    }

    prepare_htext_line_buf(num_cols, line_main, line_aux);

    for(uint glyph_line = 0; glyph_line < 16; glyph_line++) {
        struct vga_scanline *sl = vga_prepare_scanline();
        uint sl_pos = 0;

        // Pad 40 pixels on the left to center horizontally
        sl->data[sl_pos++] = (0 | THEN_EXTEND_7) | ((0 | THEN_EXTEND_7) << 16);  // 16 pixels
        sl->data[sl_pos++] = (0 | THEN_EXTEND_7) | ((0 | THEN_EXTEND_7) << 16);  // 16 pixels
        sl->data[sl_pos++] = (0 | THEN_EXTEND_3) | ((0 | THEN_EXTEND_3) << 16);  // 8 pixels

        render_htext_line_buf((uint16_t*)(sl->data + sl_pos), num_cols, glyph_line, fg_color, bg_color);
        sl_pos += num_cols * 7 / 2; // 7 pixels per column, 2 pixels per uint32_t

        sl->length = sl_pos; // 3 + (num_cols * 7 / 2)
        vga_submit_scanline(sl);
    }
}

void render_htext() {
    vga_prepare_frame();
    // Skip 48 lines to center vertically
    vga_skip_lines(48);

    for(int line = 0; line < 24; line++) {
        render_htext_line(line, soft_force_alt_textcolor);
    }
}
