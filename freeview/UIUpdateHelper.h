/**
 * @file  UIUpdateHelper.h
 * @brief REPLACE_WITH_ONE_LINE_SHORT_DESCRIPTION
 *
 */
/*
 * Original Author: Ruopeng Wang
 * CVS Revision Info:
 *    $Author: nicks $
 *    $Date: 2011/03/14 23:44:48 $
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
 */
#ifndef UIUPDATEHELPER_H
#define UIUPDATEHELPER_H

#include <QList>

class QWidget;
class QSpinBox;
class QDoubleSpinBox;
class QLineEdit;

class UIUpdateHelper
{
public:
  UIUpdateHelper();

  void ShowWidgets( const QList<QWidget*>& widgets, bool bShow );
  void EnableWidgets( const QList<QWidget*>& widgets, bool bEnable );

  void ChangeLineEditText( QLineEdit* w, const QString& strg );
  void ChangeLineEditNumber( QLineEdit* w, double val );
  void ChangeSpinBoxValue( QSpinBox* w, int nVal );
  void ChangeDoubleSpinBoxValue( QDoubleSpinBox* w, double dVal );
  void BlockAllSignals(QWidget* w, bool bBlock);
};

#endif // UIUPDATEHELPER_H
