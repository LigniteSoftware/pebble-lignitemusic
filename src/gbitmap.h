#include <lignite_music.h>

/**
 * This is not my library, so I will not comment its contents but instead give
 * credit to the amazing developer who wrote it.
 *
 * Thank you very much for this awesome library, Jon.
 *
 * https://github.com/rebootsramblings/GBitmap-Colour-Palette-Manipulator
 */

#ifdef PBL_COLOR
char* get_gbitmapformat_text(GBitmapFormat format);
const char* get_gcolor_text(GColor m_color);
#endif

#ifndef PBL_PLATFORM_APLITE
void replace_gbitmap_color(GColor color_to_replace, GColor replace_with_color, GBitmap *im, BitmapLayer *bml);
#endif

#ifdef PBL_COLOR
void spit_gbitmap_color_palette(GBitmap *im);
bool gbitmap_color_palette_contains_color(GColor m_color, GBitmap *im);
void gbitmap_fill_all_except(GColor color_to_not_change, GColor fill_color, bool fill_gcolorclear, GBitmap *im, BitmapLayer *bml);

#endif