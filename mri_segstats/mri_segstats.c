/**
 * @file  mri_segstats.c
 * @brief Computes statistics from a segmentation.
 *
 * This program will compute statistics on segmented volumes. In its
 * simplist invocation, it will report on the number of voxels and
 * volume in each segmentation. However, it can also compute statistics
 * on the segmentation based on the values from another volume. This
 * includes computing waveforms averaged inside each segmentation.
 */
/*
 * Original Author: Dougas N Greve
 * CVS Revision Info:
 *    $Author: nicks $
 *    $Date: 2011/04/27 22:18:58 $
 *    $Revision: 1.75.2.2 $
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

/*
   Subcort stuff that needs to be removed from the surface-based white
   matter volume:

   16  Brain-Stem                            119  159  176    0

    4  Left-Lateral-Ventricle                120   18  134    0
   10  Left-Thalamus-Proper                    0  118   14    0
   11  Left-Caudate                          122  186  220    0
   12  Left-Putamen                          236   13  176    0
   13  Left-Pallidum                          12   48  255    0
   17  Left-Hippocampus                      220  216   20    0
   18  Left-Amygdala                         103  255  255    0
   26  Left-Accumbens-area                   255  165    0    0
   28  Left-VentralDC                        165   42   42    0

   43  Right-Lateral-Ventricle               120   18  134    0
   49  Right-Thalamus-Proper                   0  118   14    0
   50  Right-Caudate                         122  186  220    0
   51  Right-Putamen                         236   13  176    0
   52  Right-Pallidum                         13   48  255    0
   53  Right-Hippocampus                     220  216   20    0
   54  Right-Amygdala                        103  255  255    0
   58  Right-Accumbens-area                  255  165    0    0
   60  Right-VentralDC                       165   42   42    0

*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/utsname.h>

#include "macros.h"
#include "mrisurf.h"
#include "mrisutils.h"
#include "utils.h"
#include "error.h"
#include "diag.h"
#include "mri.h"
#include "mri2.h"
#include "version.h"
#include "cma.h"
#include "gca.h"
#include "fsenv.h"
#include "annotation.h"
#include "registerio.h"
#include "cmdargs.h"

#ifdef FS_CUDA
#include "devicemanagement.h"
#endif

static int  parse_commandline(int argc, char **argv);
static void check_options(void);
static void print_usage(void) ;
static void usage_exit(void);
static void print_help(void) ;
static void print_version(void) ;
static void argnerr(char *option, int n);
static void dump_options(FILE *fp);
static int  singledash(char *flag);

typedef struct
{
  int id;
  char name[1000];
  int nhits;
  float vol;
  int red, green, blue; // 0-255
  float min, max, range, mean, std, snr;
}
STATSUMENTRY;

int MRIsegFrameAvg(MRI *seg, int segid, MRI *mri, double *favg);
int MRIsegCount(MRI *seg, int id, int frame);
int MRIsegStats(MRI *seg, int segid, MRI *mri,  int frame,
                float *min, float *max, float *range,
                float *mean, float *std);
STATSUMENTRY *LoadStatSumFile(char *fname, int *nsegid);
int DumpStatSumTable(STATSUMENTRY *StatSumTable, int nsegid);


int main(int argc, char *argv[]) ;

static char vcid[] =
  "$Id: mri_segstats.c,v 1.75.2.2 2011/04/27 22:18:58 nicks Exp $";
char *Progname = NULL, *SUBJECTS_DIR = NULL, *FREESURFER_HOME=NULL;
char *SegVolFile = NULL;
char *InVolFile = NULL;
char *InVolRegFile = NULL;
MATRIX *InVolReg = NULL;
int InVolRegHeader = 0;
char *InIntensityName = "";
char *InIntensityUnits = "unknown";
char *MaskVolFile = NULL;
char *PVVolFile = NULL;
char *BrainMaskFile = NULL;
char *StatTableFile = NULL;
char *FrameAvgFile = NULL;
char *FrameAvgVolFile = NULL;
char *SpatFrameAvgFile = NULL;
int DoFrameAvg = 0;
int frame = 0;
int synth = 0;
int debug = 0;
int dontrun = 0;
long seed = 0;
MRI *seg, *invol, *famri, *maskvol, *pvvol, *brainvol, *mri_aseg, *mri_ribbon;
int nsegid0, *segidlist0;
int nsegid, *segidlist;
int NonEmptyOnly = 1;
int UserSegIdList[1000];
int nUserSegIdList = 0;
int DoExclSegId = 0, nExcl = 0, ExclSegIdList[1000], ExclSegId;
int DoExclCtxGMWM= 0;
int DoSurfCtxVol = 0;
int DoSurfWMVol = 0;
double lhwhitevol;
double rhwhitevol;
double lhwhitevolTot, lhpialvolTot, lhctxvol;
double rhwhitevolTot, rhpialvolTot, rhctxvol;
int DoSupraTent = 0;
double SupraTentVol, SupraTentVolCor;

char *gcafile = NULL;
GCA *gca;

float maskthresh = 0.5;
int   maskinvert = 0, maskframe = 0;
char *masksign=NULL;
int   maskerode = 0;
int   nmaskhits;
int   nbrainsegvoxels = 0;
double brainsegvolume = 0;
double brainsegvolume2 = 0;
int DoSubCortGrayVol = 0;
double SubCortGrayVol = 0;
int DoTotalGrayVol = 0;
int   nbrainmaskvoxels = 0;
double brainmaskvolume = 0;
int   BrainVolFromSeg = 0;
int   DoETIV = 0;
int   DoETIVonly = 0;
int   DoOldETIVonly = 0;
char *talxfmfile = NULL;

char *ctabfile = NULL;
COLOR_TABLE *ctab = NULL;
STATSUMENTRY *StatSumTable = NULL;
STATSUMENTRY *StatSumTable2 = NULL;
char *ctabfileOut = NULL;

MRIS *mris;
char *subject = NULL;
char *hemi    = NULL;
char *annot   = NULL;

int Vox[3], DoVox = 0;
int  segbase = -1000;

int DoSquare = 0;
int DoSquareRoot = 0;
char *LabelFile = NULL;

int DoMultiply = 0;
double MultVal = 0;

int DoSNR = 0;
struct utsname uts;
char *cmdline, cwd[2000];

/*--------------------------------------------------*/
int main(int argc, char **argv)
{
  int nargs, n, nx, n0, skip, nhits, f, nsegidrep, id, ind, nthsegid;
  int c,r,s,err,DoContinue;
  float voxelvolume,vol;
  float min, max, range, mean, std, snr;
  FILE *fp;
  double  **favg, *favgmn;
  char tmpstr[1000];
  double atlas_icv=0;
  int ntotalsegid=0;
  int valid;
  int usersegid=0;
  LABEL *label;
  MRI *tmp;
  MATRIX *vox2vox = NULL;
  nhits = 0;
  vol = 0;

#if FS_CUDA
  AcquireCUDADevice();
#endif

  /* rkt: check for and handle version tag */
  nargs = handle_version_option (argc, argv, vcid, "$Name: stable5 $");
  if (nargs && argc - nargs == 1)
  {
    exit (0);
  }
  argc -= nargs;

  cmdline = argv2cmdline(argc,argv);
  uname(&uts);

  Progname = argv[0] ;
  argc --;
  argv++;
  ErrorInit(NULL, NULL, NULL) ;
  DiagInit(NULL, NULL, NULL) ;

  if (argc == 0)
  {
    usage_exit();
  }

  parse_commandline(argc, argv);
  check_options();

  dump_options(stdout);

  if (subject != NULL)
  {
    SUBJECTS_DIR = getenv("SUBJECTS_DIR");
    if (SUBJECTS_DIR==NULL)
    {
      fprintf(stderr,"ERROR: SUBJECTS_DIR not defined in environment\n");
      exit(1);
    }
  }

  if (DoETIV || DoETIVonly || DoOldETIVonly)
  {
    // calc total intracranial volume estimation
    // see this page:
    // http://surfer.nmr.mgh.harvard.edu/fswiki/eTIV
    // for info on determining the scaling factor.
    // a factor of 1948 was found to be best when using talairach.xfm
    // however when using talairach_with_skull.lta, 2150 was used
    double etiv_scale_factor = 1948.106;
    if (talxfmfile)
    {
      // path to talairach.xfm file spec'd on the command line
      sprintf(tmpstr,"%s",talxfmfile);
    }
    else
    {
      sprintf
      (tmpstr,
       "%s/%s/mri/transforms/talairach.xfm",
       SUBJECTS_DIR,
       subject);
    }
    if (DoOldETIVonly)
    {
      // back-door way to get the old way of calculating etiv, for debug
      sprintf
      (tmpstr,
       "%s/%s/mri/transforms/talairach_with_skull.lta",
       SUBJECTS_DIR,
       subject);
      etiv_scale_factor = 2150;
    }
    double determinant = 0;
    atlas_icv = MRIestimateTIV(tmpstr,etiv_scale_factor,&determinant);
    printf("atlas_icv (eTIV) = %d mm^3    (det: %3f )\n",
           (int)atlas_icv,determinant);
    if (DoETIVonly || DoOldETIVonly)
    {
      exit(0);
    }
  }

  /* Make sure we can open the output summary table file*/
  if(StatTableFile)
  {
    fp = fopen(StatTableFile,"w");
    if (fp == NULL)
    {
      printf("ERROR: could not open %s for writing\n",StatTableFile);
      int err = errno;
      printf("Errno: %s\n", strerror(err) );
      exit(1);
    }
    fclose(fp);
  }

  /* Make sure we can open the output frame average file*/
  if (FrameAvgFile != NULL)
  {
    fp = fopen(FrameAvgFile,"w");
    if (fp == NULL)
    {
      printf("ERROR: could not open %s for writing\n",FrameAvgFile);
      exit(1);
    }
    fclose(fp);
  }

  /* Load the segmentation */
  if (SegVolFile)
  {
    printf("Loading %s\n",SegVolFile);
    seg = MRIread(SegVolFile);
    if (seg == NULL)
    {
      printf("ERROR: loading %s\n",SegVolFile);
      exit(1);
    }
    if(DoVox)
    {
      printf("Replacing seg with a single voxel at %d %d %d\n",
             Vox[0],Vox[1],Vox[2]);
      for(c=0; c < seg->width; c++)
      {
        for(r=0; r < seg->height; r++)
        {
          for(s=0; s < seg->depth; s++)
          {
            if(c == Vox[0] && r == Vox[1] && s == Vox[2])
            {
              MRIsetVoxVal(seg,c,r,s,0, 1);
            }
            else
            {
              MRIsetVoxVal(seg,c,r,s,0, 0);
            }
          }
        }
      }
    }
  }
  else if(annot)
  {
    printf("Constructing seg from annotation\n");
    sprintf(tmpstr,"%s/%s/surf/%s.white",SUBJECTS_DIR,subject,hemi);
    mris = MRISread(tmpstr);
    if (mris==NULL)
    {
      exit(1);
    }
    sprintf(tmpstr,"%s/%s/label/%s.%s.annot",SUBJECTS_DIR,subject,hemi,annot);
    err = MRISreadAnnotation(mris, tmpstr);
    if (err)
    {
      exit(1);
    }
    if(segbase == -1000)
    {
      // segbase has not been set with --segbase
      if(!strcmp(annot,"aparc"))
      {
        if(!strcmp(hemi,"lh"))
        {
          segbase = 1000;
        }
        else
        {
          segbase = 2000;
        }
      }
      else if(!strcmp(annot,"aparc.a2005s"))
      {
        if(!strcmp(hemi,"lh"))
        {
          segbase = 1100;
        }
        else
        {
          segbase = 2100;
        }
      }
      else
      {
        segbase = 0;
      }
    }
    printf("Seg base %d\n",segbase);
    seg = MRISannot2seg(mris,segbase);
    // Now create a colortable in a temp location to be read out below (hokey)
    mris->ct->idbase = segbase;
    if (mris->ct)
    {
      sprintf(tmpstr,"/tmp/mri_segstats.tmp.%s.%s.%d.ctab",subject,hemi,
              nint(randomNumber(0, 255)));
      ctabfile = strcpyalloc(tmpstr);
      CTABwriteFileASCII(mris->ct,ctabfile);
    }
  }
  else
  {
    printf("Constructing seg from label\n");
    label = LabelRead(NULL, LabelFile);
    if(label == NULL)
    {
      exit(1);
    }
    sprintf(tmpstr,"%s/%s/surf/%s.white",SUBJECTS_DIR,subject,hemi);
    mris = MRISread(tmpstr);
    if (mris==NULL)
    {
      exit(1);
    }
    seg = MRIalloc(mris->nvertices,1,1,MRI_INT);
    for (n = 0; n < label->n_points; n++)
    {
      MRIsetVoxVal(seg,label->lv[n].vno,0,0,0, 1);
    }
  }

  if (ctabfile != NULL)
  {
    /* Load the color table file */
    ctab = CTABreadASCII(ctabfile);
    if (ctab == NULL)
    {
      printf("ERROR: reading %s\n",ctabfile);
      exit(1);
    }
  }

  if (gcafile != NULL)
  {
    gca = GCAread(gcafile);
    if (gca == NULL)
    {
      printf("ERROR: reading %s\n",gcafile);
      exit(1);
    }
    ctab = GCAcolorTableCMA(gca);
  }

  if(DoSurfWMVol)
  {
    printf("Getting Cerebral WM volumes from surface\n");

    sprintf(tmpstr,"%s/%s/mri/aseg.mgz",SUBJECTS_DIR,subject);
    mri_aseg = MRIread(tmpstr);
    if(mri_aseg == NULL)
    {
      exit(1);
    }

    sprintf(tmpstr,"%s/%s/surf/lh.white",SUBJECTS_DIR,subject);
    mris = MRISread(tmpstr);
    if(mris==NULL)
    {
      exit(1);
    }
    lhwhitevol = MRIScomputeWhiteVolume(mris, mri_aseg, 1.0/4.0);
    MRISfree(&mris);
    printf("lh white matter volume %g\n",lhwhitevol);

    sprintf(tmpstr,"%s/%s/surf/rh.white",SUBJECTS_DIR,subject);
    mris = MRISread(tmpstr);
    if(mris==NULL)
    {
      exit(1);
    }
    rhwhitevol = MRIScomputeWhiteVolume(mris, mri_aseg, 1.0/4.0);
    MRISfree(&mris);
    printf("rh white matter volume %g\n",rhwhitevol);

    MRIfree(&mri_aseg);
  }

  if(DoSurfCtxVol)
  {
    printf("Getting Cerebral GM and WM volumes from surfaces\n");
    // Does this include the non-cortical areas of the surface?

    sprintf(tmpstr,"%s/%s/surf/lh.white",SUBJECTS_DIR,subject);
    mris = MRISread(tmpstr);
    if(mris==NULL)
    {
      exit(1);
    }
    lhwhitevolTot = MRISvolumeInSurf(mris);
    MRISfree(&mris);

    sprintf(tmpstr,"%s/%s/surf/lh.pial",SUBJECTS_DIR,subject);
    mris = MRISread(tmpstr);
    if(mris==NULL)
    {
      exit(1);
    }
    lhpialvolTot = MRISvolumeInSurf(mris);
    lhctxvol = lhpialvolTot - lhwhitevolTot;
    MRISfree(&mris);

    sprintf(tmpstr,"%s/%s/surf/rh.white",SUBJECTS_DIR,subject);
    mris = MRISread(tmpstr);
    if(mris==NULL)
    {
      exit(1);
    }
    rhwhitevolTot = MRISvolumeInSurf(mris);
    MRISfree(&mris);

    sprintf(tmpstr,"%s/%s/surf/rh.pial",SUBJECTS_DIR,subject);
    mris = MRISread(tmpstr);
    if(mris==NULL)
    {
      exit(1);
    }
    rhpialvolTot = MRISvolumeInSurf(mris);
    rhctxvol = rhpialvolTot - rhwhitevolTot;
    MRISfree(&mris);
    mris = NULL;

    printf("lh surface-based volumes (mm3): wTot = %lf,  pTot = %lf c = %lf \n",
           lhwhitevolTot,lhpialvolTot,lhctxvol);
    printf("rh surface-based volumes (mm3): wTot = %lf,  pTot = %lf c = %lf \n",
           rhwhitevolTot,rhpialvolTot,rhctxvol);
    fflush(stdout);

    if(DoSupraTent)
    {
      printf("Computing SupraTentVolCor\n");
      sprintf(tmpstr,"%s/%s/mri/aseg.mgz",SUBJECTS_DIR,subject);
      mri_aseg = MRIread(tmpstr);
      if(mri_aseg == NULL)
      {
        exit(1);
      }
      sprintf(tmpstr,"%s/%s/mri/ribbon.mgz",SUBJECTS_DIR,subject);
      mri_ribbon = MRIread(tmpstr);
      if(mri_ribbon == NULL)
      {
        exit(1);
      }
      SupraTentVolCor = SupraTentorialVolCorrection(mri_aseg, mri_ribbon);
      SupraTentVol = SupraTentVolCor + lhpialvolTot + rhpialvolTot;
      printf("SupraTentVolCor = %8.3f\n",SupraTentVolCor);
      printf("SupraTentVol = %8.3f\n",SupraTentVol);
      MRIfree(&mri_aseg);
      MRIfree(&mri_ribbon);
    }
  }


  /* Load the input volume */
  if (InVolFile != NULL)
  {
    printf("Loading %s\n",InVolFile);
    fflush(stdout);
    invol = MRIread(InVolFile);
    if(invol == NULL)
    {
      printf("ERROR: loading %s\n",InVolFile);
      exit(1);
    }
    if(frame >= invol->nframes)
    {
      printf("ERROR: input frame = %d, input volume only has %d frames\n",
             frame,invol->nframes);
      exit(1);
    }
    if(InVolReg || InVolRegHeader)
    {
      if(InVolReg)
      {
        printf("Input Volume Registration:\n");
        MatrixPrint(stdout,InVolReg);
      }
      vox2vox = MRIvoxToVoxFromTkRegMtx(invol, seg, InVolReg);
      printf("Input Volume vox2vox:\n");
      MatrixPrint(stdout,vox2vox);
      printf("Allocating %d frames\n",invol->nframes);
      tmp = MRIcloneBySpace(seg,-1,invol->nframes);
      printf("Vol2Vol\n");
      err = MRIvol2VolVSM(invol,tmp,vox2vox,SAMPLE_NEAREST,-1,NULL);
      if(err)
      {
        exit(1);
      }
      MRIfree(&invol);
      invol = tmp;
    }
    if(MRIdimMismatch(invol,seg,0))
    {
      printf("ERROR: dimension mismatch between input volume and seg\n");
      printf("  input %d %d %d\n",invol->width,invol->height,invol->depth);
      printf("  seg   %d %d %d\n",seg->width,seg->height,seg->depth);
      exit(1);
    }
    if(DoMultiply)
    {
      printf("Multiplying input by %lf\n",MultVal);
      MRImultiplyConst(invol,MultVal,invol);
    }
    if(DoSquare)
    {
      printf("Computing square of input\n");
      MRIsquare(invol, NULL, invol);
    }
    if(DoSquareRoot)
    {
      printf("Computing square root of input\n");
      MRIsquareRoot(invol, NULL, invol);
    }
  }

  /* Load the partial volume mri */
  if (PVVolFile != NULL)
  {
    printf("Loading %s\n",PVVolFile);
    fflush(stdout);
    pvvol = MRIread(PVVolFile);
    if (pvvol == NULL)
    {
      printf("ERROR: loading %s\n",PVVolFile);
      exit(1);
    }
    if(MRIdimMismatch(pvvol,seg,0))
    {
      printf("ERROR: dimension mismatch between PV volume and seg\n");
      printf("  pvvol %d %d %d\n",pvvol->width,pvvol->height,pvvol->depth);
      printf("  seg   %d %d %d\n",seg->width,seg->height,seg->depth);
      exit(1);
    }
  }

  /* Load the brain volume */
  if (BrainMaskFile != NULL)
  {
    printf("Loading %s\n",BrainMaskFile);
    fflush(stdout);
    brainvol = MRIread(BrainMaskFile);
    if(brainvol == NULL)
    {
      printf("ERROR: loading %s\n",BrainMaskFile);
      exit(1);
    }
    if(MRIdimMismatch(brainvol,seg,0))
    {
      printf("ERROR: dimension mismatch between brain volume and seg\n");
      exit(1);
    }
    nbrainmaskvoxels = MRItotalVoxelsOn(brainvol, WM_MIN_VAL) ;
    brainmaskvolume =
      nbrainmaskvoxels * brainvol->xsize * brainvol->ysize * brainvol->zsize;
    MRIfree(&brainvol) ;
    printf("# nbrainmaskvoxels %d\n",nbrainmaskvoxels);
    printf("# brainmaskvolume %10.1lf\n",brainmaskvolume);
  }

  if (BrainVolFromSeg)
  {
    nbrainsegvoxels = 0;
    for (n = 0 ; n <= MAX_CMA_LABEL ; n++)
    {
      if (!IS_BRAIN(n))
      {
        continue ;
      }
      nbrainsegvoxels += MRIvoxelsInLabel(seg, n) ;
    }
    brainsegvolume = nbrainsegvoxels * seg->xsize * seg->ysize * seg->zsize;
    printf("# nbrainsegvoxels %d\n",nbrainsegvoxels);
    printf("# brainsegvolume %10.1lf\n",brainsegvolume);
  }

  /* Load the mask volume */
  if (MaskVolFile != NULL)
  {
    printf("Loading %s\n",MaskVolFile);
    fflush(stdout);
    maskvol = MRIread(MaskVolFile);
    if (maskvol == NULL)
    {
      printf("ERROR: loading %s\n",MaskVolFile);
      exit(1);
    }
    if (maskframe >= maskvol->nframes)
    {
      printf("ERROR: mask frame = %d, mask volume only has %d frames\n",
             maskframe,maskvol->nframes);
      exit(1);
    }
    if(MRIdimMismatch(maskvol,seg,0))
    {
      printf("ERROR: dimension mismatch between brain volume and seg\n");
      exit(1);
    }
    mri_binarize(maskvol, maskthresh, masksign, maskinvert,
                 maskvol, &nmaskhits);
    if (nmaskhits == 0)
    {
      printf("WARNING: no voxels in mask meet thresholding criteria.\n");
      printf("The output table will be empty.\n");
      printf("thresh = %g, sign = %s, inv = %d\n",maskthresh,masksign,maskinvert);
      //exit(1);
    }
    printf("There were %d voxels in the orginal mask\n",nmaskhits);
    if(maskerode > 0)
    {
      printf("Eroding %d voxels in 3d\n",maskerode);
      for(n=0; n<maskerode; n++)
      {
        MRIerode(maskvol,maskvol);
      }
    }

    /* perform the masking */
    for (c=0; c < seg->width; c++)
    {
      for (r=0; r < seg->height; r++)
      {
        for (s=0; s < seg->depth; s++)
        {
          // Set voxels out of the mask to 0
          if (! (int)MRIgetVoxVal(maskvol,c,r,s,maskframe))
          {
            MRIsetVoxVal(seg,c,r,s,0,0);
          }
        }
      }
    }
  }

  if (!mris)
  {
    voxelvolume = seg->xsize * seg->ysize * seg->zsize;
    printf("Voxel Volume is %g mm^3\n",voxelvolume);
  }
  else
  {
    if (mris->group_avg_surface_area > 0)
    {
      voxelvolume = mris->group_avg_surface_area/mris->nvertices;
    }
    else
    {
      voxelvolume = mris->total_area/mris->nvertices;
    }
    printf("Vertex Area is %g mm^3\n",voxelvolume);
  }

  /* There are three ways that the list of segmentations
     can be specified:
     1. User does not specify, then get all from the seg itself
     2. User specfies with --id (can be multiple)
     3. User supplies a color table
     If the user specficies a color table and --id, then the
     segs from --id are used ang the color table is only
     used to determine the name of the segmentation.
  */

  printf("Generating list of segmentation ids\n");
  fflush(stdout);
  segidlist0 = MRIsegIdList(seg, &nsegid0,0);

  if (ctab == NULL && nUserSegIdList == 0)
  {
    /* Must get list of segmentation ids from segmentation itself*/
    segidlist = segidlist0;
    nsegid = nsegid0;
    StatSumTable = (STATSUMENTRY *) calloc(sizeof(STATSUMENTRY),nsegid);
    for (n=0; n < nsegid; n++)
    {
      StatSumTable[n].id = segidlist[n];
      strcpy(StatSumTable[n].name, "\0");
    }
  }
  else   /* Get from user or color table */
  {
    if (ctab != NULL)
    {
      if (nUserSegIdList == 0)
      {
        /* User has not spec anything, so use all the ids in the color table */
        /* We want to fill StatSumTable with all the valid entries
           from the color table. So we'll get the number of valid
           entries and create StatSumTable with that many
           elements. Then walk through the entirity of the ctab and
           skip past the invalid entries. Copy the valid entries into
           StatSumTable. We use a separate index with StatSumTable
           that only goes from 0->the number of valid entries.*/
        CTABgetNumberOfValidEntries(ctab,&nsegid);
        CTABgetNumberOfTotalEntries(ctab,&ntotalsegid);
        StatSumTable = (STATSUMENTRY *) calloc(sizeof(STATSUMENTRY),nsegid);
        usersegid=0;
        for (n=0; n < ntotalsegid; n++)
        {
          CTABisEntryValid(ctab,n,&valid);
          if(!valid)
          {
            continue;
          }
          StatSumTable[usersegid].id = n;
          CTABcopyName(ctab,n,StatSumTable[usersegid].name,
                       sizeof(StatSumTable[usersegid].name));
          CTABrgbAtIndexi(ctab, n, &StatSumTable[usersegid].red,
                          &StatSumTable[usersegid].green,
                          &StatSumTable[usersegid].blue);
          usersegid++;
        }
      }
      else
      {
        /* User has specified --id, use those and get names from ctab */
        nsegid = nUserSegIdList;
        StatSumTable = (STATSUMENTRY *) calloc(sizeof(STATSUMENTRY),nsegid);
        for (n=0; n < nsegid; n++)
        {
          StatSumTable[n].id = UserSegIdList[n];
          /* Here ind should be the same as the ctab entry, but make
             sure it's valid. */
          ind = StatSumTable[n].id;
          CTABisEntryValid(ctab,ind,&valid);
          if (!valid)
          {
            printf("ERROR: cannot find seg id %d in %s\n",
                   StatSumTable[n].id,ctabfile);
            exit(1);
          }
          CTABcopyName(ctab,ind,StatSumTable[n].name,
                       sizeof(StatSumTable[n].name));
          CTABrgbAtIndexi(ctab, ind, &StatSumTable[n].red,
                          &StatSumTable[n].green,
                          &StatSumTable[n].blue);
        }
      }
    }
    else   /* User specified ids, but no color table */
    {
      nsegid = nUserSegIdList;
      StatSumTable = (STATSUMENTRY *) calloc(sizeof(STATSUMENTRY),nsegid);
      for (n=0; n < nsegid; n++)
      {
        StatSumTable[n].id = UserSegIdList[n];
      }
    }
  }

  printf("Found %3d segmentations\n",nsegid);
  printf("Computing statistics for each segmentation\n");
  fflush(stdout);
  for (n=0; n < nsegid; n++)
  {
    if(DoExclSegId)
    {
      DoContinue = 0;
      for(nx=0; nx < nExcl; nx++)
      {
        if(StatSumTable[n].id == ExclSegIdList[nx])
        {
          DoContinue = 1;
          break;
        }
      }
      if(DoContinue)
      {
        continue;
      }
    }
    printf("%3d   %3d  %s ",n,StatSumTable[n].id,StatSumTable[n].name);
    fflush(stdout);

    // Skip ones that are not represented
    skip = 1;
    for (n0=0; n0 < nsegid0; n0++)
      if (StatSumTable[n].id == segidlist0[n0])
      {
        skip = 0;
      }
    if (skip)
    {
      printf(" 0\n");
      continue;
    }

    if (!dontrun)
    {
      if (!mris)
      {
        if (pvvol == NULL)
        {
          nhits = MRIsegCount(seg, StatSumTable[n].id, 0);
        }
        else
          nhits =
            MRIvoxelsInLabelWithPartialVolumeEffects
            (seg, pvvol, StatSumTable[n].id, NULL, NULL);
        vol = nhits*voxelvolume;
      }
      else
      {
        // Compute area here
        nhits = 0;
        vol = 0;
        for (c=0; c < mris->nvertices; c++)
        {
          if (MRIgetVoxVal(seg,c,0,0,0)==StatSumTable[n].id)
          {
            nhits++;
            if (mris->group_avg_vtxarea_loaded)
            {
              vol += mris->vertices[c].group_avg_area;
            }
            else
            {
              vol += mris->vertices[c].area;
            }
          }
        }
      }
    }
    else
    {
      nhits = n;
    }

    printf("%4d  %g\n",nhits,vol);
    fflush(stdout);
    StatSumTable[n].nhits = nhits;
    StatSumTable[n].vol = vol;
    if (InVolFile != NULL && !dontrun)
    {
      if (nhits > 0)
      {
        MRIsegStats(seg, StatSumTable[n].id, invol, frame,
                    &min, &max, &range, &mean, &std);
        snr = mean/std;
      }
      else
      {
        min=0;
        max=0;
        range=0;
        mean=0;
        std=0;
        snr=0;
      }
      StatSumTable[n].min   = min;
      StatSumTable[n].max   = max;
      StatSumTable[n].range = range;
      StatSumTable[n].mean  = mean;
      StatSumTable[n].std   = std;
      StatSumTable[n].snr   = snr;
    }
  }
  printf("\n");


  /* Remove empty segmentations, if desired */
  if (NonEmptyOnly || DoExclSegId)
  {
    // Count the number of nonempty segmentations
    nsegidrep = 0;
    for (n=0; n < nsegid; n++)
    {
      if(NonEmptyOnly && StatSumTable[n].nhits==0)
      {
        continue;
      }
      if(DoExclSegId)
      {
        DoContinue = 0;
        for(nx=0; nx < nExcl; nx++)
        {
          if(StatSumTable[n].id == ExclSegIdList[nx])
          {
            DoContinue = 1;
            break;
          }
        }
        if(DoContinue)
        {
          continue;
        }
      }
      nsegidrep ++;
    }
    StatSumTable2 = (STATSUMENTRY *) calloc(sizeof(STATSUMENTRY),nsegidrep);
    nthsegid = 0;
    for (n=0; n < nsegid; n++)
    {
      if(NonEmptyOnly && StatSumTable[n].nhits==0)
      {
        continue;
      }

      if(DoExclSegId)
      {
        DoContinue = 0;
        for(nx=0; nx < nExcl; nx++)
        {
          if(StatSumTable[n].id == ExclSegIdList[nx])
          {
            DoContinue = 1;
            break;
          }
        }
        if(DoContinue)
        {
          continue;
        }
      }

      StatSumTable2[nthsegid].id    = StatSumTable[n].id;
      StatSumTable2[nthsegid].nhits = StatSumTable[n].nhits;
      StatSumTable2[nthsegid].vol   = StatSumTable[n].vol;
      StatSumTable2[nthsegid].min   = StatSumTable[n].min;
      StatSumTable2[nthsegid].max   = StatSumTable[n].max;
      StatSumTable2[nthsegid].range = StatSumTable[n].range;
      StatSumTable2[nthsegid].mean  = StatSumTable[n].mean;
      StatSumTable2[nthsegid].std   = StatSumTable[n].std;
      StatSumTable2[nthsegid].snr   = StatSumTable[n].snr;
      StatSumTable2[nthsegid].red   = StatSumTable[n].red;
      StatSumTable2[nthsegid].green = StatSumTable[n].green;
      StatSumTable2[nthsegid].blue  = StatSumTable[n].blue;
      strcpy(StatSumTable2[nthsegid].name,StatSumTable[n].name);
      nthsegid++;
    }
    free(StatSumTable);
    StatSumTable = StatSumTable2;
    nsegid = nsegidrep;
  }
  printf("Reporting on %3d segmentations\n",nsegid);

  if(BrainVolFromSeg)
  {
    brainsegvolume2 = 0.0;
    for(n=0; n < nsegid; n++)
    {
      id = StatSumTable[n].id;
      if(!IS_BRAIN(id))
      {
        continue ;
      }
      if(IS_CSF(id) || IS_CSF_CLASS(id))
      {
        continue;
      }
      brainsegvolume2 += StatSumTable[n].vol;
    }
  }
  if(DoSubCortGrayVol)
  {
    SubCortGrayVol = 0.0;
    for(n=0; n < nsegid; n++)
    {
      id = StatSumTable[n].id;
      if(! IsSubCorticalGray(id))
      {
        continue ;
      }
      SubCortGrayVol += StatSumTable[n].vol;
    }
    printf("SubCortGrayVol = %g\n",SubCortGrayVol);
  }

  /* Dump the table to the screen */
  if (debug)
  {
    for (n=0; n < nsegid; n++)
    {
      printf("%3d  %8d %10.1f  ", StatSumTable[n].id,StatSumTable[n].nhits,
             StatSumTable[n].vol);
      if (ctab != NULL)
      {
        printf("%-30s ",StatSumTable[n].name);
      }
      if (InVolFile != NULL)
      {
        printf("%10.4f %10.4f %10.4f %10.4f %10.4f ",
               StatSumTable[n].min, StatSumTable[n].max,
               StatSumTable[n].range, StatSumTable[n].mean,
               StatSumTable[n].std);
        if(DoSNR)
        {
          printf("%10.4f ",StatSumTable[n].snr);
        }
      }
      printf("\n");
    }
  }

  /* Print the table to the output file */
  if (StatTableFile != NULL)
  {
    fp = fopen(StatTableFile,"w");
    fprintf(fp,"# Title Segmentation Statistics \n");
    fprintf(fp,"# \n");
    fprintf(fp,"# generating_program %s\n",Progname);
    fprintf(fp,"# cvs_version %s\n",vcid);
    fprintf(fp,"# cmdline %s\n",cmdline);
    fprintf(fp,"# sysname  %s\n",uts.sysname);
    fprintf(fp,"# hostname %s\n",uts.nodename);
    fprintf(fp,"# machine  %s\n",uts.machine);
    fprintf(fp,"# user     %s\n",VERuser());
    if (mris)
    {
      fprintf(fp,"# anatomy_type surface\n");
    }
    else
    {
      fprintf(fp,"# anatomy_type volume\n");
    }
    fprintf(fp,"# \n");
    if (subject != NULL)
    {
      fprintf(fp,"# SUBJECTS_DIR %s\n",SUBJECTS_DIR);
      fprintf(fp,"# subjectname %s\n",subject);
    }
    if (BrainMaskFile)
    {
      fprintf(fp,"# BrainMaskFile  %s \n",BrainMaskFile);
      fprintf(fp,"# BrainMaskFileTimeStamp  %s \n",
              VERfileTimeStamp(BrainMaskFile));
      fprintf(fp,"# Measure BrainMask, BrainMaskNVox, "
              "Number of Brain Mask Voxels, %7d, unitless\n",
              nbrainmaskvoxels);
      fprintf(fp,"# Measure BrainMask, BrainMaskVol, "
              "Brain Mask Volume, %f, mm^3\n",brainmaskvolume);
    }
    if (BrainVolFromSeg)
    {
      fprintf(fp,"# Measure BrainSegNotVent, BrainSegVolNotVent, "
              "Brain Segmentation Volume Without Ventricles, %f, mm^3\n",
              brainsegvolume2);
      fprintf(fp,"# Measure BrainSeg, BrainSegNVox, "
              "Number of Brain Segmentation Voxels, %7d, unitless\n",
              nbrainsegvoxels);
      fprintf(fp,"# Measure BrainSeg, BrainSegVol, "
              "Brain Segmentation Volume, %f, mm^3\n",
              brainsegvolume);
    }
    if(DoSurfCtxVol)
    {
      // Does this include the non-cortical areas of the surface?
      fprintf(fp,"# Measure lhCortex, lhCortexVol, "
              "Left hemisphere cortical gray matter volume, %f, mm^3\n",lhctxvol);
      fprintf(fp,"# Measure rhCortex, rhCortexVol, "
              "Right hemisphere cortical gray matter volume, %f, mm^3\n",rhctxvol);
      fprintf(fp,"# Measure Cortex, CortexVol, "
              "Total cortical gray matter volume, %f, mm^3\n",lhctxvol+rhctxvol);
    }
    if(DoSurfWMVol)
    {
      fprintf(fp,"# Measure lhCorticalWhiteMatter, lhCorticalWhiteMatterVol, "
              "Left hemisphere cortical white matter volume, %f, mm^3\n",lhwhitevol);
      fprintf(fp,"# Measure rhCorticalWhiteMatter, rhCorticalWhiteMatterVol, "
              "Right hemisphere cortical white matter volume, %f, mm^3\n",rhwhitevol);
      fprintf(fp,"# Measure CorticalWhiteMatter, CorticalWhiteMatterVol, "
              "Total cortical white matter volume, %f, mm^3\n",lhwhitevol+rhwhitevol);
    }
    if(DoSubCortGrayVol)
    {
      fprintf(fp,"# Measure SubCortGray, SubCortGrayVol, "
              "Subcortical gray matter volume, %f, mm^3\n",
              SubCortGrayVol);
    }
    if(DoTotalGrayVol)
    {
      fprintf(fp,"# Measure TotalGray, TotalGrayVol, "
              "Total gray matter volume, %f, mm^3\n",
              SubCortGrayVol+lhctxvol+rhctxvol);
    }
    if(DoSupraTent)
    {
      fprintf(fp,"# Measure SupraTentorial, SupraTentorialVol, "
              "Supratentorial volume, %f, mm^3\n",SupraTentVol);

    }

    if (DoETIV)
    {
      fprintf(fp,"# Measure IntraCranialVol, ICV, "
              "Intracranial Volume, %f, mm^3\n",atlas_icv);
    }
    if (SegVolFile)
    {
      fprintf(fp,"# SegVolFile %s \n",SegVolFile);
      fprintf(fp,"# SegVolFileTimeStamp  %s \n",VERfileTimeStamp(SegVolFile));
    }
    if (annot)
    {
      fprintf(fp,"# Annot %s %s %s\n",subject,hemi,annot);
    }
    if (LabelFile)
    {
      fprintf(fp,"# Label %s %s %s\n",subject,hemi,LabelFile);
    }
    if (ctabfile)
    {
      fprintf(fp,"# ColorTable %s \n",ctabfile);
      fprintf(fp,"# ColorTableTimeStamp %s \n",VERfileTimeStamp(ctabfile));
    }
    if (gcafile)
    {
      fprintf(fp,"# ColorTableFromGCA %s \n",gcafile);
      fprintf(fp,"# GCATimeStamp %s \n",VERfileTimeStamp(gcafile));
    }
    if (MaskVolFile)
    {
      fprintf(fp,"# MaskVolFile  %s \n",MaskVolFile);
      fprintf(fp,"#   MaskVolFileTimeStamp  %s \n",
              VERfileTimeStamp(MaskVolFile));
      fprintf(fp,"#   MaskThresh %f \n",maskthresh);
      fprintf(fp,"#   MaskSign   %s \n",masksign);
      fprintf(fp,"#   MaskFrame  %d \n",maskframe);
      fprintf(fp,"#   MaskInvert %d \n",maskframe);
    }
    if (InVolFile)
    {
      fprintf(fp,"# InVolFile  %s \n",InVolFile);
      fprintf(fp,"# InVolFileTimeStamp  %s \n",VERfileTimeStamp(InVolFile));
      fprintf(fp,"# InVolFrame %d \n",frame);
    }
    if (PVVolFile)
    {
      fprintf(fp,"# PVVolFile  %s \n",PVVolFile);
      fprintf(fp,"# PVVolFileTimeStamp  %s \n",VERfileTimeStamp(PVVolFile));
    }
    if(DoExclCtxGMWM)
    {
      fprintf(fp,"# Excluding Cortical Gray and White Matter\n");
    }
    if(DoExclSegId)
    {
      fprintf(fp,"# ExcludeSegId ");
      for(nx=0; nx < nExcl; nx++)
      {
        fprintf(fp,"%d ",ExclSegIdList[nx]);
      }
      fprintf(fp,"\n");
    }
    if(NonEmptyOnly)
    {
      fprintf(fp,"# Only reporting non-empty segmentations\n");
    }
    if(!mris)
    {
      fprintf(fp,"# VoxelVolume_mm3 %g \n",voxelvolume);
    }
    else
    {
      fprintf(fp,"# VertexArea_mm2 %g \n",voxelvolume);
    }
    fprintf(fp,"# TableCol  1 ColHeader Index \n");
    fprintf(fp,"# TableCol  1 FieldName Index \n");
    fprintf(fp,"# TableCol  1 Units     NA \n");

    fprintf(fp,"# TableCol  2 ColHeader SegId \n");
    fprintf(fp,"# TableCol  2 FieldName Segmentation Id\n");
    fprintf(fp,"# TableCol  2 Units     NA\n");
    if (!mris)
    {
      fprintf(fp,"# TableCol  3 ColHeader NVoxels \n");
      fprintf(fp,"# TableCol  3 FieldName Number of Voxels\n");
      fprintf(fp,"# TableCol  3 Units     unitless\n");
      fprintf(fp,"# TableCol  4 ColHeader Volume_mm3\n");
      fprintf(fp,"# TableCol  4 FieldName Volume\n");
      fprintf(fp,"# TableCol  4 Units     mm^3\n");
    }
    else
    {
      fprintf(fp,"# TableCol  3 ColHeader NVertices \n");
      fprintf(fp,"# TableCol  3 FieldName Number of Vertices\n");
      fprintf(fp,"# TableCol  3 Units     unitless\n");
      fprintf(fp,"# TableCol  4 ColHeader Area_mm2\n");
      fprintf(fp,"# TableCol  4 FieldName Area\n");
      fprintf(fp,"# TableCol  4 Units     mm^2\n");
    }
    n = 5;
    if (ctab)
    {
      fprintf(fp,"# TableCol %2d ColHeader StructName\n",n);
      fprintf(fp,"# TableCol %2d FieldName Structure Name\n",n);
      fprintf(fp,"# TableCol %2d Units     NA\n",n);
      n++;
    }

    if (InVolFile)
    {
      fprintf(fp,"# TableCol %2d ColHeader %sMean \n",n,InIntensityName);
      fprintf(fp,"# TableCol %2d FieldName Intensity %sMean\n",
              n,InIntensityName);
      fprintf(fp,"# TableCol %2d Units     %s\n",n,InIntensityUnits);
      n++;

      fprintf(fp,"# TableCol %2d ColHeader %sStdDev\n",n,InIntensityName);
      fprintf(fp,"# TableCol %2d FieldName Itensity %sStdDev\n",
              n,InIntensityName);
      fprintf(fp,"# TableCol %2d Units     %s\n",n,InIntensityUnits);
      n++;

      fprintf(fp,"# TableCol %2d ColHeader %sMin\n",n,InIntensityName);
      fprintf(fp,"# TableCol %2d FieldName Intensity %sMin\n",
              n,InIntensityName);
      fprintf(fp,"# TableCol %2d Units     %s\n",n,InIntensityUnits);
      n++;

      fprintf(fp,"# TableCol %2d ColHeader %sMax\n",n,InIntensityName);
      fprintf(fp,"# TableCol %2d FieldName Intensity %sMax\n",
              n,InIntensityName);
      fprintf(fp,"# TableCol %2d Units     %s\n",n,InIntensityUnits);
      n++;

      fprintf(fp,"# TableCol %2d ColHeader %sRange\n",n,InIntensityName);
      fprintf(fp,"# TableCol %2d FieldName Intensity %sRange\n",
              n,InIntensityName);
      fprintf(fp,"# TableCol %2d Units     %s\n",n,InIntensityUnits);
      n++;

    }
    fprintf(fp,"# NRows %d \n",nsegid);
    fprintf(fp,"# NTableCols %d \n",n-1);

    fprintf(fp,"# ColHeaders  Index SegId ");
    if (!mris)
    {
      fprintf(fp,"NVoxels Volume_mm3 ");
    }
    else
    {
      fprintf(fp,"NVertices Area_mm2 ");
    }
    fprintf(fp,"StructName ");
    if(InVolFile)
    {
      fprintf(fp,"%sMean %sStdDev %sMin %sMax %sRange  ",
              InIntensityName, InIntensityName,
              InIntensityName,InIntensityName,
              InIntensityName);
      if(DoSNR)
      {
        fprintf(fp,"%sSNR ",InIntensityName);
      }
    }
    fprintf(fp,"\n");

    for (n=0; n < nsegid; n++)
    {
      fprintf(fp,"%3d %3d  %8d %10.1f  ", n+1, StatSumTable[n].id,
              StatSumTable[n].nhits, StatSumTable[n].vol);
      if(ctab != NULL)
      {
        fprintf(fp,"%-30s ",StatSumTable[n].name);
      }
      else
      {
        fprintf(fp,"Seg%04d ",StatSumTable[n].id);
      }
      if (InVolFile != NULL)
      {
        fprintf(fp,"%10.4f %10.4f %10.4f %10.4f %10.4f ",
                StatSumTable[n].mean, StatSumTable[n].std,
                StatSumTable[n].min, StatSumTable[n].max,
                StatSumTable[n].range);
        if(DoSNR)
        {
          fprintf(fp,"%10.4f ",StatSumTable[n].snr);
        }
      }
      fprintf(fp,"\n");
    }
    fclose(fp);
  }

  if(ctabfileOut != NULL)
  {
    fp = fopen(ctabfileOut,"w");
    for (n=0; n < nsegid; n++)
      fprintf(fp,"%d %-30s %3d %3d %3d 0\n",StatSumTable[n].id,
              StatSumTable[n].name,StatSumTable[n].red,
              StatSumTable[n].green,StatSumTable[n].blue);
    fclose(fp);
  }

  // Average input across space to create a waveform
  // for each segmentation
  if (DoFrameAvg)
  {
    printf("Computing spatial average of each frame\n");
    favg = (double **) calloc(sizeof(double *),nsegid);
    for (n=0; n < nsegid; n++)
    {
      favg[n] = (double *) calloc(sizeof(double),invol->nframes);
    }
    favgmn = (double *) calloc(sizeof(double *),nsegid);
    for (n=0; n < nsegid; n++)
    {
      printf("%3d",n);
      if (n%20 == 19)
      {
        printf("\n");
      }
      fflush(stdout);
      MRIsegFrameAvg(seg, StatSumTable[n].id, invol, favg[n]);
      favgmn[n] = 0.0;
      for(f=0; f < invol->nframes; f++)
      {
        favgmn[n] += favg[n][f];
      }
      favgmn[n] /= invol->nframes;
    }
    printf("\n");

    // Save mean over space and frames in simple text file
    // Each seg on a separate line
    if(SpatFrameAvgFile)
    {
      printf("Writing to %s\n",SpatFrameAvgFile);
      fp = fopen(SpatFrameAvgFile,"w");
      for (n=0; n < nsegid; n++)
      {
        fprintf(fp,"%g\n",favgmn[n]);
        printf("%d %g\n",n,favgmn[n]);
      }
      fclose(fp);
    }

    // Save as a simple text file
    if(FrameAvgFile)
    {
      printf("Writing to %s\n",FrameAvgFile);
      fp = fopen(FrameAvgFile,"w");
      //fprintf(fp,"-1 -1 ");
      //for (n=0; n < nsegid; n++) fprintf(fp,"%4d ", StatSumTable[n].id);
      //fprintf(fp,"\n");
      for (f=0; f < invol->nframes; f++)
      {
        //fprintf(fp,"%3d %7.3f ",f,f*invol->tr/1000);
        for (n=0; n < nsegid; n++)
        {
          fprintf(fp,"%g ",favg[n][f]);
        }
        fprintf(fp,"\n");
      }
      fclose(fp);
    }

    // Save as an MRI "volume"
    if(FrameAvgVolFile)
    {
      printf("Writing to %s\n",FrameAvgVolFile);
      famri = MRIallocSequence(nsegid,1,1,MRI_FLOAT,invol->nframes);
      for (f=0; f < invol->nframes; f++)
      {
        for (n=0; n < nsegid; n++)
        {
          MRIsetVoxVal(famri,n,0,0,f,(float)favg[n][f]);
        }
      }
      MRIwrite(famri,FrameAvgVolFile);
    }
  }// Done with Frame Average

#ifdef FS_CUDA
  PrintGPUtimers();
#endif

  return(0);
}
/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/

