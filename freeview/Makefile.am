if ENABLE_QT_APPS

include $(top_srcdir)/build-aux/autotroll.mk

bin_PROGRAMS = freeview

BUILT_SOURCES = \
	ui_MainWindow.h \
	ui_PanelVolume.h \
	ui_PanelSurface.h \
	ui_PanelROI.h \
	ui_PanelPointSet.h \
	ui_DialogLoadVolume.h \
	ui_DialogPreferences.h \
	ui_ToolWindowMeasure.h \
	ui_ToolWindowEdit.h \
	ui_DialogNewVolume.h \
	ui_DialogNewROI.h \
	ui_DialogNewPointSet.h \
	ui_ToolWindowROIEdit.h \
	ui_DialogLoadPointSet.h \
	ui_DialogLoadDTI.h \
	ui_WindowConfigureOverlay.h \
	ui_DialogTransformVolume.h \
	ui_DialogCropVolume.h \
	ui_DialogSaveScreenshot.h \
	ui_DialogVolumeFilter.h \
	ui_DialogAbout.h \
	ui_DialogWriteMovieFrames.h \
	ui_DialogGradientFilter.h \
	ui_WindowQuickReference.h \
	ui_FloatingStatusBar.h \
	ui_TermWidget.h \
	ui_PanelTrack.h \
	ui_DialogSavePointSet.h \
	ui_DialogSaveVolume.h \
  ui_DialogReplaceLabel.h \
	qtcolorpicker.moc \
	moc_BrushProperty.cpp \
	moc_CommandEdit.cpp \
	moc_Contour2D.cpp \
	moc_Cursor2D.cpp \
	moc_Cursor3D.cpp \
	moc_DialogAbout.cpp \
	moc_DialogCropVolume.cpp \
	moc_DialogGradientFilter.cpp \
	moc_DialogLoadDTI.cpp \
	moc_DialogLoadPointSet.cpp \
	moc_DialogLoadVolume.cpp \
	moc_DialogPreferences.cpp \
	moc_DialogNewPointSet.cpp \
	moc_DialogNewROI.cpp \
	moc_DialogNewVolume.cpp \
	moc_DialogSavePointSet.cpp \
	moc_DialogSaveScreenshot.cpp \
	moc_DialogTransformVolume.cpp \
	moc_DialogVolumeFilter.cpp \
	moc_DialogWriteMovieFrames.cpp \
	moc_FloatingStatusBar.cpp \
	moc_FSLabel.cpp \
	moc_FSSurface.cpp \
	moc_FSVolume.cpp \
	moc_GenericRenderView.cpp \
	moc_InfoTreeWidget.cpp \
	moc_Interactor.cpp \
  moc_Interactor2D.cpp \
  moc_Interactor2DNavigate.cpp \
	moc_Interactor2DMeasure.cpp \
	moc_Interactor2DVolumeCrop.cpp \
	moc_Interactor2DVolumeEdit.cpp \
	moc_Interactor3D.cpp \
	moc_Interactor3DMeasure.cpp \
	moc_Layer.cpp \
	moc_LayerCollection.cpp \
	moc_LayerDTI.cpp \
	moc_LayerEditable.cpp \
	moc_LayerPLabel.cpp \
	moc_LayerPointSet.cpp \
	moc_LayerProperty.cpp \
	moc_LayerPropertyDTI.cpp \
	moc_LayerPropertyMRI.cpp \
	moc_LayerPropertyPointSet.cpp \
	moc_LayerPropertyROI.cpp \
	moc_LayerPropertySurface.cpp \
	moc_LayerROI.cpp \
	moc_LayerSurface.cpp \
	moc_LayerTreeWidget.cpp \
	moc_LayerVolumeBase.cpp \
	moc_LayerMRI.cpp \
	moc_MainWindow.cpp \
	moc_PanelLayer.cpp \
	moc_PanelPointSet.cpp \
  moc_PanelROI.cpp \
	moc_PanelSurface.cpp \
	moc_PanelVolume.cpp \
	moc_qtcolorpicker.cpp \
	moc_QVTKWidget.cpp \
	moc_Region2D.cpp \
	moc_RenderView.cpp \
	moc_RenderView2D.cpp \
	moc_RenderView3D.cpp \
	moc_SurfaceLabel.cpp \
	moc_SurfaceOverlay.cpp \
	moc_SurfaceOverlayProperty.cpp \
	moc_SurfaceRegion.cpp \
	moc_SurfaceRegionGroups.cpp \
	moc_TermWidget.cpp \
	moc_ThreadBuildContour.cpp \
	moc_ThreadIOWorker.cpp \
	moc_ToolWindowEdit.cpp \
	moc_ToolWindowMeasure.cpp \
	moc_ToolWindowROIEdit.cpp \
	moc_VolumeCropper.cpp \
	moc_VolumeFilter.cpp \
	moc_WidgetHistogram.cpp \
	moc_WindowConfigureOverlay.cpp \
	moc_WindowQuickReference.cpp \
	moc_FSTrack.cpp \
	moc_TrackData.cpp \
	moc_LayerTrack.cpp \
	moc_TrackGroup.cpp \
	moc_PanelTrack.cpp \
	moc_LayerPropertyTrack.cpp \
  moc_DialogSaveVolume.cpp \
  moc_DialogReplaceLabel.cpp \
  moc_LayerVolumeTrack.cpp \
  moc_LayerLandmarks.cpp \
  qrc_freeview.cpp

