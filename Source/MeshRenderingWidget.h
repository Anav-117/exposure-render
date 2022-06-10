#include "Stable.h"
#include <QVTKOpenGLNativeWidget.h>

#pragma once

class QMeshRenderingWidget : public QVTKOpenGLNativeWidget
{
	Q_OBJECT

public:
    QMeshRenderingWidget(QWidget* pParent = NULL);
	virtual ~QMeshRenderingWidget();

public slots:
	void OnRenderBegin(void);
	
};