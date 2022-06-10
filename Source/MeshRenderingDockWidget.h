#pragma once

#include "MeshRenderingWidget.h"

class QMeshRenderingDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    QMeshRenderingDockWidget(QWidget *parent = NULL);
    virtual ~QMeshRenderingDockWidget();

private:
	QMeshRenderingWidget    m_MeshRenderingWidget;
};