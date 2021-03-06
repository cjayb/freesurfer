/**
 * @file RegPowell.h
 * @brief A class to compute a robust registration using Powell
 *
 */

/*
 * Original Author: Martin Reuter
 * CVS Revision Info:
 *    $Author: nicks $
 *    $Date: 2011/03/02 00:04:24 $
 *    $Revision: 1.5 $
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

//
// written by Martin Reuter
// Apr. 22th ,2009
//

#ifndef RegPowell_H
#define RegPowell_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "matrix.h"
#include "mri.h"
#ifdef __cplusplus
}
#endif

#include <utility>
#include <string>
#include <vector>
#include "Registration.h"


class RegPowell : public Registration
{
public:
  RegPowell():Registration()
  {};
  RegPowell(MRI * s, MRI *t):Registration(s,t)
  {};
  virtual ~RegPowell()
  {};

  virtual void computeIterativeRegistration( int n,double epsit,MRI * mriS=NULL, MRI* mriT=NULL, const vnl_matrix < double > &Minit = vnl_matrix<double>(), double iscaleinit = 1.0);

protected:

  static float costFunction(float p[] );
  static RegPowell* tocurrent;
  static MRI * scf;
  static MRI * tcf;
  static int pcount;
  static vnl_matrix_fixed < double , 4, 4 > mh1;
  static vnl_matrix_fixed < double , 4, 4 > mh2;
  static int icount;

};


#endif

