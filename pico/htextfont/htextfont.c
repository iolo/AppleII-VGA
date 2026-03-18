#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "htextfont.h"

extern const uint8_t asc_wcomm[];
extern const uint8_t han_wcomm[];

// 8x16 pixels
// 16 bytes per glyph
// 256 * 16 = 2048 bytes total
#define ASC_GLYPH_SIZE 16
#define NUM_ASC_GLYPHS 128
#define ASC_FONT_SIZE (NUM_ASC_GLYPHS * ASC_GLYPH_SIZE)

const uint8_t *asc_font = asc_wcomm;

inline void set_asc_font(uint8_t *font) {
  memcpy((void*)asc_font, font ? font : asc_wcomm, ASC_FONT_SIZE);
}

inline uint8_t* get_asc_glyph(uint8_t code)
{
    return (uint8_t*)(asc_font + (code * ASC_GLYPH_SIZE));
}

#define HAN_CODE_UNICODE
//#define HAN_CODE_JOHAB
//#define HAN_FONT_844
#define HAN_FONT_621

// 16x16 pixels
// 32 bytes per glyph
#define HAN_GLYPH_SIZE 32

#define FILLER 1

// filler,ㄱ, ㄲ, ㄴ, ㄷ, ㄸ, ㄹ, ㅁ, ㅂ, ㅃ, ㅅ, ㅆ, ㅇ, ㅈ, ㅉ, ㅊ, ㅋ, ㅌ, ㅍ, ㅎ
#define NUM_INITIAL (19+FILLER)
// filler, ㅏ, ㅐ, ㅑ, ㅒ, ㅓ, ㅔ, ㅕ, ㅖ, ㅗ, ㅘ, ㅙ, ㅚ, ㅛ, ㅜ, ㅝ, ㅞ, ㅟ, ㅠ, ㅡ, ㅢ, ㅣ
#define NUM_MEDIAL (21+FILLER)
// filler, ㄱ, ㄲ, ㄳ, ㄴ, ㄵ, ㄶ, ㄷ, ㄹ, ㄺ, ㄻ, ㄼ, ㄽ, ㄾ, ㄿ, ㅀ, ㅁ, ㅂ, ㅄ, ㅅ, ㅆ, ㅇ, ㅈ, ㅊ, ㅋ, ㅌ, ㅍ, ㅎ
#define NUM_FINAL (27+FILLER)

#ifdef HAN_FONT_844
// 8 * 20 + 4 * 22 + 4 * 28 = 360 glyphs
// 360 * 32 = 11520 bytes total
#define INITIAL_GLYPH_KINDS 8
#define MEDIAL_GLYPH_KINDS 4
#define FINAL_GLYPH_KINDS 4
#endif

#ifdef HAN_FONT_621
// 6*20 + 2*22 + 1*28 = 120 + 44 + 28 = 192 glyphs
// 192 * 32 = 6144 bytes total
#define INITIAL_GLYPH_KINDS 6
#define MEDIAL_GLYPH_KINDS 2
#define FINAL_GLYPH_KINDS 1
#endif

#define NUM_HAN_GLYPHS INITIAL_GLYPH_KINDS * NUM_INITIAL + MEDIAL_GLYPH_KINDS * NUM_MEDIAL + FINAL_GLYPH_KINDS * NUM_FINAL
#define HAN_FONT_SIZE (NUM_HAN_GLYPHS * HAN_GLYPH_SIZE)

const uint8_t *han_font = han_wcomm;

inline void set_han_font(uint8_t *font) {
  memcpy((void*)han_font, font ? font : han_wcomm, HAN_FONT_SIZE);
}

#ifdef HAN_CODE_UNICODE
//
// unicode support
//

// hangul syllables(0xac00..0xd7a3)
#define HANGUL_SYLLABLE_BASE 0xAC00 // 가
#define HANGUL_SYLLABLE_END 0xD7A3 // 힣

