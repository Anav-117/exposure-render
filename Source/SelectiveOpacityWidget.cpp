#include "Stable.h"
#include <string>
#include "SelectiveOpacityWidget.h"
#include "Scene.h"

#define NUM_SEGMENTS 3

QSelectiveOpacityWidget::QSelectiveOpacityWidget(QWidget* pParent) :
	QGroupBox(pParent),
	m_MainLayout(),
    m_Tree()
{
    m_MainLayout.setAlignment(Qt::AlignBottom);
	setLayout(&m_MainLayout);

    QTreeWidgetItem *MajorClass = new QTreeWidgetItem();
    MajorClass->setText(0, tr("Major Class"));
    QTreeWidgetItem *SubClass = new QTreeWidgetItem(MajorClass);
    SubClass->setText(0, tr("SubClass1"));
    SubClass->setText(1, tr("SubClass2"));
    MajorClass->addChild(SubClass);

    m_Tree.addTopLevelItem(MajorClass);

    
    m_MainLayout.addWidget(&m_Tree, 3, 0);

}

QSelectiveOpacityWidget::~QSelectiveOpacityWidget() {}

void QSelectiveOpacityWidget::OnRenderBegin(void)
{

}