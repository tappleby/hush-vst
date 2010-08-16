#ifndef _LICE_COMBINE_H_
#define _LICE_COMBINE_H_

#define __LICE_BOUND(x,lo,hi) ((x)<(lo)?(lo):((x)>(hi)?(hi):(x)))

#ifdef _MSC_VER
static inline int __LICE_TOINT(double x) // don't use this _everywhere_ since it doesnt round the same as (int)
{
  int tmp;
  __asm
  {
    fld x
    fistp tmp
  };
  return tmp;
}
#else
#define __LICE_TOINT(x) ((int)(x))
#endif

static inline void __LICE_BilinearFilter(int *r, int *g, int *b, int *a, LICE_pixel_chan *pin, LICE_pixel_chan *pinnext, double xfrac, double yfrac)
{
  double f4=xfrac*yfrac;
  double f1=1.0-yfrac-xfrac+f4; // (1.0-xfrac)*(1.0-yfrac);
  double f3=yfrac-f4; // (1.0-xfrac)*yfrac;
  double f2=xfrac-f4; // xfrac*(1.0-yfrac);
  *r=__LICE_TOINT(pin[LICE_PIXEL_R]*f1 + pin[4+LICE_PIXEL_R]*f2 + pinnext[LICE_PIXEL_R]*f3 + pinnext[4+LICE_PIXEL_R]*f4);
  *g=__LICE_TOINT(pin[LICE_PIXEL_G]*f1 + pin[4+LICE_PIXEL_G]*f2 + pinnext[LICE_PIXEL_G]*f3 + pinnext[4+LICE_PIXEL_G]*f4);
  *b=__LICE_TOINT(pin[LICE_PIXEL_B]*f1 + pin[4+LICE_PIXEL_B]*f2 + pinnext[LICE_PIXEL_B]*f3 + pinnext[4+LICE_PIXEL_B]*f4);
  *a=__LICE_TOINT(pin[LICE_PIXEL_A]*f1 + pin[4+LICE_PIXEL_A]*f2 + pinnext[LICE_PIXEL_A]*f3 + pinnext[4+LICE_PIXEL_A]*f4);
}


static void inline _LICE_MakePixel(LICE_pixel_chan *out, int r, int g, int b, int a)
{
  if (r&~0xff) out[LICE_PIXEL_R]=r<0?0:255; else out[LICE_PIXEL_R] = (LICE_pixel_chan) (r);
  if (g&~0xff) out[LICE_PIXEL_G]=g<0?0:255; else out[LICE_PIXEL_G] = (LICE_pixel_chan) (g);
  if (b&~0xff) out[LICE_PIXEL_B]=b<0?0:255; else out[LICE_PIXEL_B] = (LICE_pixel_chan) (b);
  if (a&~0xff) out[LICE_PIXEL_A]=a<0?0:255; else out[LICE_PIXEL_A] = (LICE_pixel_chan) (a);
}


class _LICE_CombinePixelsCopy 
{
public:
  static inline void doPix(LICE_pixel_chan *dest, int r, int g, int b, int a, int alpha)
  {
    int a2=(256-alpha);

    // we could check alpha=0 here, but the caller should (since alpha is usually used for static alphas)
    _LICE_MakePixel(dest,
      (dest[LICE_PIXEL_R]*a2+r*alpha)/256,
      (dest[LICE_PIXEL_G]*a2+g*alpha)/256,
      (dest[LICE_PIXEL_B]*a2+b*alpha)/256,
      (dest[LICE_PIXEL_A]*a2+a*alpha)/256);
  }
};

class _LICE_CombinePixelsCopySourceAlpha
{
public:
  static inline void doPix(LICE_pixel_chan *dest, int r, int g, int b, int a, int alpha)
  {
    if (a)
    {
      alpha = (alpha*a)/256;

      int a2=(255-alpha);

      _LICE_MakePixel(dest,
        (dest[LICE_PIXEL_R]*a2+r*alpha)/256,
        (dest[LICE_PIXEL_G]*a2+g*alpha)/256,
        (dest[LICE_PIXEL_B]*a2+b*alpha)/256,
        (dest[LICE_PIXEL_A]*a2+a*alpha)/256);  
    }
  }
};
class _LICE_CombinePixelsAdd
{
public:
  static inline void doPix(LICE_pixel_chan *dest, int r, int g, int b, int a, int alpha)
  { 
    // we could check alpha=0 here, but the caller should (since alpha is usually used for static alphas)

    _LICE_MakePixel(dest,
      dest[LICE_PIXEL_R]+(r*alpha)/256,
      dest[LICE_PIXEL_G]+(g*alpha)/256,
      dest[LICE_PIXEL_B]+(b*alpha)/256,
      dest[LICE_PIXEL_A]+(a*alpha)/256);

  }
};
class _LICE_CombinePixelsAddSourceAlpha
{
public:
  static inline void doPix(LICE_pixel_chan *dest, int r, int g, int b, int a, int alpha)
  { 
    if (a)
    {
      alpha=(alpha*a)/256;
      _LICE_MakePixel(dest,
        dest[LICE_PIXEL_R]+(r*alpha)/256,
        dest[LICE_PIXEL_G]+(g*alpha)/256,
        dest[LICE_PIXEL_B]+(b*alpha)/256,
        dest[LICE_PIXEL_A]+(a*alpha)/256);
    }
  }
};

//#define __LICE__ACTION(comb) templateclass<comb>::function(parameters)
//__LICE_ACTIONBYMODE(mode,alpha);
//#undef __LICE__ACTION





#define __LICE_ACTIONBYMODE(mode,alpha) \
     switch ((mode)&LICE_BLIT_MODE_MASK) { \
      case LICE_BLIT_MODE_COPY: \
        if ((alpha)>0.0) {  \
          if ((mode)&LICE_BLIT_USE_ALPHA) __LICE__ACTION(_LICE_CombinePixelsCopySourceAlpha); \
          else __LICE__ACTION(_LICE_CombinePixelsCopy); \
        } \
      break;  \
      case LICE_BLIT_MODE_ADD:  \
        if ((alpha)!=0.0) { \
          if ((mode)&LICE_BLIT_USE_ALPHA) __LICE__ACTION(_LICE_CombinePixelsAddSourceAlpha);  \
          else __LICE__ACTION(_LICE_CombinePixelsAdd);  \
        } \
      break;  \
    }

// used by GradRect, etc
#define __LICE_ACTIONBYMODE_NOALPHA(mode) \
     switch ((mode)&LICE_BLIT_MODE_MASK) { \
      case LICE_BLIT_MODE_COPY: __LICE__ACTION(_LICE_CombinePixelsCopy); break;  \
      case LICE_BLIT_MODE_ADD: __LICE__ACTION(_LICE_CombinePixelsAdd); break;  \
    }




#endif