// unicode hangul jamo(0x1100..0x11ff)
#define HANGUL_JAMO_BASE 0x1100
#define HANGUL_JAMO_INITIAL_BASE 0x1100
#define HANGUL_JAMO_INITIAL_END HANGUL_JAMO_INITIAL_BASE + NUM_INITIAL - FILLER
#define HANGUL_JAMO_MEDIAL_BASE 0x1112
#define HANGUL_JAMO_MEDIAL_END HANGUL_JAMO_MEDIAL_BASE + NUM_MEDIAL - FILLER
#define HANGUL_JAMO_FINAL_BASE 0x11a8
#define HANGUL_JAMO_FINAL_END HANGUL_JAMO_FINAL_BASE + NUM_FINAL - FILLER
#define HANGUL_JAMO_END 0x11ff

inline bool is_hangul_syllable(int code) {
  return HANGUL_SYLLABLE_BASE <= code && code <= HANGUL_SYLLABLE_END;
}

inline int compose_hangul_syllable(int initial, int medial, int final) {
  if (!initial || !medial) {
    if (!initial) {
      return HANGUL_JAMO_INITIAL_BASE + initial - FILLER;
    }
    if (!medial) {
      return HANGUL_JAMO_MEDIAL_BASE + medial - FILLER;
    }
    if (!final) {
      return HANGUL_JAMO_FINAL_BASE + final - FILLER;
    }
    return 0;
  }
  return HANGUL_SYLLABLE_BASE + (((initial - FILLER) * (NUM_MEDIAL - FILLER)) + (medial - FILLER)) * NUM_FINAL + final;
}

inline void decompose_hangul_syllable(int code, int* initial, int* medial, int* final) {
  if (code >= HANGUL_JAMO_BASE && code <= HANGUL_JAMO_END) {
    *initial = *medial = *final = 0;
    if (code >= HANGUL_JAMO_INITIAL_BASE && code < HANGUL_JAMO_INITIAL_END) {
      *initial = code - HANGUL_JAMO_INITIAL_BASE + FILLER;
      return;
    }
    if (code >= HANGUL_JAMO_MEDIAL_BASE && code < HANGUL_JAMO_MEDIAL_END) {
      *medial = code - HANGUL_JAMO_MEDIAL_BASE + FILLER;
      return;
    }
    if (code >= HANGUL_JAMO_FINAL_BASE && code < HANGUL_JAMO_FINAL_END) {
      *final = code - HANGUL_JAMO_FINAL_BASE + FILLER;
      return;
    }
  }

  int index = code - HANGUL_SYLLABLE_BASE;
  *final = index % NUM_FINAL;
  *medial = (index / NUM_FINAL) % (NUM_MEDIAL - FILLER) + FILLER;
  *initial = (index / (NUM_MEDIAL - FILLER)) / NUM_FINAL + FILLER;
}
#endif

#ifdef HAN_CODE_JOHAB
//
// johab hangul code(aka. KSSM) support
//