freeview_SOURCES = $(BUILT_SOURCES) \
	Annotation2D.cpp \
	CommandEdit.cpp \
	Contour2D.cpp \
	Cursor2D.cpp \
	Cursor3D.cpp \
	CursorFactory.cpp \
	BrushProperty.cpp	 \
	DialogAbout.cpp \
	DialogCropVolume.cpp \
	DialogGradientFilter.cpp \
	DialogLoadDTI.cpp \
	DialogLoadPointSet.cpp \
	DialogLoadVolume.cpp \
	DialogNewROI.cpp \
	DialogNewPointSet.cpp \
	DialogNewVolume.cpp \
	DialogPreferences.cpp \
	DialogSavePointSet.cpp \
	DialogSaveScreenshot.cpp \
	DialogTransformVolume.cpp \
	DialogVolumeFilter.cpp \
	DialogWriteMovieFrames.cpp \
	FloatingStatusBar.cpp \
	FSLabel.cpp\
	FSPointSet.cpp \
	FSSurface.cpp \
	FSVolume.cpp \
	GenericRenderView.cpp \
	InfoTreeWidget.cpp \
	Interactor.cpp \
	Interactor2D.cpp \
	Interactor2DMeasure.cpp \
	Interactor2DNavigate.cpp \
	Interactor2DPointSetEdit.cpp \
	Interactor2DROIEdit.cpp \
	Interactor2DVolumeCrop.cpp \
	Interactor2DVolumeEdit.cpp \
	Interactor2DVoxelEdit.cpp \
	Interactor3D.cpp \
	Interactor3DMeasure.cpp \
	Interactor3DNavigate.cpp \
	Interactor3DVolumeCrop.cpp \
	Layer.cpp \
	LayerCollection.cpp \
	LayerDTI.cpp \
	LayerEditable.cpp \
	LayerMRI.cpp \
	LayerPLabel.cpp \
	LayerPointSet.cpp \
	LayerProperty.cpp \
	LayerPropertyDTI.cpp \
	LayerPropertyMRI.cpp \
	LayerPropertyPointSet.cpp \
	LayerPropertyROI.cpp \
	LayerPropertySurface.cpp \
	LayerROI.cpp \
	LayerSurface.cpp \
	LayerTreeWidget.cpp \
	LayerVolumeBase.cpp \
	LivewireTool.cpp \
	LUTDataHolder.cpp \
	main.cpp \
	MyCmdLineParser.cpp \
	MyUtils.cpp \
	MyVTKUtils.cpp \
	PanelLayer.cpp \
	PanelPointSet.cpp \
	PanelROI.cpp \
	PanelSurface.cpp \
	PanelVolume.cpp \
	qtcolorpicker.cpp \
	QVTKWidget.cxx \
	QVTKWidget.h \
	QVTKPaintEngine.cxx \
	QVTKPaintEngine.h \
	QVTKWin32Header.h \
	Region2D.cpp \
	Region2DLine.cpp \
	Region2DPolyline.cpp \
	Region2DRectangle.cpp \
	RenderView.cpp \
	RenderView2D.cpp \
	RenderView3D.cpp \
	SurfaceAnnotation.cpp \
	SurfaceLabel.cpp \
	SurfaceOverlay.cpp \
	SurfaceOverlayProperty.cpp \
	SurfaceRegion.cpp \
	SurfaceRegionGroups.cpp \
	TermWidget.cpp \
	ThreadBuildContour.cpp \
	ThreadIOWorker.cpp \
	ToolWindowEdit.cpp \
	ToolWindowMeasure.cpp \
	ToolWindowROIEdit.cpp \
	UIUpdateHelper.cpp \
	VolumeCropper.cpp \
	VolumeFilter.cpp \
	VolumeFilter.h \
	VolumeFilterConvolve.cpp \
	VolumeFilterConvolve.h \
	VolumeFilterGradient.cpp \
	VolumeFilterGradient.h \
	VolumeFilterMean.cpp \
	VolumeFilterMean.h \
	VolumeFilterMedian.cpp \
	VolumeFilterMedian.h \
	VolumeFilterSobel.cpp \
	VolumeFilterSobel.h \
	vtkSimpleLabelEdgeFilter.cpp \
	WidgetHistogram.cpp \
	WindowConfigureOverlay.cpp \
	WindowQuickReference.cpp \
	FSTrack.cpp \
	track_io/TrackIO.cpp \
	track_io/TrackIO.h \
	track_io/ByteSwap.h \
	track_io/ErrorCode.h \
	TrackData.cpp \
	Track.cpp \
	Track.h \
	LayerTrack.cpp \
	TrackGroup.cpp \
	PanelTrack.cpp \
	LayerPropertyTrack.cpp \
	DialogSaveVolume.cpp \
	MainWindow.cpp \
	Annotation2D.h \
	BrushProperty.h \
	CommandEdit.h \
	CommonDataStruct.h \
	Contour2D.h \
	Cursor2D.h \
	Cursor3D.h \
	CursorFactory.h \
	DialogAbout.h \
	DialogCropVolume.h \
	DialogGradientFilter.h \
	DialogLoadDTI.h \
	DialogLoadPointSet.h \
	DialogLoadVolume.h \
	DialogPreferences.h \
	DialogNewPointSet.h \
	DialogNewROI.h \
	DialogNewVolume.h \
	DialogSavePointSet.h \
	DialogSaveScreenshot.h \
	DialogTransformVolume.h \
	DialogVolumeFilter.h \
	DialogWriteMovieFrames.h \
	FloatingStatusBar.h \
  FSLabel.h \
	FSPointSet.h \
	FSSurface.h \
	FSVolume.h \
	GenericRenderView.h \
	InfoTreeWidget.h \
	Interactor.h \
	Interactor2D.h \
	Interactor2DMeasure.h \
	Interactor2DNavigate.h \
	Interactor2DPointSetEdit.h \
	Interactor2DROIEdit.h \
	Interactor2DVolumeCrop.h \
	Interactor2DVolumeEdit.h \
	Interactor2DVoxelEdit.h \
	Interactor3D.h \
	Interactor3DMeasure.h \
	Interactor3DNavigate.h \
	Interactor3DVolumeCrop.h \
	Layer.h \
	LayerCollection.h \
	LayerDTI.h \
	LayerEditable.h \
	LayerPLabel.h \
	LayerPointSet.h \
	LayerProperty.h \
	LayerPropertyDTI.h \
	LayerPropertyMRI.h \
	LayerPropertyPointSet.h \
	LayerPropertyROI.h \
	LayerPropertySurface.h \
	StockColorMap.h \
	LayerROI.h \
	LayerSurface.h \
	LayerTreeWidget.h \
	LayerVolumeBase.h \
	LayerMRI.h \
	LivewireTool.h \
	LUTDataHolder.h \
	MainWindow.h \
	MyCmdLineParser.h \
	MyUtils.h \
	MyVTKUtils.h \
	PanelLayer.h \
	PanelPointSet.h \
	PanelROI.h \
	PanelSurface.h \
	PanelVolume.h \
	qtcolorpicker.h \
	Region2D.h \
	Region2DLine.h \
	Region2DPolyline.h \
	Region2DRectangle.h \
	RenderView.h \
	RenderView2D.h \
	RenderView3D.h \
	SurfaceLabel.h \
	SurfaceAnnotation.h \
	SurfaceOverlay.h \
	SurfaceOverlayProperty.h \
	SurfaceRegion.h \
	SurfaceRegionGroups.h \
	TermWidget.h \
	ThreadBuildContour.h \
	ThreadIOWorker.h \
	ToolWindowEdit.h \
	ToolWindowMeasure.h \
	ToolWindowROIEdit.h \
	UIUpdateHelper.h \
	VolumeCropper.h \
	VolumeFilter.h \
	VolumeFilterSobel.h \
	vtkSimpleLabelEdgeFilter.h \
	WidgetHistogram.h \
	WindowConfigureOverlay.h \
	WindowQuickReference.h \
	FSTrack.h \
	TrackData.h \
	LayerTrack.h \
	TrackGroup.h \
	PanelTrack.h \
	LayerPropertyTrack.h \
  DialogSaveVolume.h \
  DialogReplaceLabel.cpp \
  DialogReplaceLabel.h \
  LayerVolumeTrack.cpp \
  LayerVolumeTrack.h \
  LayerLandmarks.cpp \
  LayerLandmarks.h

