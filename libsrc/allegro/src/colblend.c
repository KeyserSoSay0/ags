/*         ______   ___    ___ 
 *        /\  _  \ /\_ \  /\_ \ 
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      Interpolation routines for hicolor and truecolor pixels.
 *
 *      By Cloud Wu and Burton Radons.
 *
 *      Alpha blending optimised by Peter Cech.
 *
 *      See readme.txt for copyright information.
 */


#include "allegro.h"
#include "allegro/internal/aintern.h"



#define BLEND(bpp, r, g, b)   _blender_trans##bpp(makecol##bpp(r, g, b), y, n)

#define T(x, y, n)            (((y) - (x)) * (n) / 255 + (x))



/* _blender_black:
 *  Fallback routine for when we don't have anything better to do.
 */
uint32_t _blender_black(uint32_t x, uint32_t y, uint32_t n)
{
   return 0;
}



#if (defined ALLEGRO_COLOR24) || (defined ALLEGRO_COLOR32)



#if (defined ALLEGRO_NO_ASM) || (!defined ALLEGRO_I386) 
				    /* i386 asm version is in imisc.s */


/* _blender_trans24:
 *  24 bit trans blender function.
 */
uint32_t _blender_trans24(uint32_t x, uint32_t y, uint32_t n)
{
   uint32_t res, g;

   if (n)
      n++;

   res = ((x & 0xFF00FF) - (y & 0xFF00FF)) * n / 256 + y;
   y &= 0xFF00;
   x &= 0xFF00;
   g = (x - y) * n / 256 + y;

   res &= 0xFF00FF;
   g &= 0xFF00;

   return res | g;
}


#endif      /* C version */



/* _blender_alpha24:
 *  Combines a 32 bit RGBA sprite with a 24 bit RGB destination.
 */
uint32_t _blender_alpha24(uint32_t x, uint32_t y, uint32_t n)
{
   uint32_t xx = makecol24(getr32(x), getg32(x), getb32(x));
   uint32_t res, g;

   n = geta32(x);

   if (n)
      n++;

   res = ((xx & 0xFF00FF) - (y & 0xFF00FF)) * n / 256 + y;
   y &= 0xFF00;
   xx &= 0xFF00;
   g = (xx - y) * n / 256 + y;

   res &= 0xFF00FF;
   g &= 0xFF00;

   return res | g;
}



/* _blender_alpha32:
 *  Combines a 32 bit RGBA sprite with a 32 bit RGB destination.
 */
uint32_t _blender_alpha32(uint32_t x, uint32_t y, uint32_t n)
{
   uint32_t res, g;

   n = geta32(x);

   if (n)
      n++;

   res = ((x & 0xFF00FF) - (y & 0xFF00FF)) * n / 256 + y;
   y &= 0xFF00;
   x &= 0xFF00;
   g = (x - y) * n / 256 + y;

   res &= 0xFF00FF;
   g &= 0xFF00;

   return res | g;
}



/* _blender_alpha24_bgr:
 *  Combines a 32 bit RGBA sprite with a 24 bit RGB destination, optimised
 *  for when one is in a BGR format and the other is RGB.
 */
uint32_t _blender_alpha24_bgr(uint32_t x, uint32_t y, uint32_t n)
{
   uint32_t res, g;

   n = x >> 24;

   if (n)
      n++;

   x = ((x>>16)&0xFF) | (x&0xFF00) | ((x<<16)&0xFF0000);

   res = ((x & 0xFF00FF) - (y & 0xFF00FF)) * n / 256 + y;
   y &= 0xFF00;
   x &= 0xFF00;
   g = (x - y) * n / 256 + y;

   res &= 0xFF00FF;
   g &= 0xFF00;

   return res | g;
}



/* _blender_add24:
 *  24 bit additive blender function.
 */
uint32_t _blender_add24(uint32_t x, uint32_t y, uint32_t n)
{
   int r = getr24(y) + getr24(x) * n / 256;
   int g = getg24(y) + getg24(x) * n / 256;
   int b = getb24(y) + getb24(x) * n / 256;

   r = MIN(r, 255);
   g = MIN(g, 255);
   b = MIN(b, 255);

   return makecol24(r, g, b);
}



/* _blender_burn24:
 *  24 bit burn blender function.
 */
uint32_t _blender_burn24(uint32_t x, uint32_t y, uint32_t n)
{
   return BLEND(24, MAX(getr24(x) - getr24(y), 0),
		    MAX(getg24(x) - getg24(y), 0),
		    MAX(getb24(x) - getb24(y), 0));
}



