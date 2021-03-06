/*****************************************************************************
 * HTemplate.h
 ***************************************************************************** 
 *
 * Project:     Halcon/C++
 * Description: Class HTemplate
 *
 * (c) 1996-2005 by MVTec Software GmbH
 *                  www.mvtec.com
 * 
 *****************************************************************************
 *
 * $Revision: 1.6 $
 * $Date: 2005/05/06 07:36:06 $
 *
 */

#ifndef H_TEMPLATE_H
#define H_TEMPLATE_H


namespace Halcon {


class LIntExport HTemplate: public HRootObject {
public:
  HTemplate(void) {id = -1;}
  HTemplate(
      const HImage &Template, 
      const HTuple &FirstError, 
      const HTuple &NumLevel, 
      const HTuple &Optimize, 
      const HTuple &GrayValues);
  HTemplate(
      const HImage &Template, 
      Hlong          FirstError, 
      Hlong          NumLevel, 
      const char   *Optimize = "sort", 
      const char   *GrayValues = "original");
  HTemplate(
      const HImage &Template, 
      const HTuple &NumLevel, 
      const HTuple &AngleStart, 
      const HTuple &AngleExtend, 
      const HTuple &AngleStep, 
      const HTuple &Optimize, 
      const HTuple &GrayValues);
  HTemplate(
      const HImage &Template, 
      Hlong          NumLevel, 
      double        AngleStart, 
      double        AngleExtend, 
      double        AngleStep, 
      const char   *Optimize = "sort", 
      const char   *GrayValues = "original");

  ~HTemplate(void);
private:
  // Copying and Assignment not allowed
  HTemplate(const HTemplate &inst);
  HTemplate &operator= (const HTemplate &inst);
  // Internal Cleanup
  void Cleanup(void);

public: 
  // Tools
  const char *ClassName(void) const {return "HTemplate";}

  // Cast Operator
  operator HTuple () const {return HTuple(GetHandle());}

