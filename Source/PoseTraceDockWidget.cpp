#include "Stable.h"

#include "PoseTraceDockWidget.h"

QPoseTraceDockWidget::QPoseTraceDockWidget(QWidget* pParent) :
	QDockWidget(pParent),
	m_PoseTraceWidget()
{
	setWindowTitle("PoseTrace");
	setToolTip("<img src=':/Images/camera--arrow.png'><div>PoseTrace</div>");
	setWindowIcon(GetIcon("camera--arrow"));

	setWidget(&m_PoseTraceWidget);
}