/* _blender_color24:
 *  24 bit color blender function.
 */
uint32_t _blender_color24(uint32_t x, uint32_t y, uint32_t n)
{
   float xh, xs, xv;
   float yh, ys, yv;
   int r, g, b;

   rgb_to_hsv(getr24(x), getg24(x), getb24(x), &xh, &xs, &xv);
   rgb_to_hsv(getr24(y), getg24(y), getb24(y), &yh, &ys, &yv);

   xs = T(xs, ys, n);
   xh = T(xh, yh, n);

   hsv_to_rgb(xh, xs, xv, &r, &g, &b);

   return makecol24(r, g, b);
}



/* _blender_difference24:
 *  24 bit difference blender function.
 */
uint32_t _blender_difference24(uint32_t x, uint32_t y, uint32_t n)
{
   return BLEND(24, ABS(getr24(y) - getr24(x)),
		    ABS(getg24(y) - getg24(x)),
		    ABS(getb24(y) - getb24(x)));
}



/* _blender_dissolve24:
 *  24 bit dissolve blender function.
 */
uint32_t _blender_dissolve24(uint32_t x, uint32_t y, uint32_t n)
{
   if (n == 255)
      return x;

   return ((_al_rand() & 255) < (int)n) ? x : y;
}



/* _blender_dodge24:
 *  24 bit dodge blender function.
 */
uint32_t _blender_dodge24(uint32_t x, uint32_t y, uint32_t n)
{
   return BLEND(24, getr24(x) + (getr24(y) * n / 256),
		    getg24(x) + (getg24(y) * n / 256),
		    getb24(x) + (getb24(y) * n / 256));
}



/* _blender_hue24:
 *  24 bit hue blender function.
 */
uint32_t _blender_hue24(uint32_t x, uint32_t y, uint32_t n)
{
   float xh, xs, xv;
   float yh, ys, yv;
   int r, g, b;

   rgb_to_hsv(getr24(x), getg24(x), getb24(x), &xh, &xs, &xv);
   rgb_to_hsv(getr24(y), getg24(y), getb24(y), &yh, &ys, &yv);

   xh = T(xh, yh, n);

   hsv_to_rgb(xh, xs, xv, &r, &g, &b);

   return makecol24(r, g, b);
}



/* _blender_invert24:
 *  24 bit invert blender function.
 */
uint32_t _blender_invert24(uint32_t x, uint32_t y, uint32_t n)
{
   return BLEND(24, 255-getr24(x), 255-getg24(x), 255-getb24(x));
}



/* _blender_luminance24:
 *  24 bit luminance blender function.
 */
uint32_t _blender_luminance24(uint32_t x, uint32_t y, uint32_t n)
{
   float xh, xs, xv;
   float yh, ys, yv;
   int r, g, b;

   rgb_to_hsv(getr24(x), getg24(x), getb24(x), &xh, &xs, &xv);
   rgb_to_hsv(getr24(y), getg24(y), getb24(y), &yh, &ys, &yv);

   xv = T(xv, yv, n);

   hsv_to_rgb(xh, xs, xv, &r, &g, &b);

   return makecol24(r, g, b);
}



/* _blender_multiply24:
 *  24 bit multiply blender function.
 */
uint32_t _blender_multiply24(uint32_t x, uint32_t y, uint32_t n)
{
   return BLEND(24, getr24(x) * getr24(y) / 256, 
		    getg24(x) * getg24(y) / 256, 
		    getb24(x) * getb24(y) / 256);
}



/* _blender_saturation24:
 *  24 bit saturation blender function.
 */
uint32_t _blender_saturation24(uint32_t x, uint32_t y, uint32_t n)
{
   float xh, xs, xv;
   float yh, ys, yv;
   int r, g, b;

   rgb_to_hsv(getr24(x), getg24(x), getb24(x), &xh, &xs, &xv);
   rgb_to_hsv(getr24(y), getg24(y), getb24(y), &yh, &ys, &yv);

   xs = T(xs, ys, n);

   hsv_to_rgb(xh, xs, xv, &r, &g, &b);

   return makecol24(r, g, b);
}



/* _blender_screen24:
 *  24 bit screen blender function.
 */
uint32_t _blender_screen24(uint32_t x, uint32_t y, uint32_t n)
{
   return BLEND(24, 255 - ((255 - getr24(x)) * (255 - getr24(y))) / 256,
		    255 - ((255 - getg24(x)) * (255 - getg24(y))) / 256,
		    255 - ((255 - getb24(x)) * (255 - getb24(y))) / 256);
}



#endif      /* end of 24/32 bit routines */


