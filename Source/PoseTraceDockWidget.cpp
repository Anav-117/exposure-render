#include "Stable.h"

#include "PoseTraceDockWidget.h"

QPoseTraceDockWidget::QPoseTraceDockWidget(QWidget* pParent) :
	QDockWidget(pParent),
	m_PoseTraceWidget()
{
	setWindowTitle("PoseTrace");
	setToolTip("<img src=':/Images/light-bulb.png'><div>Mesh</div>");
	setWindowIcon(GetIcon("light-bulb"));

	setWidget(&m_PoseTraceWidget);

}

QPoseTraceDockWidget::~QPoseTraceDockWidget() {}