if HAVE_MAC_OSX
AM_CXXFLAGS=\
	-fno-strict-aliasing \
	-Wno-deprecated \
	-Wno-unused \
	-Wno-uninitialized \
	-Wno-return-type \
	-Wno-reorder \
	-Wno-sign-compare \
	-I$(top_srcdir)/include \
	$(GL_CFLAGS) \
	$(VTK_COCOA_CXXFLAGS) \
	-I$(top_srcdir)/vtkfsio \
	-I$(top_srcdir)/vtkutils \
	$(ITK_CFLAGS) -DHAVE_ITK_LIBS \
	$(VXL_CFLAGS)


freeview_CXXFLAGS = $(QT_CXXFLAGS) $(AM_CXXFLAGS)
freeview_CPPFLAGS = $(QT_CPPFLAGS) $(AM_CPPFLAGS)
freeview_LDFLAGS	= $(QT_LDFLAGS) $(OS_LDFLAGS) \
	-framework Cocoa -framework IOKit
freeview_LDADD		= $(QT_LIBS)	\
	$(addprefix $(top_builddir)/, $(LIBS_MGH)) \
	$(top_builddir)/vtkfsio/libvtkfsio.a \
	$(top_builddir)/vtkutils/libvtkutils.a \
	$(VTK_COCOA_LIBS) $(VTK_VERDICT_LIB) -lvtkGraphics \
	$(VTK_EXPAT_LIB) $(VTK_FREETYPE_LIB) \
	$(VTK_TIFF_LIB) $(VTK_JPEG_LIB) \
	$(VTK_METAIO_LIB) $(VTK_PNG_LIB) $(VTK_Z_LIB) \
	$(VTK_SQLITE_LIB) \
	-lvtkImaging -lvtkFiltering \
	-lvtkCommon -lvtksys -lvtkGenericFiltering \
	-lvtkexoIIc -lvtkNetCDF \
	-lvtkVolumeRendering -lvtkRendering -lvtkftgl \
	-lvtkWidgets -lvtkHybrid \
	-lvtkIO	-lvtkDICOMParser $(VTK_MPEG2ENCODE_LIB)


