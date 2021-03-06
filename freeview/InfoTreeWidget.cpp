/**
 * @file  InfoTreeWidget.cpp
 * @brief REPLACE_WITH_ONE_LINE_SHORT_DESCRIPTION
 *
 */
/*
 * Original Author: Ruopeng Wang
 * CVS Revision Info:
 *    $Author: rpwang $
 *    $Date: 2011/05/13 15:04:32 $
 *    $Revision: 1.4.2.3 $
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
#include "InfoTreeWidget.h"
#include "MainWindow.h"
#include "LayerCollection.h"
#include "LayerMRI.h"
#include "LayerPLabel.h"
#include "LayerPropertyMRI.h"
#include "LayerPropertySurface.h"
#include "LayerSurface.h"
#include "SurfaceOverlay.h"
#include "SurfaceAnnotation.h"
#include "MyUtils.h"
#include "LayerProperty.h"
#include <QTreeWidgetItem>
#include <QLineEdit>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QShowEvent>
#include <QDebug>

InfoTreeWidget::InfoTreeWidget(QWidget* parent) :
  QTreeWidget(parent)
{
  this->setAlternatingRowColors(true);
  m_editor = new QLineEdit(this);
  m_editor->hide();
  connect(this, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
          this, SLOT(OnItemClicked(QTreeWidgetItem*,int)), Qt::QueuedConnection);
  connect(m_editor, SIGNAL(returnPressed()), this, SLOT(OnEditFinished()));
}

void InfoTreeWidget::OnCursorPositionChanged()
{
  MainWindow::GetMainWindow()->GetLayerCollection("MRI")->GetSlicePosition(m_dRAS);
  UpdateAll();
}

void InfoTreeWidget::OnMousePositionChanged()
{
  MainWindow::GetMainWindow()->GetLayerCollection("MRI")->GetCurrentRASPosition(m_dRAS);
  UpdateAll();
}

void InfoTreeWidget::showEvent(QShowEvent * e)
{
  // hack to fix a qdesigner bug
  headerItem()->setText(1,"");
  QTreeWidget::showEvent(e);
}

void InfoTreeWidget::UpdateAll()
{
  this->clear();
  m_editor->hide();
  LayerCollection* lc_mri = MainWindow::GetMainWindow()->GetLayerCollection( "MRI" );
  LayerCollection* lc_surf = MainWindow::GetMainWindow()->GetLayerCollection( "Surface" );

  if ( lc_mri->IsEmpty() && lc_surf->IsEmpty())
  {
    return;
  }

  QTreeWidgetItem* item = new QTreeWidgetItem(this);
  item->setText(0, "RAS");

  double ras[3] = {m_dRAS[0], m_dRAS[1], m_dRAS[2]};
  if (!lc_mri->IsEmpty())
  {
    qobject_cast<LayerMRI*>(lc_mri->GetLayer(0))->RemapPositionToRealRAS(m_dRAS, ras);
  }
  QVariantMap map;
  item->setText(1, QString("%1, %2, %3")
                .arg(ras[0], 0, 'f', 2)
                .arg(ras[1], 0, 'f', 2)
                .arg(ras[2], 0, 'f', 2));
  map["Type"] = "RAS";
  map["EditableText"] = item->text(1);
  item->setData(1, Qt::UserRole, map);

  for (int i = 0; i < lc_mri->GetNumberOfLayers(); i++)
  {
    LayerMRI* layer = (LayerMRI*)lc_mri->GetLayer(i);
    int nIndex[3];
    if ( layer->GetProperty()->GetShowInfo() )
    {
      QTreeWidgetItem* item = new QTreeWidgetItem(this);
      item->setText(0, layer->GetName());
      layer->RASToOriginalIndex( ras, nIndex );
      double dvalue = layer->GetVoxelValue( m_dRAS );
      QString editable = QString("%1, %2, %3").arg(nIndex[0]).arg(nIndex[1]).arg(nIndex[2]);
      QString strg = QString("%1 \t[%2]").arg(dvalue).arg(editable);
      QString labelStrg;
      if (layer->IsTypeOf("PLabel"))
      {
        labelStrg = ((LayerPLabel*)layer)->GetLabelName(m_dRAS);
      }
      else
      {
        labelStrg = layer->GetLabelName( dvalue );
      }
      if (!labelStrg.isEmpty())
      {
        strg += "  " + labelStrg;
      }
      item->setText(1, strg);
      map.clear();
      map["Type"] = "MRI";
      map["EditableText"] = editable;
      map["Object"] = QVariant::fromValue((QObject*)layer);
      item->setData(1, Qt::UserRole, map);
    }
  }

  for (int i = 0; i < lc_surf->GetNumberOfLayers(); i++)
  {
    LayerSurface* surf = (LayerSurface*)lc_surf->GetLayer(i);
    if ( surf->GetProperty()->GetShowInfo() )
    {
      QTreeWidgetItem* item = new QTreeWidgetItem(this);
      item->setText(0, surf->GetName());
      double sf_pos[3];
      surf->GetSurfaceRASAtTarget( m_dRAS, sf_pos );
      QString editable = QString("%1, %2, %3")
                         .arg(sf_pos[0], 0, 'f', 2)
                         .arg(sf_pos[1], 0, 'f', 2)
                         .arg(sf_pos[2], 0, 'f', 2);
      item->setText(1, QString("SurfaceRAS\t[%1]").arg(editable));
      map.clear();
      map["Type"] = "SurfaceRAS";
      map["EditableText"] = editable;
      map["Object"] = QVariant::fromValue((QObject*)surf);
      item->setData(1, Qt::UserRole, map);

      int nVertex = surf->GetVertexIndexAtTarget( m_dRAS, NULL );
      if ( nVertex >= 0 )
      {
        surf->GetSurfaceRASAtVertex( nVertex, sf_pos );
        QTreeWidgetItem* item = new QTreeWidgetItem(this);
        item->setText(1, QString("Vertex \t%1  [%2, %3, %4]")
                      .arg(nVertex)
                      .arg(sf_pos[0], 0, 'f', 2)
                      .arg(sf_pos[1], 0, 'f', 2)
                      .arg(sf_pos[2], 0, 'f', 2));
        map.clear();
        map["Type"] = "SurfaceVertex";
        map["EditableText"] = QString::number(nVertex);
        map["Object"] = QVariant::fromValue((QObject*)surf);
        item->setData(1, Qt::UserRole, map);

        double vec[3];
        surf->GetNormalAtVertex( nVertex, vec );
        item = new QTreeWidgetItem(this);
        item->setText(1, QString("Normal \t[%1, %2, %3]")
                      .arg(vec[0], 0, 'f', 2)
                      .arg(vec[1], 0, 'f', 2)
                      .arg(vec[2], 0, 'f', 2));

        if ( surf->GetActiveVector() >= 0 )
        {
          surf->GetVectorAtVertex( nVertex, vec );
          item = new QTreeWidgetItem(this);
          item->setText(1, QString("Vector \t[%1, %2, %3]")
                        .arg(vec[0], 0, 'f', 2)
                        .arg(vec[1], 0, 'f', 2)
                        .arg(vec[2], 0, 'f', 2));
        }

        if ( surf->HasCurvature() )
        {
          item = new QTreeWidgetItem(this);
          item->setText(1, QString("Curvature \t%1").arg(surf->GetCurvatureValue(nVertex)));
        }


        int nOverlays = surf->GetNumberOfOverlays();
        for ( int i = 0; i < nOverlays; i++ )
        {
          SurfaceOverlay* overlay = surf->GetOverlay( i );
          item = new QTreeWidgetItem(this);
          item->setText(1, QString("%1 \t%2").arg(overlay->GetName()).arg(overlay->GetDataAtVertex( nVertex )));
        }

        int nAnnotations = surf->GetNumberOfAnnotations();
        for ( int i = 0; i < nAnnotations; i++ )
        {
          SurfaceAnnotation* annot = surf->GetAnnotation( i );
          item = new QTreeWidgetItem(this);
          item->setText(1, QString("%1 \t%2").arg(annot->GetName()).arg(annot->GetAnnotationNameAtVertex( nVertex )));
        }
      }
    }
  }
}

void InfoTreeWidget::OnItemClicked(QTreeWidgetItem *item, int column)
{
  if ( !item || item != m_itemEdited)
  {
    m_editor->hide();
    m_itemEdited = item;
    return;
  }
  QVariantMap map = item->data(column, Qt::UserRole).toMap();
  if (map.contains("EditableText"))
  {
    m_editor->setText(map["EditableText"].toString());
    QRect rect = this->visualRect(indexFromItem(item, column));
    m_editor->show();
    m_editor->move(rect.topLeft() + QPoint(0,header()->height()));
    m_editor->resize(rect.size());
    m_itemEdited = item;
  }
  else
  {
    m_editor->hide();
  }
}

void InfoTreeWidget::OnEditFinished()
{
  if (!m_itemEdited)
  {
    return;
  }

  QStringList list = m_editor->text().trimmed().split(",", QString::SkipEmptyParts);
  QVariantMap map = m_itemEdited->data(1, Qt::UserRole).toMap();
  if ( list.size() < 3)
  {
    list = m_editor->text().trimmed().split(" ", QString::SkipEmptyParts);
  }
  QString type = map["Type"].toString();
  double ras[3];
  bool bSuccess = false;
  QObject* layer = map["Object"].value<QObject*>();
  if ( type == "SurfaceVertex")
  {
    bool bOK;
    int nVertex = list[0].toInt(&bOK);
    if (bOK && qobject_cast<LayerSurface*>(layer)->GetTargetAtVertex(nVertex, ras))
    {
      bSuccess = true;
    }
    else
    {
      std::cerr << "Error: Invalid input";
    }
  }
  else
  {
    if ( list.size() < 3 )
    {
      std::cerr << "Error: Need to enter 3 numbers.";
    }
    else
    {
      bool bOK;
      ras[0] = list[0].toDouble(&bOK);
      ras[1] = list[1].toDouble(&bOK);
      ras[2] = list[2].toDouble(&bOK);
      if (bOK)
      {
        if (type == "RAS")
        {
          LayerMRI* mri = (LayerMRI*)MainWindow::GetMainWindow()->GetLayerCollection("MRI")->GetLayer( 0 );
          if ( mri )
          {
            mri->RASToTarget( ras, ras );
          }
        }
        else if (type == "MRI")
        {
          LayerMRI* mri = qobject_cast<LayerMRI*>(layer);
          int nv[3] = {(int)ras[0], (int)ras[1], (int)ras[2]};
          mri->OriginalIndexToRAS( nv, ras );
          mri->RASToTarget( ras, ras );
        }
        else if (type == "SurfaceRAS")
        {
          qobject_cast<LayerSurface*>(layer)->GetTargetAtSurfaceRAS( ras, ras );
        }
        bSuccess = true;
      }
      else
      {
        std::cerr << "Error: Invalid input";
      }
    }
  }
  if (bSuccess)
  {
    m_editor->hide();
    emit RASChangeTriggered(ras[0], ras[1], ras[2]);
  }
}

void InfoTreeWidget::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Escape)
  {
    m_editor->hide();
  }

  QTreeWidget::keyPressEvent(event);
}

void InfoTreeWidget::mousePressEvent(QMouseEvent *event)
{
  if (!itemAt(event->pos()))
  {
    m_editor->hide();
  }

  QTreeWidget::mousePressEvent(event);
}

void InfoTreeWidget::UpdateTrackVolumeAnnotation(Layer *layer, const QVariantMap &info)
{
  for (int i = 0; i < this->topLevelItemCount(); i++)
  {
    QTreeWidgetItem* item = this->topLevelItem(i);
    if (item)
    {
      QVariantMap map = item->data(1, Qt::UserRole).toMap();
      if (map.contains("Object") && map["Object"].value<QObject*>() == layer)
      {
        item->setText(1, QString("%1 \t%2").arg(info["label"].toInt()).arg(info["name"].toString()));
      }
    }
  }
}
