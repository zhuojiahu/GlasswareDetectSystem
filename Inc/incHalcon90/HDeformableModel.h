/*****************************************************************************
 * HDeformableModel.h
 ***************************************************************************** 
 *
 * Project:     HALCON/C++
 * Description: Class HDeformableModel
 *
 * (c) 2007-2008 by MVTec Software GmbH
 *                  www.mvtec.com
 * 
 *****************************************************************************
 *
 * $Revision: 1.2 $
 * $Date: 2008/12/06 13:35:12 $
 *
 */

#ifndef H_DEFORMABLE_MODEL_H
#define H_DEFORMABLE_MODEL_H


#include "HCPPToolRef.h"


namespace Halcon {


class LIntExport HDeformableModel: public HToolBase 
{
public:

  // Tool-specific functional class constructors
  // Calibrated Case
  // Version with Tuple
  HDeformableModel(
          const HImage  &Template,
          const HTuple  &CamParam,
          const HTuple  &ReferencePose,
          const HTuple  &NumLevels,
          const HTuple  &AngleStart,
          const HTuple  &AngleExtent,
          const HTuple  &AngleStep,
          const HTuple  &ScaleRMin,
          const HTuple  &ScaleRMax,
          const HTuple  &ScaleRStep,
          const HTuple  &ScaleCMin,
          const HTuple  &ScaleCMax,
          const HTuple  &ScaleCStep,
          const HTuple  &Optimization,
          const HTuple  &Metric,
          const HTuple  &Contrast,
          const HTuple  &MinContrast,
          const HTuple  &ParamName,
          const HTuple  &ParamValue);

  // Version with Native data structure
  HDeformableModel(
          const HImage &Template,
          const HTuple &CamParam,
          const HTuple &ReferencePose,
          Hlong         NumLevels,
          double        AngleStart,
          double        AngleExtent,
          double        AngleStep,
          double        ScaleRMin,
          double        ScaleRMax,
          double        ScaleRStep,
          double        ScaleCMin,
          double        ScaleCMax,
          double        ScaleCStep,
          const char   *Optimization,
          const char   *Metric,
          Hlong         Contrast,
          Hlong         MinContrast,
          const HTuple &ParamName,
          const HTuple &ParamValue);


   // Uncalibrated Case
   // Version with Tuple
  HDeformableModel(
          const HImage  &Template,
          const HTuple  &NumLevels,
          const HTuple  &AngleStart,
          const HTuple  &AngleExtent,
          const HTuple  &AngleStep,
          const HTuple  &ScaleRMin,
          const HTuple  &ScaleRMax,
          const HTuple  &ScaleRStep,
          const HTuple  &ScaleCMin,
          const HTuple  &ScaleCMax,
          const HTuple  &ScaleCStep,
          const HTuple  &Optimization,
          const HTuple  &Metric,
          const HTuple  &Contrast,
          const HTuple  &MinContrast,
          const HTuple  &ParamName,
          const HTuple  &ParamValue);

  // Version with Native data structure
  HDeformableModel(
          const HImage &Template,
          const Hlong   NumLevels,
          const double  AngleStart,
          const double  AngleExtent,
          const double  AngleStep,
          const double  ScaleRMin,
          const double  ScaleRMax,
          const double  ScaleRStep,
          const double  ScaleCMin,
          const double  ScaleCMax,
          const double  ScaleCStep,
          const char   *Optimization,
          const char   *Metric,
          const Hlong   Contrast,
          const Hlong   MinContrast,
          const HTuple &ParamName,
          const HTuple &ParamValue);

  // Common tool class management functionality

  HDeformableModel();
  HDeformableModel(Hlong ID);
  const char *ClassName(void) const;
  Hlong GetHandle(void) const;
  void  SetHandle(Hlong ID);

  // Tool-specific member functions

