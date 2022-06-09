#pragma once

#include "SelectiveOpacityWidget.h"
//#include "MeshRenderWidget.h"

class QSelectiveOpacityDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    QSelectiveOpacityDockWidget(QWidget *parent = NULL);
    virtual ~QSelectiveOpacityDockWidget();

private:
	QSelectiveOpacityWidget      m_SelectiveOpacityWidget;
    //QMeshRenderWidget           m_MeshRenderWidget;
    //QGridLayout                 m_MainLayout;
};