# need to create a bundle for Macs
install-exec-hook:freeview
	mkdir -p $(DESTDIR)$(bindir)/Freeview.app
	mkdir -p $(DESTDIR)$(bindir)/Freeview.app/Contents
	mkdir -p $(DESTDIR)$(bindir)/Freeview.app/Contents/MacOS
	mkdir -p $(DESTDIR)$(bindir)/Freeview.app/Contents/Resources
	mkdir -p $(DESTDIR)$(bindir)/Freeview.app/Contents/Resources/English.lproj
#	mkdir -p $(DESTDIR)$(bindir)/Freeview.app/Contents/Frameworks
	cp $(top_builddir)/freeview/Info.plist $(DESTDIR)$(bindir)/Freeview.app/Contents
	echo -n 'APPL????' > $(DESTDIR)$(bindir)/Freeview.app/Contents/PkgInfo
	cp $(top_builddir)/freeview/freeview $(DESTDIR)$(bindir)/Freeview.app/Contents/MacOS/freeview.bin
	if [[ -d  /usr/pubsw/packages/qt/current/lib/qt_menu.nib ]]; then \
	cp -rv /usr/pubsw/packages/qt/current/lib/qt_menu.nib $(DESTDIR)$(bindir)/Freeview.app/Contents/Resources; fi
	echo "#!/bin/tcsh -ef" > $(DESTDIR)$(bindir)/freeview
#	echo "source \$$FREESURFER_HOME/bin/tcl_setup" \
		>> $(DESTDIR)$(bindir)/freeview
	echo "source \$$FREESURFER_HOME/bin/qt_setup" \
		>> $(DESTDIR)$(bindir)/freeview
	echo "exec \$$FREESURFER_HOME/bin/Freeview.app/Contents/MacOS/freeview.bin \$$*" >> $(DESTDIR)$(bindir)/freeview
	chmod a+x $(DESTDIR)$(bindir)/freeview
