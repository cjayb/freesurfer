/**
 * @file  kvlCroppedImageReader.h
 * @brief REPLACE_WITH_ONE_LINE_SHORT_DESCRIPTION
 *
 * REPLACE_WITH_LONG_DESCRIPTION_OR_REFERENCE
 */
/*
 * Original Author: Koen Van Leemput
 * CVS Revision Info:
 *    $Author: nicks $
 *    $Date: 2011/09/28 21:04:05 $
 *    $Revision: 1.1.2.4 $
 *
 * Copyright © 2011 The General Hospital Corporation (Boston, MA) "MGH"
 *
 * Terms and conditions for use, reproduction, distribution and contribution
 * are found in the 'FreeSurfer Software License Agreement' contained
 * in the file 'LICENSE' found in the FreeSurfer distribution, and here:
 *
 * https://surfer.nmr.mgh.harvard.edu/fswiki/FreeSurferSoftwareLicense
 *
 * Reporting: freesurfer@nmr.mgh.harvard.edu
 *
 */
#ifndef __kvlCroppedImageReader_h
#define __kvlCroppedImageReader_h

#include "itkImage.h"
#include "itkAffineTransform.h"


namespace kvl
{



/**
 *
 * Given two filenames, this class will read in a cropped version of the first filename. It does this
 * by reading an affine transformation describing the mapping from image grid onto world coordinates
 * in SPM99's way (*.mat file) for each of the two images, computing the outer limits of the bounding
 * box of the second image in the first image (after calculating the mapping from the second image's
 * image grid coordinate system onto the first image's image grid using the affine matrices), and
 * cropping the first image accordingly.
 *
 */
class CroppedImageReader: public itk::Object
{
public :

  /** Standard class typedefs */
  typedef CroppedImageReader  Self;
  typedef itk::Object  Superclass;
  typedef itk::SmartPointer< Self >  Pointer;
  typedef itk::SmartPointer< const Self >  ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro( Self );

  /** Run-time type information (and related methods). */
  itkTypeMacro( CroppedImageReader, itk::Object );

  /** Some typedefs */
  typedef itk::Image< unsigned short, 3 >  ImageType;
  typedef itk::AffineTransform< float, 3 >  TransformType;

  // Read
  void  Read( const char* fileName, const char* boundingFileName = 0 );

  // Get image
  const ImageType*  GetImage() const
  {
    return m_Image;
  }

  // Get transform mapping the voxel grid of the second image into the
  // the voxel grid of the cropped first image
  const TransformType*  GetTransform() const
  {
    return m_Transform;
  }

  // Get the wordl-to-image transform of the cropped image, if the first voxel
  // is considered having index (0,0,0)^T
  const TransformType*  GetWorldToImageTransform() const
  {
    return m_WorldToImageTransform;
  }

  //
  void SetExtraFraction( float extraFraction )
  {
    m_ExtraFraction = extraFraction;
  }

  //
  float GetExtraFraction() const
  {
    return m_ExtraFraction;
  }

  //
  void SetDownSamplingFactor( int downSamplingFactor )
  {
    m_DownSamplingFactor = downSamplingFactor;
  }

  //
  int GetDownSamplingFactor() const
  {
    return m_DownSamplingFactor;
  }

  //
  const int* GetBoundingBoxSize() const
  {
    return m_BoundingBoxSize;
  }

  //
  const ImageType::RegionType&  GetCroppedImageRegion() const
  {
    return m_CroppedImageRegion;
  }

  //
  const ImageType::RegionType&  GetOriginalImageRegion() const
  {
    return m_OriginalImageRegion;
  }

  //
  const ImageType::RegionType&  GetOriginalImageOriginalRegion() const
  {
    return m_OriginalImageOriginalRegion;
  }


protected:
  CroppedImageReader();
  virtual ~CroppedImageReader();


private:
  CroppedImageReader(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  static TransformType::Pointer  GetTransformOfFileName( const std::string& filename );


  ImageType::Pointer  m_Image;
  TransformType::Pointer  m_Transform;
  TransformType::Pointer  m_WorldToImageTransform;

  float  m_ExtraFraction;
  int  m_DownSamplingFactor;
  int  m_BoundingBoxSize[ 3 ];

  ImageType::RegionType  m_CroppedImageRegion;
  ImageType::RegionType  m_OriginalImageRegion;
  ImageType::RegionType  m_OriginalImageOriginalRegion;


};


} // end namespace kvl

#endif