/* --------------------------------------------- */
static int parse_commandline(int argc, char **argv)
{
  int  nargc , nargsused, nth;
  char **pargv, *option ;

  if (argc < 1)
  {
    usage_exit();
  }

  nargc   = argc;
  pargv = argv;
  while (nargc > 0)
  {

    option = pargv[0];
    if (debug)
    {
      printf("%d %s\n",nargc,option);
    }
    nargc -= 1;
    pargv += 1;

    nargsused = 0;

    if (!strcasecmp(option, "--help")||!strcasecmp(option, "--usage")
        ||!strcasecmp(option, "-h")||!strcasecmp(option, "-u"))
    {
      print_help() ;
    }
    else if (!strcasecmp(option, "--version"))
    {
      print_version() ;
    }
    else if (!strcasecmp(option, "--debug"))
    {
      debug = 1;
    }
    else if (!strcasecmp(option, "--dontrun"))
    {
      dontrun = 1;
    }
    else if (!strcasecmp(option, "--nonempty"))
    {
      NonEmptyOnly = 1;
    }
    else if (!strcasecmp(option, "--empty"))
    {
      NonEmptyOnly = 0;
    }
    else if ( !strcmp(option, "--brain-vol-from-seg") )
    {
      BrainVolFromSeg = 1;
    }
    else if ( !strcmp(option, "--subcortgray") )
    {
      DoSubCortGrayVol = 1;
    }
    else if ( !strcmp(option, "--supratent") )
    {
      DoSupraTent = 1;
    }
    else if ( !strcmp(option, "--totalgray") )
    {
      DoTotalGrayVol = 1;
    }
    else if ( !strcmp(option, "--etiv") )
    {
      DoETIV = 1;
    }
    else if ( !strcmp(option, "--etiv-only") )
    {
      DoETIVonly = 1;
    }
    else if ( !strcmp(option, "--old-etiv-only") )
    {
      DoOldETIVonly = 1;
    }
    else if ( !strcmp(option, "--surf-ctx-vol") )
    {
      DoSurfCtxVol = 1;
    }
    else if ( !strcmp(option, "--surf-wm-vol") )
    {
      DoSurfWMVol = 1;
    }
    else if ( !strcmp(option, "--sqr") )
    {
      DoSquare = 1;
    }
    else if ( !strcmp(option, "--sqrt") )
    {
      DoSquareRoot = 1;
    }
    else if ( !strcmp(option, "--snr") )
    {
      DoSNR = 1;
    }

    else if ( !strcmp(option, "--mul") )
    {
      if(nargc < 1)
      {
        argnerr(option,1);
      }
      DoMultiply = 1;
      sscanf(pargv[0],"%lf",&MultVal);
      nargsused = 1;
    }
    else if ( !strcmp(option, "--talxfm") )
    {
      if (nargc < 1)
      {
        argnerr(option,1);
      }
      talxfmfile = pargv[0];
      nargsused = 1;
    }
    else if ( !strcmp(option, "--sd") )
    {
      if(nargc < 1)
      {
        argnerr(option,1);
      }
      FSENVsetSUBJECTS_DIR(pargv[0]);
      nargsused = 1;
    }
    else if ( !strcmp(option, "--ctab-default") )
    {
      FREESURFER_HOME = getenv("FREESURFER_HOME");
      ctabfile = (char *) calloc(sizeof(char),1000);
      sprintf(ctabfile,"%s/FreeSurferColorLUT.txt",FREESURFER_HOME);
      printf("Using defalt ctab %s\n",ctabfile);
    }
    else if ( !strcmp(option, "--ctab-gca") )
    {
      if (nargc < 1)
      {
        argnerr(option,1);
      }
      gcafile = pargv[0];
      nargsused = 1;
    }
    else if ( !strcmp(option, "--seg") )
    {
      if (nargc < 1)
      {
        argnerr(option,1);
      }
      SegVolFile = pargv[0];
      nargsused = 1;
    }
    else if ( !strcmp(option, "--vox") )
    {
      if (nargc < 1)
      {
        argnerr(option,3);
      }
      sscanf(pargv[0],"%d",&Vox[0]);
      sscanf(pargv[1],"%d",&Vox[1]);
      sscanf(pargv[2],"%d",&Vox[2]);
      DoVox = 1;
      nargsused = 3;
    }
    else if ( !strcmp(option, "--in") || !strcmp(option, "--i") )
    {
      if (nargc < 1)
      {
        argnerr(option,1);
      }
      InVolFile = pargv[0];
      nargsused = 1;
    }
    else if(!strcmp(option, "--reg"))
    {
      if(nargc < 1)
      {
        argnerr(option,1);
      }
      InVolRegFile = pargv[0];
      InVolReg = regio_read_registermat(InVolRegFile);
      if(InVolReg == NULL)
      {
        exit(1);
      }
      nargsused = 1;
    }
    else if(!strcmp(option, "--regheader"))
    {
      InVolRegHeader = 1;
    }
    else if ( !strcmp(option, "--in-intensity-name") )
    {
      if (nargc < 1)
      {
        argnerr(option,1);
      }
      InIntensityName = pargv[0];
      nargsused = 1;
    }
    else if ( !strcmp(option, "--in-intensity-units") )
    {
      if (nargc < 1)
      {
        argnerr(option,1);
      }
      InIntensityUnits = pargv[0];
      nargsused = 1;
    }
    else if ( !strcmp(option, "--brainmask") )
    {
      if (nargc < 1)
      {
        argnerr(option,1);
      }
      BrainMaskFile = pargv[0];
      nargsused = 1;
    }
    else if ( !strcmp(option, "--id") )
    {
      if(nargc < 1)
      {
        argnerr(option,1);
      }
      nth = 0;
      while(CMDnthIsArg(nargc, pargv, nth) )
      {
        sscanf(pargv[nth],"%d",&UserSegIdList[nUserSegIdList]);
        nUserSegIdList++;
        nth++;
      }
      nargsused = nth;
    }
    else if ( !strcmp(option, "--excl-ctxgmwm") )
    {
      DoExclSegId = 1;
      DoExclCtxGMWM = 1;
      ExclSegIdList[nExcl] =  2;
      nExcl++;
      ExclSegIdList[nExcl] =  3;
      nExcl++;
      ExclSegIdList[nExcl] = 41;
      nExcl++;
      ExclSegIdList[nExcl] = 42;
      nExcl++;
    }
    else if( !strcmp(option, "--excludeid") ||
             !strcmp(option, "--exclude") )
    {
      if(nargc < 1)
      {
        argnerr(option,1);
      }
      nth = 0;
      while(CMDnthIsArg(nargc, pargv, nth) )
      {
        sscanf(pargv[nth],"%d",&ExclSegIdList[nExcl]);
        nExcl ++;
        nth ++;
      }
      DoExclSegId = 1;
      nargsused = nth;
    }
    else if ( !strcmp(option, "--mask") )
    {
      if (nargc < 1)
      {
        argnerr(option,1);
      }
      MaskVolFile = pargv[0];
      nargsused = 1;
    }
    else if (!strcmp(option, "--masksign"))
    {
      if (nargc < 1)
      {
        argnerr(option,1);
      }
      masksign = pargv[0];
      nargsused = 1;
      if (strncasecmp(masksign,"abs",3) &&
          strncasecmp(masksign,"pos",3) &&
          strncasecmp(masksign,"neg",3))
      {
        fprintf(stderr,"ERROR: mask sign = %s, must be abs, pos, or neg\n",
                masksign);
        exit(1);
      }
    }
    else if (!strcmp(option, "--maskthresh"))
    {
      if (nargc < 1)
      {
        argnerr(option,1);
      }
      sscanf(pargv[0],"%f",&maskthresh);
      nargsused = 1;
    }
    else if (!strcmp(option, "--maskframe"))
    {
      if (nargc < 1)
      {
        argnerr(option,1);
      }
      sscanf(pargv[0],"%d",&maskframe);
      nargsused = 1;
    }
    else if (!strcmp(option, "--maskerode"))
    {
      if (nargc < 1)
      {
        argnerr(option,1);
      }
      sscanf(pargv[0],"%d",&maskerode);
      nargsused = 1;
    }
    else if (!strcasecmp(option, "--maskinvert"))
    {
      maskinvert = 1;
    }

    else if ( !strcmp(option, "--sum") )
    {
      if (nargc < 1)
      {
        argnerr(option,1);
      }
      StatTableFile = pargv[0];
      nargsused = 1;
    }
    else if ( !strcmp(option, "--sum-in") )
    {
      if (nargc < 1)
      {
        argnerr(option,1);
      }
      StatTableFile = pargv[0];
      StatSumTable = LoadStatSumFile(StatTableFile,&nsegid);
      printf("Found %d\n",nsegid);
      DumpStatSumTable(StatSumTable,nsegid);
      exit(1);
      nargsused = 1;
    }
    else if ( !strcmp(option, "--avgwf") )
    {
      if (nargc < 1)
      {
        argnerr(option,1);
      }
      FrameAvgFile = pargv[0];
      DoFrameAvg = 1;
      nargsused = 1;
    }
    else if ( !strcmp(option, "--sfavg") )
    {
      if (nargc < 1)
      {
        argnerr(option,1);
      }
      SpatFrameAvgFile = pargv[0];
      DoFrameAvg = 1;
      nargsused = 1;
    }
    else if ( !strcmp(option, "--avgwfvol") )
    {
      if (nargc < 1)
      {
        argnerr(option,1);
      }
      FrameAvgVolFile = pargv[0];
      DoFrameAvg = 1;
      nargsused = 1;
    }
    else if ( !strcmp(option, "--ctab") )
    {
      if (nargc < 1)
      {
        argnerr(option,1);
      }
      ctabfile = pargv[0];
      nargsused = 1;
    }
    else if ( !strcmp(option, "--ctab-out") )
    {
      if (nargc < 1)
      {
        argnerr(option,1);
      }
      ctabfileOut = pargv[0];
      nargsused = 1;
    }
    else if (!strcmp(option, "--frame"))
    {
      if (nargc < 1)
      {
        argnerr(option,1);
      }
      sscanf(pargv[0],"%d",&frame);
      nargsused = 1;
    }
    else if (!strcmp(option, "--subject"))
    {
      if (nargc < 1)
      {
        argnerr(option,1);
      }
      subject = pargv[0];
      nargsused = 1;
    }
    else if (!strcmp(option, "--annot"))
    {
      if (nargc < 3)
      {
        argnerr(option,1);
      }
      subject = pargv[0];
      hemi    = pargv[1];
      annot   = pargv[2];
      nargsused = 3;
    }
    else if (!strcmp(option, "--slabel"))
    {
      if (nargc < 3)
      {
        argnerr(option,1);
      }
      subject = pargv[0];
      hemi    = pargv[1];
      LabelFile = pargv[2];
      ExclSegId = 0;
      DoExclSegId = 1;
      nargsused = 3;
    }
    else if (!strcmp(option, "--segbase"))
    {
      if (nargc < 1)
      {
        argnerr(option,1);
      }
      sscanf(pargv[0],"%d",&segbase);
      nargsused = 1;
    }
    else if (!strcmp(option, "--synth"))
    {
      if (nargc < 1)
      {
        argnerr(option,1);
      }
      sscanf(pargv[0],"%ld",&seed);
      synth = 1;
      nargsused = 1;
    }
    else if ( !strcmp(option, "--pv") )
    {
      if (nargc < 1)
      {
        argnerr(option,1);
      }
      PVVolFile = pargv[0];
      nargsused = 1;
    }
    else
    {
      fprintf(stderr,"ERROR: Option %s unknown\n",option);
      if (singledash(option))
      {
        fprintf(stderr,"       Did you really mean -%s ?\n",option);
      }
      exit(-1);
    }
    nargc -= nargsused;
    pargv += nargsused;
  }
  return(0);
}
/* ------------------------------------------------------ */
static void usage_exit(void)
{
  print_usage() ;
  exit(1) ;
}
/* --------------------------------------------- */
#include "mri_segstats.help.xml.h"
static void print_usage(void)
{
  outputHelpXml(mri_segstats_help_xml,
                mri_segstats_help_xml_len);
}

