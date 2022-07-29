#include "Stable.h"

#include "SelectiveOpacityDockWidget.h"

QSelectiveOpacityDockWidget::QSelectiveOpacityDockWidget(QWidget* pParent) :
	QDockWidget(pParent),
	//m_MainLayout(),
	m_SelectiveOpacityWidget()
{
	QSelectiveOpacityWidget *gSelectiveOpacityWidget = new QSelectiveOpacityWidget();

	//m_MainLayout.setAlignment(Qt::AlignTop);
	//setLayout(&m_MainLayout);

	// Window title and tooltip
	setWindowTitle("Selective Opacity");
	setToolTip("<img src=':/Images/application-tree.png'><div>Selective Opacity</div>");
	setWindowIcon(GetIcon("application-tree"));

	setWidget(&m_SelectiveOpacityWidget);//, 0, 0);

}