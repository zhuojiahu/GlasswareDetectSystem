/*****************************************************************************
 * HInt1Image.h
 *****************************************************************************
 * Project:     Halcon/libhalcon
 * Description: Signed Byte Images
 *
 * (c) 1996-2005 by MVTec Software GmbH
 *                  www.mvtec.com
 * 
 *****************************************************************************
 *
 * $Revision: 1.10 $
 * $Date: 2005/05/06 07:36:05 $
 *
 */


#ifndef H_INT1_IMAGE_H
#define H_INT1_IMAGE_H


namespace Halcon {


class LIntExport HInt1Image: public HImage {
public:
  HInt1Image(void);
  HInt1Image(const char *file);
  HInt1Image(const HImage &image);
  HInt1Image(const HInt1Image &image);
  HInt1Image(int width, int height);
  HInt1Image(int *ptr, int width, int height);
  virtual ~HInt1Image(void);
  
  // Tools
  const char *ClassName(void) const { return "HInt1Image"; }

  // Pixelzugriff [k]
  INT1 &operator[] (Hlong k);
    //{ if (!privat) Private(); return ((INT1*)ptr)[k]; }
  INT1  operator[] (Hlong k) const;
    //{ if (!init) ((HInt1Image*)this)->Init(); return ((INT1*)ptr)[k]; }
  
  // Pixelzugriff (k)
  INT1 &operator() (Hlong k)
  { if (!privat) Private(); return ((INT1*)ptr)[k]; }
  INT1  operator() (Hlong k) const
  { if (!init) ((HInt1Image*)this)->Init(); return ((INT1*)ptr)[k]; }
  
  // Pixelzugriff (x,y)
  INT1 &operator()(int x, int y) 
  { if (!privat) Private();
    return ((INT1*)ptr)[HLinearCoord(x,y,width)]; }
  INT1  operator()(int x, int y) const
  { if (!init) ((HInt1Image*)this)->Init(); 
    return ((INT1*)ptr)[HLinearCoord(x,y,width)]; }

  // Klassenoperationen
  static Hlong NumInst(void) { return num_images; }
protected:
private:
  static Hlong num_images;
};


}


#endif
