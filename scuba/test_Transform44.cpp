/**
 * @file  test_Transform44.cpp
 * @brief test 4x4 Transform routines
 *
 */
/*
 * Original Author: Kevin Teich
 * CVS Revision Info:
 *    $Author: nicks $
 *    $Date: 2011/03/02 00:04:39 $
 *    $Revision: 1.10 $
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


#include <sstream>
#include <fstream>
#include "Transform44.h"
extern "C" {
#include "macros.h"
}
#include "Scuba-impl.h"

const char* Progname = "test_Transform44";

using namespace std;

#define Assert(x,s)   \
  if(!(x)) { \
  stringstream ss; \
  ss << "Line " << __LINE__ << ": " << s; \
  cerr << ss.str().c_str() << endl; \
  throw runtime_error( ss.str() ); \
  }

#define VFEQUAL(v,a,b,c) \
   (FEQUAL(((v)[0]),a) && FEQUAL(((v)[1]),b) && FEQUAL(((v)[2]),c))

class Transform44Tester {
public:
  void Test();
};

void
Transform44Tester::Test () {

  stringstream ssError;

  try {

    Transform44 transform;

    // Set to identity, make sure multing a vector returns same vector.
    float in[3], out[3];
    in[0] = 5;
    in[1] = 35.67;
    in[2] = 1000;
    transform.MultiplyVector3( in, out );
    Assert((in[0] == out[0] && in[1] == out[1] && in[2] == out[2]),
           "Identity mult check failed");


    // Set to a scale matrix, make sure multing a vector returns
    // correct response.
    transform.SetMainTransform( 5, 0, 0, 0,
                                0, 5, 0, 0,
                                0, 0, 5, 0,
                                0, 0, 0, 1 );

    in[0] = 5;
    in[1] = 6;
    in[2] = 7;
    transform.MultiplyVector3( in, out );
    if ( !(in[0]*5 == out[0] && in[1]*5 == out[1] && in[2]*5 == out[2]) ) {
      ssError << "Scale mult check failed" << endl
      << transform << endl
      << "out " << Point3<float>(out) << endl;
      throw(runtime_error(ssError.str()));
    }

    // Check the inverse.
    transform.InvMultiplyVector3( out, in );
    Assert((FEQUAL( in[0], 5.0 ) &&
            FEQUAL( in[1], 6.0 ) && FEQUAL( in[2], 7.0) ),
           "Inv scale mult check failed");

    // Try loading an LTA.
    string fnLTA = "test_data/testTransform44.lta";
    ifstream fLTA( fnLTA.c_str(), ios::in );
    if ( !fLTA ) {
      cerr << "WARNING: File " + fnLTA + " not found, test skipped." << endl;
    } else {

      Transform44 l;
      l.LoadFromLTAFile( fnLTA );
      Assert((l(0,0) == 1 && l(1,0) == 2 && l(2,0) == 3 && l(3,0) == 4 &&
              l(0,1) == 5 && l(1,1) == 6 && l(2,1) == 7 && l(3,1) == 8 &&
              l(0,2) == 9 && l(1,2) == 10 && l(2,2) == 11 && l(3,2) == 12 &&
              l(0,3) == 13 && l(1,3) == 14 && l(2,3) == 15 && l(3,3) == 16),
             "LTA didn't load properly.");
    }
  } catch ( runtime_error& e ) {
    cerr << "failed with exception: " << e.what() << endl;
    exit( 1 );
  } catch (...) {
    cerr << "failed" << endl;
    exit( 1 );
  }
}


int main ( int argc, char** argv ) {

  cerr << "Beginning test" << endl;

  try {

    Transform44Tester tester0;
    tester0.Test();

  } catch ( runtime_error& e ) {
    cerr << "failed with exception: " << e.what() << endl;
    exit( 1 );
  } catch (...) {
    cerr << "failed" << endl;
    exit( 1 );
  }

  cerr << "Success" << endl;

  exit( 0 );
}

