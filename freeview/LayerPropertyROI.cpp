/**
 * @file  LayerPropertyROI.cxx
 * @brief Implementation for ROI layer properties.
 *
 * In 2D, the MRI is viewed as a single slice, and controls are
 * provided to change the color table and other viewing options. In
 * 3D, the MRI is viewed in three planes in 3D space, with controls to
 * move each plane axially.
 */
/*
 * Original Author: Kevin Teich
 * CVS Revision Info:
 *    $Author: nicks $
 *    $Date: 2011/03/14 23:44:47 $
 *    $Revision: 1.4 $
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
 *
 */


#include <assert.h>
#include "LayerPropertyROI.h"
#include "vtkRGBAColorTransferFunction.h"

using namespace std;

LayerPropertyROI::LayerPropertyROI ( QObject* parent) : LayerProperty( parent )
{
  mOpacity = 0.7;
  mRGB[0] = 1;
  mRGB[1] = 1;
  mRGB[2] = 0;

  mLUTTable = vtkSmartPointer<vtkRGBAColorTransferFunction>::New();
// mLUTTable->ClampingOff();
  this->SetColorMapChanged();

  connect( this, SIGNAL(ColorMapChanged()), this, SIGNAL(PropertyChanged()) );
  connect( this, SIGNAL(OpacityChanged(double)), this, SIGNAL(PropertyChanged()) );
}

LayerPropertyROI::~LayerPropertyROI ()
{}


vtkRGBAColorTransferFunction* LayerPropertyROI::GetLookupTable () const
{
  return mLUTTable;
}

void LayerPropertyROI::SetColorMapChanged()
{
  assert( mLUTTable.GetPointer() );

  mLUTTable->RemoveAllPoints();
  mLUTTable->AddRGBAPoint( 1-0.001, 0, 0, 0, 0 );
  mLUTTable->AddRGBAPoint( 1,    mRGB[0], mRGB[1], mRGB[2], 1 );
  mLUTTable->AddRGBAPoint( 100,  mRGB[0], mRGB[1], mRGB[2], 1 );

  mLUTTable->Build();

  // Notify the layers that use the color map stuff.
  emit ColorMapChanged();
}

void LayerPropertyROI::SetColor ( double r, double g, double b )
{
  mRGB[0] = r;
  mRGB[1] = g;
  mRGB[2] = b;
  this->SetColorMapChanged();
}

double LayerPropertyROI::GetOpacity() const
{
  return mOpacity;
}

void LayerPropertyROI::SetOpacity( double opacity )
{
  if ( mOpacity != opacity )
  {
    mOpacity = opacity;
    emit OpacityChanged( opacity );
  }
}

