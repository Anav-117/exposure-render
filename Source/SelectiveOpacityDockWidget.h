#pragma once

#include "LightingWidget.h"

class QSelectiveOpacityDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    QSelectiveOpacityDockWidget(QWidget *parent = NULL);
    virtual ~QSelectiveOpacityDockWidget();

private:
	QLightingWidget		m_LightingWidget;
};