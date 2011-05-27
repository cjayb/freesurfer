#! /bin/tcsh -ef

#
# nu_correct_one.csh
#
# REPLACE_WITH_ONE_LINE_SHORT_DESCRIPTION
#
# Original Author: REPLACE_WITH_FULL_NAME_OF_CREATING_AUTHOR
# CVS Revision Info:
#    $Author: nicks $
#    $Date: 2011/03/02 20:16:39 $
#    $Revision: 1.3 $
#
# Copyright © 2011 The General Hospital Corporation (Boston, MA) "MGH"
#
# Terms and conditions for use, reproduction, distribution and contribution
# are found in the 'FreeSurfer Software License Agreement' contained
# in the file 'LICENSE' found in the FreeSurfer distribution, and here:
#
# https://surfer.nmr.mgh.harvard.edu/fswiki/FreeSurferSoftwareLicense
#
# Reporting: freesurfer@nmr.mgh.harvard.edu
#
#


set s=$1
set iter=3

set mdir=$SUBJECTS_DIR/$s/mri
if (-e $mdir/orig == 0) then
		set ORIG = orig.mgz
else
		set ORIG = orig
endif

mri_convert $mdir/$ORIG /tmp/nu$$0.mnc
nu_correct -stop .0001 -iterations $iter -normalize_field -clobber /tmp/nu$$0.mnc /tmp/nu$$1.mnc
mkdir -p $mdir/nu
mri_convert /tmp/nu$$1.mnc $mdir/nu
rm /tmp/nu$$0.mnc /tmp/nu$$1.mnc
find  /tmp  -prune -name "*.imp" -user $LOGNAME -exec rm -f {} \;
#rm -f /tmp/*.imp


