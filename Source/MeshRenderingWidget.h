#pragma once

#include "Stable.h"
#include <QVTKOpenGLNativeWidget.h>
#include "MeshRendering.h"
#include "MouseInteractorHighLightActor.h"
#include <QKeyEvent>

class QMeshRenderingWidget : public QVTKOpenGLNativeWidget
{
	Q_OBJECT

public:
    QMeshRenderingWidget(QWidget* pParent = NULL);
	void SetupRenderer();
	int Selected;

public slots:
	void OnScalarRangeChanged(void);
	void OnDelete(void);
	void OnReset(void);

private:
  vtkSmartPointer<vtkInteractorStyleTrackballCamera>  ClassPicker;
  QGridLayout	m_Layout;
  QPushButton	m_DeleteButton;
  QPushButton	m_ResetButton;
};