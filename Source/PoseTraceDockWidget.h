#pragma once

#include "PoseTraceWidget.h"

class QPoseTraceDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    QPoseTraceDockWidget(QWidget *parent = NULL);

private:
	QPoseTraceWidget    m_PoseTraceWidget;
};