#	./mac_package_dylibs.sh $(WXWIDGETS_DIR)lib $(DESTDIR)$(bindir)/Freeview.app/Contents/Frameworks $(DESTDIR)$(bindir)/Freeview.app/Contents/MacOS/freeview.bin
	if [[ -f /usr/pubsw/bin/chrpath ]]; then \
	/usr/pubsw/bin/chrpath -d $(DESTDIR)$(bindir)/Freeview.app/Contents/MacOS/freeview.bin; fi

uninstall-hook:
	rm -f $(DESTDIR)$(bindir)/freeview
	rm -rf $(DESTDIR)$(bindir)/Freeview.app

else

AM_CXXFLAGS=\
	-fno-strict-aliasing \
	-Wno-deprecated \
	-Wno-unused \
	-Wno-uninitialized \
	-Wno-return-type \
	-Wno-reorder \
	-Wno-sign-compare \
	-I$(top_srcdir)/include \
	$(GL_CFLAGS) \
	$(VTK_CXXFLAGS) \
	-I$(top_srcdir)/vtkfsio \
	-I$(top_srcdir)/vtkutils \
	$(ITK_CFLAGS) -DHAVE_ITK_LIBS \
	$(VXL_CFLAGS)

freeview_CXXFLAGS = $(QT_CXXFLAGS) $(AM_CXXFLAGS)
freeview_CPPFLAGS = $(QT_CPPFLAGS) $(AM_CPPFLAGS)
freeview_LDFLAGS	= $(QT_LDFLAGS) $(OS_LDFLAGS)
freeview_LDADD		= $(QT_LIBS)	\
	$(addprefix $(top_builddir)/, $(LIBS_MGH)) \
	$(top_builddir)/vtkfsio/libvtkfsio.a \
	$(top_builddir)/vtkutils/libvtkutils.a \
	$(VTK_LIBS) $(VTK_VERDICT_LIB) -lvtkGraphics \
	$(VTK_EXPAT_LIB) $(VTK_FREETYPE_LIB) \
	$(VTK_TIFF_LIB) $(VTK_JPEG_LIB) \
	$(VTK_METAIO_LIB) $(VTK_PNG_LIB) $(VTK_Z_LIB) \
	$(VTK_SQLITE_LIB) \
	-lvtkImaging -lvtkFiltering \
	-lvtkCommon -lvtksys -lvtkGenericFiltering \
	-lvtkexoIIc -lvtkNetCDF \
	-lvtkVolumeRendering -lvtkRendering -lvtkftgl \
	-lvtkWidgets -lvtkHybrid \
	-lvtkIO	-lvtkDICOMParser $(VTK_MPEG2ENCODE_LIB)

# put a wrapper around the bin, used to setup tcltktixblt,vtk enviro vars
# and if the OS is MACOSX have a different install-hook to create a bundle
install-exec-hook:freeview
	cp $(top_builddir)/freeview/freeview $(DESTDIR)$(bindir)/freeview.bin
	echo "#!/bin/tcsh -ef" > $(DESTDIR)$(bindir)/freeview
	echo "source \$$FREESURFER_HOME/bin/tcl_setup" \
		>> $(DESTDIR)$(bindir)/freeview
	echo "source \$$FREESURFER_HOME/bin/vtk_setup" \
		>> $(DESTDIR)$(bindir)/freeview
	echo "source \$$FREESURFER_HOME/bin/qt_setup" \
		>> $(DESTDIR)$(bindir)/freeview
	echo "freeview.bin \$$argv" >> $(DESTDIR)$(bindir)/freeview
	chmod a+x $(DESTDIR)$(bindir)/freeview
	if [[ -f /usr/pubsw/bin/chrpath ]]; then \
	/usr/pubsw/bin/chrpath -d $(DESTDIR)$(bindir)/freeview.bin; fi

uninstall-hook:
	rm -f $(DESTDIR)$(bindir)/freeview.bin
endif

clean-local:
	rm -f $(BUILT_SOURCES)

EXTRA_DIST=$(freeview_SOURCES)

# Our release target. Include files to be excluded here. They will be
# found and removed after 'make install' is run during the 'make
# release' target.
EXCLUDE_FILES=
include $(top_srcdir)/Makefile.extra

endif 
