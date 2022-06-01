#include "Stable.h"

#include "SelectiveOpacityDockWidget.h"

QSelectiveOpacityDockWidget::QSelectiveOpacityDockWidget(QWidget* pParent) :
	QDockWidget(pParent)
	//m_LightingWidget()
{
	// Window title and tooltip
	setWindowTitle("Selective Opacity");
	setToolTip("<img src=':/Images/light-bulb.png'><div>Lighting Properties</div>");
	setWindowIcon(GetIcon("light-bulb"));

	// Apply widget
	//setWidget(&m_LightingWidget);
}

QSelectiveOpacityDockWidget::~QSelectiveOpacityDockWidget() {}