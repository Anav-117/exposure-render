#pragma once

#include "PoseTraceWidget.h"

class QPoseTraceDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    QPoseTraceDockWidget(QWidget *parent = NULL);
    virtual ~QPoseTraceDockWidget();

private:
	QPoseTraceWidget    m_PoseTraceWidget;
};