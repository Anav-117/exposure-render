#include "Stable.h"
#include <QVTKOpenGLNativeWidget.h>
#include "MeshRendering.h"
#include "MouseInteractorHighLightActor.h"
#include <QKeyEvent>

#pragma once

class QMeshRenderingWidget : public QVTKOpenGLNativeWidget
{
	Q_OBJECT

public:
    QMeshRenderingWidget(QWidget* pParent = NULL);
	void SetupRenderer();
	virtual ~QMeshRenderingWidget();
	int Selected;

public slots:
	void OnRenderBegin(void);
	void OnScalarRangeChanged(void);
	void OnDelete(void);
	void OnReset(void);

private:
  vtkSmartPointer<vtkInteractorStyleTrackballCamera>  ClassPicker;
  QGridLayout	m_Layout;
  QPushButton	m_DeleteButton;
  QPushButton	m_ResetButton;
};