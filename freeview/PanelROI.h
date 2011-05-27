/**
 * @file  PanelROI.h
 * @brief REPLACE_WITH_ONE_LINE_SHORT_DESCRIPTION
 *
 */
/*
 * Original Author: Ruopeng Wang
 * CVS Revision Info:
 *    $Author: nicks $
 *    $Date: 2011/03/14 23:44:48 $
 *    $Revision: 1.13 $
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
#ifndef PANELROI_H
#define PANELROI_H

#include "PanelLayer.h"

namespace Ui
{
class PanelROI;
}

class PanelROI : public PanelLayer
{
  Q_OBJECT

public:
  explicit PanelROI(QWidget *parent = 0);
  ~PanelROI();

protected:
  void DoUpdateWidgets();
  void DoIdle();
  virtual void ConnectLayer( Layer* layer );

protected slots:
  void OnSliderOpacity( int val );

private:
  Ui::PanelROI *ui;
};

#endif // PANELROI_H