/* --------------------------------------------- */
static void print_help(void)
{
  print_usage() ;
  exit(1) ;
}
/* --------------------------------------------- */
static void print_version(void)
{
  printf("%s\n", vcid) ;
  exit(1) ;
}
/* --------------------------------------------- */
static void argnerr(char *option, int n)
{
  if (n==1)
  {
    fprintf(stderr,"ERROR: %s flag needs %d argument\n",option,n);
  }
  else
  {
    fprintf(stderr,"ERROR: %s flag needs %d arguments\n",option,n);
  }
  exit(-1);
}
/* --------------------------------------------- */
static void check_options(void)
{
  if (SegVolFile == NULL && annot == NULL && LabelFile == NULL &&
      DoETIVonly == 0 && DoOldETIVonly == 0)
  {
    printf("ERROR: must specify a segmentation volume\n");
    exit(1);
  }
  if (StatTableFile == NULL && FrameAvgFile == NULL && DoETIVonly == 0 && DoOldETIVonly == 0)
  {
    printf("ERROR: must specify an output table file\n");
    exit(1);
  }
  if (DoFrameAvg && InVolFile == NULL)
  {
    printf("ERROR: cannot do frame average without input volume\n");
    exit(1);
  }
  if (DoETIV && subject == NULL)
  {
    printf("ERROR: need subject with --etiv\n");
    exit(1);
  }
  if (DoSupraTent && !DoSurfCtxVol)
  {
    printf("ERROR: need --surf-ctx-vol  with --supratent\n");
    exit(1);
  }
  if (ctabfile != NULL && gcafile != NULL)
  {
    printf("ERROR: cannot specify ctab and gca\n");
    exit(1);
  }
  if(DoSurfCtxVol && subject == NULL)
  {
    printf("ERROR: need --subject with --surf-ctx-vol\n");
    exit(1);
  }
  if(DoTotalGrayVol && !DoSurfCtxVol)
  {
    printf("ERROR: need --surf-ctx-vol with --totalgray\n");
    exit(1);
  }
  if(ctabfileOut && !ctabfile)
  {
    printf("ERROR: need an input ctab to create output ctab\n");
    exit(1);
  }
  if(ctabfileOut)
  {
    if(!strcmp(ctabfileOut,ctabfile))
    {
      printf("ERROR: output ctab is the same as input\n");
      exit(1);
    }
  }
  if (masksign == NULL)
  {
    masksign = "abs";
  }
  return;
}

