/**
 * @file  mrilabels_cuda.cu
 * @brief Holds various MRI 'label' routines for the GPU
 *
 */
/*
 * Original Author: Richard Edgar
 * CVS Revision Info:
 *    $Author: rge21 $
 *    $Date: 2011/03/16 19:22:03 $
 *    $Revision: 1.7 $
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
#include <cstdlib>
#include <cstdio>

#include <iostream>
#include <iomanip>

#include "mri.h"

#include "chronometer.hpp"
#include "cudacheck.h"
#include "mriframegpu.hpp"
#include "ctfactory.hpp"

#include "fixedmap.hpp"

#include "mrilabels_cuda.hpp"

// ==============================================


//! Texture for unsigned char source
texture<unsigned char, 3, cudaReadModeElementType> dt_src_uchar;

//! Texture for unsigned char mri
texture<unsigned char, 3, cudaReadModeElementType> dt_mri_uchar;

//! Texture for unsigned char mri_vals
texture<unsigned char, 3, cudaReadModeElementType> dt_mri_vals_uchar;


namespace GPU {
  namespace Algorithms {

    //! Helper function to get texture values
    __device__
    unsigned char FetchSrcVoxel( const int ix,
				 const int iy,
				 const int iz ) {
      unsigned char texVal;

      texVal = tex3D( dt_src_uchar, ix+0.5f, iy+0.5f, iz+0.5f );

      return( texVal );
    }


    
    //! GPU kernel for MarkLabelBorderVoxel
    template<bool sixConnect>
    __global__
    void MarkLabelBorderVoxelKernel( GPU::Classes::MRIframeOnGPU<unsigned char> dst,
				     const int label,
				     const int mark ) {
      const unsigned int bx = ( blockIdx.x * blockDim.x );
      const unsigned int by = ( blockIdx.y * blockDim.y );
      const unsigned int ix = threadIdx.x + bx;
      const unsigned int iy = threadIdx.y + by;
      
      // Loop over z
      for( unsigned int iz=0; iz<dst.dims.z; iz++ ) {

	// Ensure we're in range
	if( dst.InVolume(ix,iy,iz) ) {
	  const int this_label = FetchSrcVoxel( ix, iy, iz );
	  int border = 0;

	  // Loop over local volume
	  for( int xk=-1; xk<=1 && !border; xk++ ) {
	    for( int yk=-1; yk<=1 && !border; yk++ ) {
	      for( int zk=-1; zk<=1; zk++ ) {

		if( sixConnect && (abs(xk)+abs(yk)+abs(zk) != 1) ) {
		  continue;
		}

		const int that_label = FetchSrcVoxel( ix+xk, iy+yk, iz+zk );
		if( ((this_label == label) && (that_label != label)) ||
		    ((this_label != label) && (that_label == label)) ) {
		  border = 1 ;
		  break ;
		}
	      }
	    }
	  }
	  if( border ) {
	    dst(ix,iy,iz) = mark;
	  } else {
	    dst(ix,iy,iz) = 0;
	  }
	}
      }
    }
    

    // =============================================

    void MRIlabels::MarkLabelBorderVoxels( const GPU::Classes::MRIframeGPU<unsigned char>& src,
					   GPU::Classes::MRIframeGPU<unsigned char>& dst,
					   const int label,
					   const int mark,
					   const int sixConnect ) const {
     
      
      MRIlabels::tMarkLabelBorderVoxelsTot.Start();

      // Verify we have output allocated
      dst.Allocate( src );

      // Get the input into a texture
      GPU::Classes::CTfactory srcArray( src, dt_src_uchar );

      // Run the kernel
      const unsigned int kKernelSize = 16;

      dim3 grid, threads;
      threads.x = threads.y = kKernelSize;
      threads.z = 1;

      grid = src.CoverBlocks( kKernelSize );
      grid.z = 1;

      if( sixConnect ) {
	MarkLabelBorderVoxelKernel<true><<<grid,threads>>>( dst, label, mark );
      } else {
	MarkLabelBorderVoxelKernel<false><<<grid,threads>>>( dst, label, mark );
      }
      CUDA_CHECK_ERROR( "MarkLabelBorderVoxelKernel failed!\n" );


      // Release the texture
      CUDA_SAFE_CALL( cudaUnbindTexture( dt_src_uchar ) );

      MRIlabels::tMarkLabelBorderVoxelsTot.Stop();
    }



    // ==============================================================

    // We use atomic float operations, so need Fermi GPU
#ifdef GCAMORPH_ON_GPU
    template<typename T, unsigned int nVals>
    __device__
    void ZeroArray( T* arr ) {
      for( unsigned int i=0; i<nVals; i++ ) {
	arr[i] = 0;
      }
    }

    //! Texture wrapper for mri
    __device__
    unsigned char FetchMRIVoxel( const int ix,
				 const int iy,
				 const int iz ) {

      unsigned char texVal;

      texVal = tex3D( dt_mri_uchar, ix+0.5f, iy+0.5f, iz+0.5f );

      return( texVal );
    }


    //! Texture wrapper for mri_vals
    __device__
    unsigned char FetchMRIval( const int ix,
			       const int iy,
			       const int iz ) {
      unsigned char texVal;
      texVal = tex3D( dt_mri_vals_uchar, ix+0.5f, iy+0.5f, iz+0.5f );

      return( texVal );
    }

    
    //! Implementation of MRIcomputeLabelNbhd
    template<typename T, typename U>
    __device__
    void ComputeLabelNbhd( const int x, const int y, const int z,
			   T& label_counts,
			   U& label_means,
			   const int whalf ) {
      /*!
	This is an implementation of MRIcomputeLabelNbhd specific
	to the VoxInLabelPartVolumeKernel
	The two input MRIs (always assumed present) are passed
	via textures, since the same two are always supplied in
	MRIvoxelsInLabelWithPartialVolumeEffects.
	The size of the arrays is passed via the template
	parameter nVals
      */
      
      label_counts.clear();
      label_means.clear();

      for( int zk=-whalf; zk<=whalf; zk++ ) {
	for( int yk=-whalf; yk<=whalf; yk++ ) {
	  for( int xk=-whalf; xk<=whalf; xk++ ) {
	    const int label = FetchMRIVoxel( x+xk, y+yk, z+zk );
	    label_counts[label]++;

	    const float val = FetchMRIval( x+xk, y+yk, z+zk );
	    label_means[label] += val;
	  }
	}
      }

      for( unsigned int i=0; i<label_counts.size(); i++ ) {
	if( label_counts.ValueByIndex(i) > 0 ) {
	  label_means.ValueByIndex(i) /= label_counts.ValueByIndex(i);
	}
      }
    }


    __global__
    void VoxInLabelPartVolumeKernel( const GPU::Classes::MRIframeOnGPU<unsigned char> mri_border,
				     GPU::Classes::MRIframeOnGPU<unsigned char> mri_nbr_labels,
				     GPU::Classes::MRIframeOnGPU<float> mri_mixing_coef,
				     const float vox_vol,
				     const int label,
				     float* volume ) {
      const unsigned int bx = ( blockIdx.x * blockDim.x );
      const unsigned int by = ( blockIdx.y * blockDim.y );
      const unsigned int ix = threadIdx.x + bx;
      const unsigned int iy = threadIdx.y + by;


      // Loop over z
      for( unsigned int iz=0; iz<mri_nbr_labels.dims.z; iz++ ) {

	

	// Ensure we're in range
	if( mri_nbr_labels.InVolume(ix,iy,iz) ) {

	  const int vox_label = FetchMRIVoxel(ix,iy,iz);
	  const int border = mri_border(ix,iy,iz);
 
	  if( (vox_label!=label) && (border==0) ) {
	    continue;
	  }

	  if( border == 0 ) {
	    atomicAdd( volume, vox_vol );
	  } else {
	    
	    GPU::Classes::FixedMap<int,int,32> nbr_label_counts( -1, 0 );
	    GPU::Classes::FixedMap<int,int,1024> label_counts( -1, 0 );
	    GPU::Classes::FixedMap<int,float,1024> label_means(-1, 0 );

	    ComputeLabelNbhd( ix, iy, iz, nbr_label_counts, label_means, 1 );
	    ComputeLabelNbhd( ix, iy, iz, label_counts, label_means, 7 );

	    const float val = FetchMRIval( ix, iy, iz );

	    const float mean_label = label_means[vox_label];
	    int nbr_label = -1;
	    int max_count = 0;
	    float pv, mean_nbr;

	    /*
	      look for a label that is a nbr and is
	      on the other side of val from the label mean
	    */


	    for( unsigned int i=0; i<nbr_label_counts.size(); i++ ) {

	      const GPU::Classes::KVpair<int,int> nlpair = nbr_label_counts.AccessByIndex(i);

	      const int this_label = nlpair.key;

	      if( this_label == vox_label ) {
		continue ;
	      }

	      /*
		// Guaranteed to work
	      if( nbr_label_counts[this_label] == 0 ) {
		continue ;
	      }
	      */

	      if( (label_counts[this_label] > max_count) &&
		  ((label_means[this_label] - val) *
		   (mean_label - val) < 0) ) {
		max_count = label_means[this_label] ;
		nbr_label = this_label ;
	      }
	    }

	    if( vox_label != label && nbr_label != label ) {
	      continue; // this struct not in voxel 
	    }
	    

	    if( max_count == 0 ) {
	      atomicAdd( volume, vox_vol ); // couldn't find an appropriate label
	      
	      // find max nbr label anyway for caller
	      for( unsigned int i=0; i<nbr_label_counts.size(); i++ ) {
		  
		const GPU::Classes::KVpair<int,int> nlpair = nbr_label_counts.AccessByIndex(i);

		const int this_label = nlpair.key;

		if( this_label == vox_label ) {
		  continue;
		}
		
		/*!
		  // Guaranteed to be always false
		if( nbr_label_counts[this_label] == 0 ) {
		  continue ;
		}
		*/
		  
		if( label_counts[this_label] > max_count ) {
		  max_count = label_means[this_label] ;
		  nbr_label = this_label ;
		}
	      }
		
	      mri_nbr_labels( ix, iy, iz ) = nbr_label;
	      mri_mixing_coef( ix, iy, iz ) = 1;	
	      
	      
	    } else {
	      // compute partial volume pct 
	      mean_nbr = label_means[nbr_label] ;
	      pv = (val - mean_nbr) / (mean_label - mean_nbr) ;
	      
	      if (pv > 1) {
		pv = 1 ;
	      }
	      
	      if (pv < 0) {
		continue ;  // shouldn't happen
	      }
	      
	      if( vox_label != label ) {
		pv = 1-pv ;
	      }
	      
	      atomicAdd( volume, vox_vol * pv );
	      
	      mri_mixing_coef( ix, iy, iz ) = pv;
	      
	      
	      if (vox_label != label) {
		mri_nbr_labels( ix, iy, iz ) = vox_label;
	      } else {
		mri_nbr_labels( ix, iy, iz ) = nbr_label;
	      }
	      
	      
	    }
	    
	  }
	}
      }
    }




