#include "Stable.h"

#include "SelectiveOpacityDockWidget.h"

QSelectiveOpacityDockWidget::QSelectiveOpacityDockWidget(QWidget* pParent) :
	QDockWidget(pParent),
	//m_MainLayout(),
	m_SelectiveOpacityWidget()
{
	//m_MainLayout.setAlignment(Qt::AlignTop);
	//setLayout(&m_MainLayout);

	// Window title and tooltip
	setWindowTitle("Selective Opacity");
	setToolTip("<img src=':/Images/light-bulb.png'><div>Lighting Properties</div>");
	setWindowIcon(GetIcon("light-bulb"));

	setWidget(&m_SelectiveOpacityWidget);//, 0, 0);

}

QSelectiveOpacityDockWidget::~QSelectiveOpacityDockWidget() {}