#if (defined ALLEGRO_COLOR15) || (defined ALLEGRO_COLOR16)



/* _blender_trans16:
 *  16 bit trans blender function.
 */
uint32_t _blender_trans16(uint32_t x, uint32_t y, uint32_t n)
{
   uint32_t result;

   if (n)
      n = (n + 1) / 8;

   x = ((x & 0xFFFF) | (x << 16)) & 0x7E0F81F;
   y = ((y & 0xFFFF) | (y << 16)) & 0x7E0F81F;

   result = ((x - y) * n / 32 + y) & 0x7E0F81F;

   return ((result & 0xFFFF) | (result >> 16));
}



/* _blender_alpha16:
 *  Combines a 32 bit RGBA sprite with a 16 bit RGB destination.
 */
uint32_t _blender_alpha16(uint32_t x, uint32_t y, uint32_t n)
{
   uint32_t result;

   n = geta32(x);

   if (n)
      n = (n + 1) / 8;

   x = makecol16(getr32(x), getg32(x), getb32(x));

   x = (x | (x << 16)) & 0x7E0F81F;
   y = ((y & 0xFFFF) | (y << 16)) & 0x7E0F81F;

   result = ((x - y) * n / 32 + y) & 0x7E0F81F;

   return ((result & 0xFFFF) | (result >> 16));
}



/* _blender_alpha16_rgb
 *  Combines a 32 bit RGBA sprite with a 16 bit RGB destination, optimised
 *  for when both pixels are in an RGB layout.
 */
uint32_t _blender_alpha16_rgb(uint32_t x, uint32_t y, uint32_t n)
{
   uint32_t result;

   n = x >> 24;

   if (n)
      n = (n + 1) / 8;

   x = ((x>>3)&0x001F) | ((x>>5)&0x07E0) | ((x>>8)&0xF800);

   x = (x | (x << 16)) & 0x7E0F81F;
   y = ((y & 0xFFFF) | (y << 16)) & 0x7E0F81F;

   result = ((x - y) * n / 32 + y) & 0x7E0F81F;

   return ((result & 0xFFFF) | (result >> 16));
}



/* _blender_alpha16_bgr
 *  Combines a 32 bit RGBA sprite with a 16 bit RGB destination, optimised
 *  for when one pixel is in an RGB layout and the other is BGR.
 */
uint32_t _blender_alpha16_bgr(uint32_t x, uint32_t y, uint32_t n)
{
   uint32_t result;

   n = x >> 24;

   if (n)
      n = (n + 1) / 8;

   x = ((x>>19)&0x001F) | ((x>>5)&0x07E0) | ((x<<8)&0xF800);

   x = (x | (x << 16)) & 0x7E0F81F;
   y = ((y & 0xFFFF) | (y << 16)) & 0x7E0F81F;

   result = ((x - y) * n / 32 + y) & 0x7E0F81F;

   return ((result & 0xFFFF) | (result >> 16));
}


/* _blender_trans15:
 *  15 bit trans blender function.
 */
uint32_t _blender_trans15(uint32_t x, uint32_t y, uint32_t n)
{
   uint32_t result;

   if (n)
      n = (n + 1) / 8;

   x = ((x & 0xFFFF) | (x << 16)) & 0x3E07C1F;
   y = ((y & 0xFFFF) | (y << 16)) & 0x3E07C1F;

   result = ((x - y) * n / 32 + y) & 0x3E07C1F;

   return ((result & 0xFFFF) | (result >> 16));
}



/* _blender_alpha15:
 *  Combines a 32 bit RGBA sprite with a 15 bit RGB destination.
 */
uint32_t _blender_alpha15(uint32_t x, uint32_t y, uint32_t n)
{
   uint32_t result;

   n = geta32(x);

   if (n)
      n = (n + 1) / 8;

   x = makecol15(getr32(x), getg32(x), getb32(x));

   x = (x | (x << 16)) & 0x3E07C1F;
   y = ((y & 0xFFFF) | (y << 16)) & 0x3E07C1F;

   result = ((x - y) * n / 32 + y) & 0x3E07C1F;

   return ((result & 0xFFFF) | (result >> 16));
}



/* _blender_alpha15_rgb
 *  Combines a 32 bit RGBA sprite with a 15 bit RGB destination, optimised
 *  for when both pixels are in an RGB layout.
 */
