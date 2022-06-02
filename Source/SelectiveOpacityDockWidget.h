#pragma once

#include "SelectiveOpacityWidget.h"

class QSelectiveOpacityDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    QSelectiveOpacityDockWidget(QWidget *parent = NULL);
    virtual ~QSelectiveOpacityDockWidget();

private:
	QSelectiveOpacityWidget		m_SelectiveOpacityWidget;
};