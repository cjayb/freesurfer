#include <cstdlib>
#include <cstdio>

#include <iostream>
#include <iomanip>
#include <string>
using namespace std;

#include <boost/program_options.hpp>
namespace bpo = boost::program_options;

#include "mri.h"

#include "chronometer.hpp"

#ifdef FS_CUDA
#include "devicemanagement.h"
#include "mriconvolve_cuda.h"
#endif

// ==========================================================

const string inFilenameDefault = "norm.mgz";
const float sigmaDefault = 5;
const unsigned int repeatsDefault = 5;

#ifdef FS_CUDA
const string outFilenameDefault = "gpublur.mgz";
#else
const string outFilenameDefault = "cpublur.mgz";
#endif

string inFilename;
float sigma;
unsigned int repeats;
string outFilename;


const char* Progname = "convgaussian_test";

// ==========================================================

void ReadCommandLine( int ac, char* av[] ) {

  try {
    bpo::options_description desc("Allowed options");
    desc.add_options()
      ("help", "Produce help message" )
      ("input", bpo::value<string>(&inFilename)->default_value(inFilenameDefault), "Input filename" )
      ("output", bpo::value<string>(&outFilename)->default_value(outFilenameDefault), "Output filename" )
      ("sigma", bpo::value<float>(&sigma)->default_value(sigmaDefault), "Size of convolution gaussian (must be non-zero)" )
      ("repeats", bpo::value<unsigned int>(&repeats)->default_value(repeatsDefault), "Number of times to repeat" )
      ;
    
    bpo::variables_map vm;
    bpo::store( bpo::parse_command_line( ac, av, desc ), vm );
    bpo::notify( vm );
    
    if( vm.count( "help" ) ) {
      cout << desc << endl;
      exit( EXIT_SUCCESS );
    }
  }
  catch( exception& e ) {
    cerr << "Error: " << e.what() << endl;
    exit( EXIT_FAILURE );
  }
  catch( ... ) {
    cerr << "Unknown exception" << endl;
    exit( EXIT_FAILURE );
  }

  if( sigma <= 0 ) {
    cerr << "Must have positive sigma for convolution kernel" << endl;
    exit( EXIT_FAILURE );
  }
} 


// ==========================================================

int main( int argc, char *argv[] ) {

  SciGPU::Utilities::Chronometer tTotal;

  cout << "Gaussian Convolution Tester" << endl;
  cout << "===========================" << endl << endl;

#ifdef FS_CUDA
  AcquireCUDADevice();
#else
  cout << "CPU Version" << endl;
#endif
  cout << endl;

  ReadCommandLine( argc, argv );

  // ======================================
  
  // Read in the input
  cout << "Reading input file: " << inFilename << endl;
  MRI* input = MRIread( inFilename.c_str() );
  if( !input ) {
    cerr << "Failed to open input file: " << inFilename << endl;
    exit( EXIT_FAILURE );
  }

  // Print out some information
  printf( "Sizes are w=%i h=%i d=%i nFrames=%i\n",
	  input->width, input->height, input->depth, input->nframes );
  printf( "Datatype is %i\n\n", input->type );

  // Clone it for output
  MRI* output = NULL;
  output = MRIcopy( input, output );
  


  // ======================================

  // The zero prevents the kernel size being limited
  MRI* kernel = MRIgaussian1d( sigma, 0 );

  tTotal.Start();
  for( unsigned int i=0; i<repeats; i++ ) {
    MRIconvolveGaussian( input, output, kernel );
  }
  tTotal.Stop();

  cout << "Convolution complete in " <<
    setw(9) << setprecision(6) << tTotal.GetTime() / repeats << " ms" << endl;

  // ======================================
  // Write out the result

  cout << "Writing output file: " << outFilename << endl;
  MRIwrite( output, outFilename.c_str() );

  return( EXIT_SUCCESS );
}
