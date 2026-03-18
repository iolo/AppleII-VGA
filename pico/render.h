#pragma once

#include <stdbool.h>
#include <stdint.h>


// Uncomment to enable test pattern generator
// #define RENDER_TEST_PATTERN

extern void render_init();
extern void render_loop();

extern void render_vga_testpattern();

extern void update_text_flasher();
extern void render_text();
extern void render_text_line(unsigned int line, bool force_monocolor);

extern void render_lores();
extern void render_mixed_lores();

extern void generate_hires_tables();
extern void render_hires(bool mixed);

//@@iolo
extern const uint16_t htext_base_addr[];
extern void prepare_htext_line_buf(unsigned int num_cols, const uint8_t* line_main, const uint8_t* line_aux);
extern void render_htext_line_buf(uint16_t* sl_data16, unsigned int num_cols, unsigned int glyph_line, uint_fast16_t fg, uint_fast16_t bg);
extern void render_htext40_line_buf_overlay(uint32_t* sl_data32, unsigned int glyph_line, uint_fast32_t fgfg);
extern void render_htext80_line_buf_overlay(uint16_t* sl_data16, unsigned int glyph_line, uint_fast16_t fg);
extern void render_htext_line(unsigned int line, bool force_monocolor);
extern void render_htext();
extern void render_vtext();
//@@