// 5-bit johab jamo code <-> jamo index
const uint8_t initial_code_to_index[32] = {
  0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const uint8_t initial_index_to_code[NUM_INITIAL] = {
  0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19
};

const uint8_t medial_code_to_index[32] = {
  0, 0, 0, 1, 2, 3, 4, 5, 0, 0, 6, 7, 8, 9, 10, 11,
  0, 0, 12, 13, 14, 15, 16, 17, 0, 0, 18, 19, 20, 21, 0, 0
};

const uint8_t medial_index_to_code[NUM_MEDIAL] = {
  0, 3, 4, 5, 6, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21
};

const uint8_t final_code_to_index[32] = {
  0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
  15, 16, 0, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 0, 0
};

const uint8_t final_index_to_code[NUM_FINAL] = {
  0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
  17, 18, 19, 20, 21, 22, 23, 24, 25, 26
};

inline bool is_hangul_syllable(int code) {
  return (code & 0x8000) != 0;
}

inline int compose_hangul_syllable(int initial, int medial, int final) {
  if (initial > NUM_INITIAL || medial > NUM_MEDIAL || final > NUM_FINAL) {
    return 0; // invalid input
  }
  return 0x8000 |
    (initial_index_to_code[initial] << 10) |
    (initial_index_to_code[medial] << 5) |
    initial_index_code[final];
}

inline void decompose_hangul_syllable(int code, int* initial, int* medial, int* final) {
  *initial = initial_code_to_index[(code >> 10) & 0b11111];
  *medial = medial_code_to_index[(code >> 5) & 0b11111];
  *final = final_code_to_index[code & 0b11111];
}
#endif

#define INITIAL_GLYPH_SIZE (INITIAL_GLYPH_KINDS * NUM_INITIAL * HAN_GLYPH_SIZE)
#define MEDIAL_GLYPH_SIZE (MEDIAL_GLYPH_KINDS * NUM_MEDIAL * HAN_GLYPH_SIZE)
#define FINAL_GLYPH_SIZE (FINAL_GLYPH_KINDS * NUM_FINAL * HAN_GLYPH_SIZE)

#define INITIAL_GLYPH_BASE 0
#define MEDIAL_GLYPH_BASE (INITIAL_GLYPH_BASE + INITIAL_GLYPH_SIZE)
#define FINAL_GLYPH_BASE (MEDIAL_GLYPH_BASE + MEDIAL_GLYPH_SIZE)

#ifdef HAN_FONT_844
//
// 8x4x4 hangul syllable composition rules:
// - initial consonants: 8 kinds = 4 kinds by meidial x 2 kinds by final existence = 8 kinds
// - medial vowels: 4 kinds = 2 kinds by initial x 2 kinds by final existence
// - final consonants: 4 kinds by midial
//

const uint8_t kind_initial_by_medial_without_final[NUM_MEDIAL] = {
  // ㅏ ㅐ ㅑ ㅒ ㅓ ㅔ ㅕ ㅖ ㅗ ㅘ ㅙ ㅚ ㅛ ㅜ ㅝ ㅞ ㅟ ㅠ ㅡ ㅢ ㅣ
  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 3, 1, 2, 4, 4, 4, 2, 1, 3, 0
};

const uint8_t kind_initial_by_medial_with_final[NUM_MEDIAL] = {
  // ㅏ ㅐ ㅑ ㅒ ㅓ ㅔ ㅕ ㅖ ㅗ ㅘ ㅙ ㅚ ㅛ ㅜ ㅝ ㅞ ㅟ ㅠ ㅡ ㅢ ㅣ
  0, 5, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 6, 6, 7, 7, 7, 6, 6, 7, 5
};

const uint8_t kind_medial_by_initial_without_final[NUM_INITIAL] = {
  // ㄱ ㄲ ㄴ ㄷ ㄸ ㄹ ㅁ ㅂ ㅃ ㅅ ㅆ ㅇ ㅈ ㅉ ㅊ ㅋ ㅌ ㅍ ㅎ
  0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1
};

const uint8_t kind_medial_by_initial_with_final[NUM_INITIAL] = {
  // ㄱ ㄲ ㄴ ㄷ ㄸ ㄹ ㅁ ㅂ ㅃ ㅅ ㅆ ㅇ ㅈ ㅉ ㅊ ㅋ ㅌ ㅍ ㅎ
  0, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3
};

const uint8_t kind_final_by_medial[NUM_MEDIAL] = {
  // ㅏ ㅐ ㅑ ㅒ ㅓ ㅔ ㅕ ㅖ ㅗ ㅘ ㅙ ㅚ ㅛ ㅜ ㅝ ㅞ ㅟ ㅠ ㅡ ㅢ ㅣ
  0, 0, 2, 0, 2, 1, 2, 1, 2, 3, 0, 2, 1, 3, 3, 1, 2, 1, 3, 3, 1, 1
};

void get_han_glyph_kinds(int initial, int medial, int final,
    int* initial_kind, int* medial_kind, int* final_kind)
{
  if (final == 0) {
      *final_kind = 0;
      *medial_kind = kind_medial_by_initial_without_final[initial];
      *initial_kind = kind_initial_by_medial_without_final[medial];
  } else {
      *final_kind = kind_final_by_medial[medial];
      *medial_kind = kind_medial_by_initial_with_final[initial];
      *initial_kind = kind_initial_by_medial_with_final[medial];
  }
}
#endif

#ifdef HAN_FONT_621
//
// 6x2x1 hangul syllable composition rules:
// - initial consonants: 6 kinds = 3 kinds by meidial x 2 kinds by final existence = 6 kinds
// - medial vowels: 2 kinds = 2 kinds by final existence
// - final consonants: 1 kind
//

const uint8_t kind_initial_by_medial[NUM_MEDIAL] = {
    // ㅏ ㅐ ㅑ ㅒ ㅓ ㅔ ㅕ ㅖ ㅗ ㅘ ㅙ ㅚ ㅛ ㅜ ㅝ ㅞ ㅟ ㅠ ㅡ ㅢ ㅣ
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 1, 1, 2, 2, 2, 1, 1, 2, 0
};

void get_han_glyph_kinds(int initial, int medial, int final,
    int* initial_kind, int* medial_kind, int* final_kind)
{
  (void)initial; // unused
  *initial_kind = kind_initial_by_medial[medial] + (final ? 3 : 0);
  *medial_kind = final ? 1 : 0;
  *final_kind = 0;
}
#endif

//
// hangul glyph cache for better performance
//

struct han_glyph_cache_entry {
  uint16_t hit;
  uint16_t code;
  uint8_t glyph[HAN_GLYPH_SIZE];
  //uint16_t glyph[HAN_GLYPH_SIZE/2];
  //uint32_t glyph[HAN_GLYPH_SIZE/4];
};

// (32+4) * 80 = 2880
#define HAN_GLYPH_CACHE_SIZE 80

struct han_glyph_cache_entry han_glyph_cache[HAN_GLYPH_CACHE_SIZE];

bool init_han_glyph_cache_done = false;

inline void init_han_glyph_cache() {
  if (init_han_glyph_cache_done) return;
  init_han_glyph_cache_done = true;
  memset(han_glyph_cache, 0, sizeof(han_glyph_cache));
}

inline uint8_t* lookup_han_glyph_cache(uint16_t code) {
  for (int i = 0; i < HAN_GLYPH_CACHE_SIZE; i++) {
    if (han_glyph_cache[i].code == code) {
      han_glyph_cache[i].hit++;
      return han_glyph_cache[i].glyph;
    }
  }
  return NULL;
}

inline uint8_t* alloc_han_glyph_cache(uint16_t code) {
  int min_entry = 0;
  uint16_t min_hit = 0xffff;
  // find least recently used entry
  for (int i = 0; i < HAN_GLYPH_CACHE_SIZE; i++) {
    if (han_glyph_cache[i].code == 0) {
      min_entry = i;
      break;
    }
    if (han_glyph_cache[i].hit < min_hit) {
      min_entry = i;
      min_hit = han_glyph_cache[i].hit;
    }
  }
  han_glyph_cache[min_entry].hit = 0;
  han_glyph_cache[min_entry].code = code;
  return han_glyph_cache[min_entry].glyph;
}

uint8_t* get_han_glyph(uint16_t code)
{
  // check cache
  uint8_t* glyph = lookup_han_glyph_cache(code);
  if (glyph) return glyph;

  glyph = alloc_han_glyph_cache(code);

  int initial, medial, final;
  decompose_hangul_syllable(code, &initial, &medial, &final);

  int initial_kind, medial_kind, final_kind;
  get_han_glyph_kinds(initial, medial, final, &initial_kind, &medial_kind, &final_kind);

  int initial_offset = INITIAL_GLYPH_BASE + (initial_kind * NUM_INITIAL + initial) * HAN_GLYPH_SIZE;
  int medial_offset = MEDIAL_GLYPH_BASE + (medial_kind * NUM_MEDIAL + medial) * HAN_GLYPH_SIZE;
  int final_offset = FINAL_GLYPH_BASE + (final_kind * NUM_FINAL + final) * HAN_GLYPH_SIZE;

  uint8_t* initial_glyph = (uint8_t*)(han_font + initial_offset);
  uint8_t* medial_glyph = (uint8_t*)(han_font + medial_offset);
  uint8_t* final_glyph = (uint8_t*)(han_font + final_offset);

  // row-major
  for (int i = 0; i < HAN_GLYPH_SIZE; i++) {
      glyph[i] = *initial_glyph++ | *medial_glyph++ | *final_glyph++;
  }

  // column-major
  // transpose for better rendering performance
  // 0 1    -> 0 8
  // 2 3    -> 1 9
  // ...    -> ...
  // 14 15  -> 7 15
  //for (int i = 0; i < HAN_GLYPH_SIZE/2; i++) {
  //    glyph[i] = *initial_glyph++ | *medial_glyph++ | *final_glyph++;
  //    glyph[i+16] = *initial_glyph++ | *medial_glyph++ | *final_glyph++;
  //}

  return glyph;
}