#endif



    // ===========================

    //! GPU implementation of MRIvoxelsInLabelWithPartialVolumeEffects
    float MRIlabels::VoxInLabelWithPartialVolume( const GPU::Classes::MRIframeGPU<unsigned char>& mri,
						  const GPU::Classes::MRIframeGPU<unsigned char>& mri_vals,
						  const int label,
						  GPU::Classes::MRIframeGPU<float>& mri_mixing_coeff,
						  GPU::Classes::MRIframeGPU<unsigned char>& mri_nbr_labels ) const {
      /*!
	This GPU implementation of MRIvoxelsInLabelWithPartialVolumeEffects
	assumes that both input MRIs (mri and mri_vals) are of type
	unsigned char.
      */
#ifdef GCAMORPH_ON_GPU
      MRIlabels::tVoxInLabelPartVolumeTot.Start();

      // Allocate  and zero the 'volume' global
      float *d_volume, h_volume;
      CUDA_SAFE_CALL( cudaMalloc( (void**)&(d_volume), sizeof(float) ) );
      CUDA_SAFE_CALL( cudaMemset( d_volume, 0, sizeof(float) ) );

      // Set up vox_vol
      const float3 sizes = mri.GetSizes();
      const float vox_vol = sizes.x*sizes.y*sizes.z;

      // Allocate the output MRIs
      mri_mixing_coeff.Allocate( mri );
      mri_mixing_coeff.Zero();
      mri_nbr_labels.Allocate( mri );
      mri_nbr_labels.Zero();

      // Declare the 'border' MRI
      GPU::Classes::MRIframeGPU<unsigned char> mriBorder;


      // Call the mark label border method
      this->MarkLabelBorderVoxels( mri, mriBorder, label, 1, 1 );


      // Set up the textures
      GPU::Classes::CTfactory mriArray( mri, dt_mri_uchar );
      GPU::Classes::CTfactory mri_valsArray( mri_vals, dt_mri_vals_uchar );

      
      // Run the computation
      const unsigned int kKernelSize = 16;

      dim3 grid, threads;
      threads.x = threads.y = kKernelSize;
      threads.z = 1;

      grid = mri.CoverBlocks( kKernelSize );
      grid.z = 1;
      MRIlabels::tVoxInLabelPartVolumeCompute.Start();
      VoxInLabelPartVolumeKernel<<<grid,threads>>>( mriBorder,
						    mri_nbr_labels,
						    mri_mixing_coeff,
						    vox_vol,
						    label,
						    d_volume );
      CUDA_CHECK_ERROR( "VoxInLabelPartVolumeKernel failed!\n" );
      MRIlabels::tVoxInLabelPartVolumeCompute.Stop();

      // Unbind textures
      CUDA_SAFE_CALL( cudaUnbindTexture( dt_mri_uchar ) );
      CUDA_SAFE_CALL( cudaUnbindTexture( dt_mri_vals_uchar ) );


      // Retrieve the volume global and release
      CUDA_SAFE_CALL( cudaMemcpy( &h_volume, d_volume,
				  sizeof(float),
				  cudaMemcpyDeviceToHost ) );
      CUDA_SAFE_CALL( cudaFree( d_volume ) );

      MRIlabels::tVoxInLabelPartVolumeTot.Stop();

      return( h_volume );
#else
      std::cerr << __FUNCTION__
		<< ": Requires Fermi class GPU"
		<< std::endl;
      abort();
      return(0);
#endif
    }



    // ==============================================================

    // Declare statics
    SciGPU::Utilities::Chronometer MRIlabels::tMarkLabelBorderVoxelsTot;

    SciGPU::Utilities::Chronometer MRIlabels::tVoxInLabelPartVolumeTot;
    SciGPU::Utilities::Chronometer MRIlabels::tVoxInLabelPartVolumeCompute;


    void MRIlabels::ShowTimings( void ) {
      std::cout << "=============================================" << std::endl;
      std::cout << "GPU MRI Label timers" << std::endl;
      std::cout << "--------------------" << std::endl;
#ifndef CUDA_FORCE_SYNC
      std::cout << "WARNING: CUDA_FORCE_SYNC not #defined" << std::endl;
      std::cout << "Timings may not be accurate" << std::endl;
#endif
      std::cout << std::endl;

      std::cout << "MarkLabelBorderVoxels" << std::endl;
      std::cout << "Total : " << MRIlabels::tMarkLabelBorderVoxelsTot << std::endl;

      std::cout << "VoxInLabelWithPartialVolume" << std::endl;
      std::cout << "  Compute : " << MRIlabels::tVoxInLabelPartVolumeCompute << std::endl;
      std::cout << "Total       : " << MRIlabels::tVoxInLabelPartVolumeTot << std::endl;

      std::cout << "=============================================" << std::endl;

    }

  }
}