/* --------------------------------------------- */
static void dump_options(FILE *fp)
{
  fprintf(fp,"\n");
  fprintf(fp,"%s\n",vcid);
  fprintf(fp,"cwd %s\n",cwd);
  fprintf(fp,"cmdline %s\n",cmdline);
  fprintf(fp,"sysname  %s\n",uts.sysname);
  fprintf(fp,"hostname %s\n",uts.nodename);
  fprintf(fp,"machine  %s\n",uts.machine);
  fprintf(fp,"user     %s\n",VERuser());
  return;
}
/*---------------------------------------------------------------*/
static int singledash(char *flag)
{
  int len;
  len = strlen(flag);
  if (len < 2)
  {
    return(0);
  }

  if (flag[0] == '-' && flag[1] != '-')
  {
    return(1);
  }
  return(0);
}

/* ----------------------------------------------------------
   MRIsegCount() - returns the number of times the given
   segmentation id appears in the volume.
   --------------------------------------------------------- */
int MRIsegCount(MRI *seg, int id, int frame)
{
  int nhits, v, c,r,s;
  nhits = 0;
  for (c=0; c < seg->width; c++)
  {
    for (r=0; r < seg->height; r++)
    {
      for (s=0; s < seg->depth; s++)
      {
        v = (int) MRIgetVoxVal(seg,c,r,s,frame);
        if (v == id)
        {
          nhits ++;
        }
      }
    }
  }
  return(nhits);
}
/*------------------------------------------------------------*/
STATSUMENTRY *LoadStatSumFile(char *fname, int *nsegid)
{
  FILE *fp;
  char tmpstr[1000];
  STATSUMENTRY *StatSumTable, *e;

  fp = fopen(fname,"r");
  if (fp == NULL)
  {
    printf("ERROR: cannot open %s\n",fname);
    exit(1);
  }

  // Count the number of entries
  *nsegid = 0;
  while (fgets(tmpstr,1000,fp) != NULL)
  {
    if (tmpstr[0] == '#')
    {
      continue;
    }
    (*nsegid)++;
  }
  fclose(fp);

  StatSumTable = (STATSUMENTRY *) calloc(sizeof(STATSUMENTRY),*nsegid);

  // Now actually read it in
  fp = fopen(fname,"r");
  *nsegid = 0;
  while (fgets(tmpstr,1000,fp) != NULL)
  {
    if (tmpstr[0] == '#')
    {
      continue;
    }
    e = &StatSumTable[*nsegid];
    sscanf(tmpstr,"%*d %d %d %f %s %f %f %f %f %f",
           &e->id,&e->nhits,&e->vol,&e->name[0],
           &e->mean,&e->std,&e->min,&e->max,&e->range);
    (*nsegid)++;
  }
  fclose(fp);

  return(StatSumTable);
}
//----------------------------------------------------------------
int DumpStatSumTable(STATSUMENTRY *StatSumTable, int nsegid)
{
  int n;
  for (n=0; n < nsegid; n++)
  {
    printf("%3d  %8d %10.1f  ",
           StatSumTable[n].id,
           StatSumTable[n].nhits,
           StatSumTable[n].vol);
    printf("%-30s ",StatSumTable[n].name);
    printf("%10.4f %10.4f %10.4f %10.4f %10.4f ",
           StatSumTable[n].min, StatSumTable[n].max,
           StatSumTable[n].range, StatSumTable[n].mean,
           StatSumTable[n].std);
    printf("\n");
  }
  return(0);
}
