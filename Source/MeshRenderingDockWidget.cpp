#include "Stable.h"

#include "MeshRenderingDockWidget.h"

QMeshRenderingDockWidget::QMeshRenderingDockWidget(QWidget* pParent) :
	QDockWidget(pParent),
	m_MeshRenderingWidget()
{
	setWindowTitle("Mesh");
	setToolTip("<img src=':/Images/light-bulb.png'><div>Mesh</div>");
	setWindowIcon(GetIcon("light-bulb"));

	setWidget(&m_MeshRenderingWidget);

}

QMeshRenderingDockWidget::~QMeshRenderingDockWidget() {}