// ===============================================

static GPU::Algorithms::MRIlabels myLabels;

//! Wrapper for MRImarkLabelBorderVoxels
void MRImarkLabelBorderVoxelsGPU( const MRI* mri_src,
				  MRI* mri_dst,
				  int label,
				  int mark,
				  int six_connected ) {
  GPU::Classes::MRIframeGPU<unsigned char> srcGPU;
  GPU::Classes::MRIframeGPU<unsigned char> dstGPU;

  srcGPU.Allocate( mri_src );
  srcGPU.VerifyMRI( mri_src );

  srcGPU.SendFrame( mri_src, 0 );

  myLabels.MarkLabelBorderVoxels( srcGPU, dstGPU, label, mark, six_connected );

  dstGPU.RecvFrame( mri_dst, 0 );
}



//! Wrapper for MRIvoxelsInLabelWithPartialVolumeEffects

float MRIvoxelsInLabelWithPartialVolumeEffectsGPU( const MRI *mri,
						   const MRI *mri_vals, 
						   const int label,
						   MRI *mri_mixing_coef, 
						   MRI *mri_nbr_labels ) {
  
  GPU::Classes::MRIframeGPU<unsigned char> mriGPU, mri_valsGPU;
  GPU::Classes::MRIframeGPU<unsigned char> mri_nbr_labelsGPU;
  GPU::Classes::MRIframeGPU<float> mri_mixing_coefGPU;

  // Send data to GPU
  mriGPU.Allocate( mri );
  mriGPU.VerifyMRI( mri );
  mriGPU.Send( mri, 0 );

  mri_valsGPU.Allocate( mri_vals );
  mri_valsGPU.VerifyMRI( mri_vals );
  mri_valsGPU.Send( mri_vals, 0 );

  // Run computation
  float vol = myLabels.VoxInLabelWithPartialVolume( mriGPU, mri_valsGPU,
						    label,
						    mri_mixing_coefGPU,
						    mri_nbr_labelsGPU );

  // Retrieve results
  if( mri_mixing_coef ) {
    mri_mixing_coefGPU.RecvFrame( mri_mixing_coef, 0 );
  }
  if( mri_nbr_labels ) {
    mri_nbr_labelsGPU.RecvFrame( mri_nbr_labels, 0 );
  }

  return( vol );
}
