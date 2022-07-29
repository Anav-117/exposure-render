#include "Stable.h"

#include "MeshRenderingDockWidget.h"

QMeshRenderingDockWidget::QMeshRenderingDockWidget(QWidget* pParent) :
	QDockWidget(pParent),
	m_MeshRenderingWidget()
{
	setWindowTitle("Mesh");
	setToolTip("<img src=':/Images/dummy.png'><div>Mesh View</div>");
	setWindowIcon(GetIcon("dummy"));

	setWidget(&m_MeshRenderingWidget);
}