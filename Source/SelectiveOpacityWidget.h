#include <fstream>
//#include <sstream>
#include <QVTKOpenGLNativeWidget.h>

#pragma once

class QSelectiveOpacityWidget : public QGroupBox
{
	Q_OBJECT

public:
    QSelectiveOpacityWidget(QWidget* pParent = NULL);
    virtual ~QSelectiveOpacityWidget(void);

public slots:
	void OnRenderBegin(void);
    
private:
	QGridLayout		    	m_MainLayout;
	QTreeWidget				m_Tree;
	QDoubleSlider			m_OpacitySlider;
	QDoubleSpinner			m_OpacitySpinner;
	QVTKOpenGLNativeWidget	m_RenderWindow;

	fstream				File;
};