uint32_t _blender_alpha15_rgb(uint32_t x, uint32_t y, uint32_t n)
{
   uint32_t result;

   n = x >> 24;

   if (n)
      n = (n + 1) / 8;

   x = ((x>>3)&0x001F) | ((x>>6)&0x03E0) | ((x>>9)&0xEC00);

   x = (x | (x << 16)) & 0x3E07C1F;
   y = ((y & 0xFFFF) | (y << 16)) & 0x3E07C1F;

   result = ((x - y) * n / 32 + y) & 0x3E07C1F;

   return ((result & 0xFFFF) | (result >> 16));
}



/* _blender_alpha15_bgr
 *  Combines a 32 bit RGBA sprite with a 15 bit RGB destination, optimised
 *  for when one pixel is in an RGB layout and the other is BGR.
 */
uint32_t _blender_alpha15_bgr(uint32_t x, uint32_t y, uint32_t n)
{
   uint32_t result;

   n = x >> 24;

   if (n)
      n = (n + 1) / 8;

   x = ((x>>19)&0x001F) | ((x>>6)&0x03E0) | ((x<<7)&0xEC00);

   x = (x | (x << 16)) & 0x3E07C1F;
   y = ((y & 0xFFFF) | (y << 16)) & 0x3E07C1F;

   result = ((x - y) * n / 32 + y) & 0x3E07C1F;

   return ((result & 0xFFFF) | (result >> 16));
}

#endif      /* end of 15/16 bit routines */



#ifdef ALLEGRO_COLOR16
   #define BF16(name)   name
#else
   #define BF16(name)   _blender_black
#endif


#if (defined ALLEGRO_COLOR24) || (defined ALLEGRO_COLOR32)
   #define BF24(name)   name
#else
   #define BF24(name)   _blender_black
#endif



/* these functions are all the same, so we can generate them with a macro */
#define SET_BLENDER_FUNC(name)                                 \
   void set_##name##_blender(int r, int g, int b, int a)       \
   {                                                           \
      set_blender_mode(BF16(_blender_##name##15),              \
		       BF16(_blender_##name##16),              \
		       BF24(_blender_##name##24),              \
		       r, g, b, a);                            \
   }


SET_BLENDER_FUNC(trans);



/* set_alpha_blender:
 *  Sets the special RGBA blending mode.
 */
void set_alpha_blender(void)
{
   BLENDER_FUNC f15, f16, f24, f32;
   int r, b;

   /* check which way around the 32 bit pixels are */
   if ((_rgb_g_shift_32 == 8) && (_rgb_a_shift_32 == 24)) {
      r = (_rgb_r_shift_32) ? 1 : 0;
      b = (_rgb_b_shift_32) ? 1 : 0;
   }
   else
      r = b = 0;

   #ifdef ALLEGRO_COLOR16

      /* decide which 15 bit blender to use */
      if ((_rgb_r_shift_15 == r*10) && (_rgb_g_shift_15 == 5) && (_rgb_b_shift_15 == b*10))
	 f15 = _blender_alpha15_rgb;
      else if ((_rgb_r_shift_15 == b*10) && (_rgb_g_shift_15 == 5) && (_rgb_b_shift_15 == r*10))
	 f15 = _blender_alpha15_bgr;
      else
	 f15 = _blender_alpha15;

      /* decide which 16 bit blender to use */
      if ((_rgb_r_shift_16 == r*11) && (_rgb_g_shift_16 == 5) && (_rgb_b_shift_16 == b*11))
	 f16 = _blender_alpha16_rgb;
      else if ((_rgb_r_shift_16 == b*11) && (_rgb_g_shift_16 == 5) && (_rgb_b_shift_16 == r*11))
	 f16 = _blender_alpha16_bgr;
      else
	 f16 = _blender_alpha16;

   #else

      /* hicolor not included in this build */
      f15 = _blender_black;
      f16 = _blender_black;

   #endif

   #ifdef ALLEGRO_COLOR24

      /* decide which 24 bit blender to use */
      if ((_rgb_r_shift_24 == r*16) && (_rgb_g_shift_24 == 8) && (_rgb_b_shift_24 == b*16))
	 f24 = _blender_alpha32;
      else if ((_rgb_r_shift_24 == b*16) && (_rgb_g_shift_24 == 8) && (_rgb_b_shift_24 == r*16))
	 f24 = _blender_alpha24_bgr;
      else
	 f24 = _blender_alpha24;

   #else

      /* 24 bit color not included in this build */
      f24 = _blender_black;

   #endif

   #ifdef ALLEGRO_COLOR32
      f32 = _blender_alpha32;
   #else
      f32 = _blender_black;
   #endif

   set_blender_mode_ex(_blender_black, _blender_black, _blender_black,
		       f32, f15, f16, f24, 0, 0, 0, 0);
}