  // Return the origin (reference point) of a deformable model.
  virtual double GetDeformableModelOrigin(Halcon::HTuple *Column) const;
  // Return the origin (reference point) of a deformable model.
  virtual double GetDeformableModelOrigin(double *Column) const;
  // Set the origin (reference point) of a deformable model.
  virtual void SetDeformableModelOrigin(const Halcon::HTuple &Row, const Halcon::HTuple &Column) const;
  // Set the origin (reference point) of a deformable model.
  virtual void SetDeformableModelOrigin(double Row, double Column) const;
  // Set selected parameters of the deformable model.
  virtual void SetDeformableModelParam(const Halcon::HTuple &GenParamNames, const Halcon::HTuple &GenParamValues) const;
  // Return the parameters of a deformable model.
  virtual HTuple GetDeformableModelParams(const Halcon::HTuple &GenParamNames) const;
  // Return the parameters of a deformable model.
  virtual HTuple GetDeformableModelParams(const char *GenParamNames) const;
  // Return the contour representation of a deformable model.
  virtual HXLDContArray GetDeformableModelContours(const Halcon::HTuple &Level) const;
  // Return the contour representation of a deformable model.
  virtual HXLDContArray GetDeformableModelContours(Hlong Level) const;
  // Read a deformable model from a file.
  virtual void ReadDeformableModel(const Halcon::HTuple &FileName);
  // Read a deformable model from a file.
  virtual void ReadDeformableModel(const char *FileName);
  // Write a deformable model to a file.
  virtual void WriteDeformableModel(const Halcon::HTuple &FileName) const;
  // Write a deformable model to a file.
  virtual void WriteDeformableModel(const char *FileName) const;
  // Find the best matches of a calibrated deformable model in an image and return
  // their 3D pose.
  virtual HTuple FindPlanarCalibDeformableModel(const HImage &Image, const Halcon::HTuple &AngleStart, const Halcon::HTuple &AngleExtent, const Halcon::HTuple &ScaleRMin, const Halcon::HTuple &ScaleRMax, const Halcon::HTuple &ScaleCMin, const Halcon::HTuple &ScaleCMax, const Halcon::HTuple &MinScore, const Halcon::HTuple &NumMatches, const Halcon::HTuple &MaxOverlap, const Halcon::HTuple &NumLevels, const Halcon::HTuple &Greediness, const Halcon::HTuple &ParamName, const Halcon::HTuple &ParamValue, Halcon::HTuple *CovPose, Halcon::HTuple *Score) const;
  // Find the best matches of a planar projective invariant deformable model
  // in an image.
  virtual HTuple FindPlanarUncalibDeformableModel(const HImage &Image, const Halcon::HTuple &AngleStart, const Halcon::HTuple &AngleExtent, const Halcon::HTuple &ScaleRMin, const Halcon::HTuple &ScaleRMax, const Halcon::HTuple &ScaleCMin, const Halcon::HTuple &ScaleCMax, const Halcon::HTuple &MinScore, const Halcon::HTuple &NumMatches, const Halcon::HTuple &MaxOverlap, const Halcon::HTuple &NumLevels, const Halcon::HTuple &Greediness, const Halcon::HTuple &ParamName, const Halcon::HTuple &ParamValue, Halcon::HTuple *Score) const;
  // Create a deformable model for calibrated perspective matching.
  virtual void CreatePlanarCalibDeformableModel(const HImage &Template, const Halcon::HTuple &CamParam, const Halcon::HTuple &ReferencePose, const Halcon::HTuple &NumLevels, const Halcon::HTuple &AngleStart, const Halcon::HTuple &AngleExtent, const Halcon::HTuple &AngleStep, const Halcon::HTuple &ScaleRMin, const Halcon::HTuple &ScaleRMax, const Halcon::HTuple &ScaleRStep, const Halcon::HTuple &ScaleCMin, const Halcon::HTuple &ScaleCMax, const Halcon::HTuple &ScaleCStep, const Halcon::HTuple &Optimization, const Halcon::HTuple &Metric, const Halcon::HTuple &Contrast, const Halcon::HTuple &MinContrast, const Halcon::HTuple &ParamName, const Halcon::HTuple &ParamValue);
  // Creates a deformable model for uncalibrated, perspective matching.
  virtual void CreatePlanarUncalibDeformableModel(const HImage &Template, const Halcon::HTuple &NumLevels, const Halcon::HTuple &AngleStart, const Halcon::HTuple &AngleExtent, const Halcon::HTuple &AngleStep, const Halcon::HTuple &ScaleRMin, const Halcon::HTuple &ScaleRMax, const Halcon::HTuple &ScaleRStep, const Halcon::HTuple &ScaleCMin, const Halcon::HTuple &ScaleCMax, const Halcon::HTuple &ScaleCStep, const Halcon::HTuple &Optimization, const Halcon::HTuple &Metric, const Halcon::HTuple &Contrast, const Halcon::HTuple &MinContrast, const Halcon::HTuple &ParamName, const Halcon::HTuple &ParamValue);
};

}

#endif
