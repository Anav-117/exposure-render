#include "Stable.h"
#include <QVTKOpenGLNativeWidget.h>
#include "MeshRendering.h"
#include "MouseInteractorHighLightActor.h"

#pragma once

class QMeshRenderingWidget : public QVTKOpenGLNativeWidget
{
	Q_OBJECT

public:
    QMeshRenderingWidget(QWidget* pParent = NULL);
	void SetupRenderer();
	virtual ~QMeshRenderingWidget();

public slots:
	void OnRenderBegin(void);
	void OnScalarRangeChanged(void);

private:
  vtkSmartPointer<vtkInteractorStyleTrackballCamera>  ClassPicker;
};