  // Access and Manipulation
  Hlong GetHandle(void) const {return id;}
  void SetHandle(Hlong ID) {id = ID;}

private:
  //Handle
  Hlong  id;
public:
  // Preparing a pattern for template matching with rotation.
  virtual void CreateTemplateRot(const HImage &Template, const Halcon::HTuple &NumLevel, const Halcon::HTuple &AngleStart, const Halcon::HTuple &AngleExtend, const Halcon::HTuple &AngleStep, const Halcon::HTuple &Optimize, const Halcon::HTuple &GrayValues);
  virtual void CreateTemplateRot(const HImage &Template, Hlong NumLevel, double AngleStart, double AngleExtend, double AngleStep, const char *Optimize, const char *GrayValues);
  // Preparing a pattern for template matching.
  virtual void CreateTemplate(const HImage &Template, const Halcon::HTuple &FirstError, const Halcon::HTuple &NumLevel, const Halcon::HTuple &Optimize, const Halcon::HTuple &GrayValues);
  virtual void CreateTemplate(const HImage &Template, Hlong FirstError, Hlong NumLevel, const char *Optimize, const char *GrayValues);
  // Writing a template to file.
  virtual void WriteTemplate(const Halcon::HTuple &FileName) const;
  virtual void WriteTemplate(const char *FileName) const;
  // Reading a template from file.
  virtual void ReadTemplate(const Halcon::HTuple &FileName);
  virtual void ReadTemplate(const char *FileName);
  // Gray value offset for template.
  virtual void SetOffsetTemplate(const Halcon::HTuple &GrayOffset) const;
  virtual void SetOffsetTemplate(Hlong GrayOffset) const;
  // Define reference position for a matching template.
  virtual void SetReferenceTemplate(const Halcon::HTuple &Row, const Halcon::HTuple &Column) const;
  virtual void SetReferenceTemplate(double Row, double Column) const;
  // Adapting a template to the size of an image.
  virtual void AdaptTemplate(const HImageArray &Image) const;
  // Searching all good grayvalue matches in a pyramid.
  virtual HRegionArray FastMatchMg(const HImageArray &Image, const Halcon::HTuple &MaxError, const Halcon::HTuple &NumLevel) const;
  virtual HRegionArray FastMatchMg(const HImageArray &Image, double MaxError, Hlong NumLevel) const;
  // Searching the best grayvalue matches in a pre generated pyramid.
  virtual double BestMatchPreMg(const HImageArray &ImagePyramid, const Halcon::HTuple &MaxError, const Halcon::HTuple &SubPixel, const Halcon::HTuple &NumLevels, const Halcon::HTuple &WhichLevels, Halcon::HTuple *Column, Halcon::HTuple *Error) const;
  virtual double BestMatchPreMg(const HImageArray &ImagePyramid, const Halcon::HTuple &MaxError, const Halcon::HTuple &SubPixel, const Halcon::HTuple &NumLevels, const Halcon::HTuple &WhichLevels, double *Column, double *Error) const;
  virtual double BestMatchPreMg(const HImageArray &ImagePyramid, double MaxError, const char *SubPixel, Hlong NumLevels, Hlong WhichLevels, Halcon::HTuple *Column, Halcon::HTuple *Error) const;
  virtual double BestMatchPreMg(const HImageArray &ImagePyramid, double MaxError, const char *SubPixel, Hlong NumLevels, Hlong WhichLevels, double *Column, double *Error) const;
  // Searching the best grayvalue matches in a pyramid.
  virtual double BestMatchMg(const HImageArray &Image, const Halcon::HTuple &MaxError, const Halcon::HTuple &SubPixel, const Halcon::HTuple &NumLevels, const Halcon::HTuple &WhichLevels, Halcon::HTuple *Column, Halcon::HTuple *Error) const;
  virtual double BestMatchMg(const HImageArray &Image, const Halcon::HTuple &MaxError, const Halcon::HTuple &SubPixel, const Halcon::HTuple &NumLevels, const Halcon::HTuple &WhichLevels, double *Column, double *Error) const;
  virtual double BestMatchMg(const HImageArray &Image, double MaxError, const char *SubPixel, Hlong NumLevels, Hlong WhichLevels, Halcon::HTuple *Column, Halcon::HTuple *Error) const;
  virtual double BestMatchMg(const HImageArray &Image, double MaxError, const char *SubPixel, Hlong NumLevels, Hlong WhichLevels, double *Column, double *Error) const;
  // Searching all good matches of a template and an image.
  virtual HRegionArray FastMatch(const HImageArray &Image, const Halcon::HTuple &MaxError) const;
  virtual HRegionArray FastMatch(const HImageArray &Image, double MaxError) const;
  // Searching the best matching of a template and a pyramid with rotation.
  virtual HTuple BestMatchRotMg(const HImageArray &Image, const Halcon::HTuple &AngleStart, const Halcon::HTuple &AngleExtend, const Halcon::HTuple &MaxError, const Halcon::HTuple &SubPixel, const Halcon::HTuple &NumLevels, Halcon::HTuple *Column, Halcon::HTuple *Angle, Halcon::HTuple *Error) const;
  virtual double BestMatchRotMg(const HImageArray &Image, const Halcon::HTuple &AngleStart, const Halcon::HTuple &AngleExtend, const Halcon::HTuple &MaxError, const Halcon::HTuple &SubPixel, const Halcon::HTuple &NumLevels, double *Column, double *Angle, double *Error) const;
  virtual HTuple BestMatchRotMg(const HImageArray &Image, double AngleStart, double AngleExtend, double MaxError, const char *SubPixel, Hlong NumLevels, Halcon::HTuple *Column, Halcon::HTuple *Angle, Halcon::HTuple *Error) const;
  virtual double BestMatchRotMg(const HImageArray &Image, double AngleStart, double AngleExtend, double MaxError, const char *SubPixel, Hlong NumLevels, double *Column, double *Angle, double *Error) const;
  // Searching the best matching of a template and an image with rotation.
  virtual HTuple BestMatchRot(const HImageArray &Image, const Halcon::HTuple &AngleStart, const Halcon::HTuple &AngleExtend, const Halcon::HTuple &MaxError, const Halcon::HTuple &SubPixel, Halcon::HTuple *Column, Halcon::HTuple *Angle, Halcon::HTuple *Error) const;
  virtual double BestMatchRot(const HImageArray &Image, const Halcon::HTuple &AngleStart, const Halcon::HTuple &AngleExtend, const Halcon::HTuple &MaxError, const Halcon::HTuple &SubPixel, double *Column, double *Angle, double *Error) const;
  virtual HTuple BestMatchRot(const HImageArray &Image, double AngleStart, double AngleExtend, double MaxError, const char *SubPixel, Halcon::HTuple *Column, Halcon::HTuple *Angle, Halcon::HTuple *Error) const;
  virtual double BestMatchRot(const HImageArray &Image, double AngleStart, double AngleExtend, double MaxError, const char *SubPixel, double *Column, double *Angle, double *Error) const;
  // Searching the best matching of a template and an image.
  virtual HTuple BestMatch(const HImageArray &Image, const Halcon::HTuple &MaxError, const Halcon::HTuple &SubPixel, Halcon::HTuple *Column, Halcon::HTuple *Error) const;
  virtual double BestMatch(const HImageArray &Image, const Halcon::HTuple &MaxError, const Halcon::HTuple &SubPixel, double *Column, double *Error) const;
  virtual HTuple BestMatch(const HImageArray &Image, double MaxError, const char *SubPixel, Halcon::HTuple *Column, Halcon::HTuple *Error) const;
  virtual double BestMatch(const HImageArray &Image, double MaxError, const char *SubPixel, double *Column, double *Error) const;
};

}

#endif
