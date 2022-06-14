#include <fstream>
#include "SelectiveOpacity.h"
#include "MeshRenderingWidget.h"

#pragma once

class QSelectiveOpacityWidget : public QGroupBox
{
	Q_OBJECT

public:
    QSelectiveOpacityWidget(QWidget* pParent = NULL);
    virtual ~QSelectiveOpacityWidget(void);
	void ResetTex();

public slots:
	void OnRenderBegin(void);
	void OnSetOpacity(double Opacity);
	void OnMajorChanged(int);
	void OnMinorChanged(int);
	void OnSubChanged(int);
	void OnSelection(QTreeWidgetItem* Item, int col);
	void OnButtonClick();
    
private:
	QGridLayout		    	m_MainLayout;
	QTreeWidget				m_Tree;
	QDoubleSlider			m_OpacitySlider;
	QDoubleSpinner			m_OpacitySpinnerWidget;
	QPushButton				m_Button;
	QMeshRenderingWidget	m_RenderWindow;

	fstream				File;
	bool				SubChanged = false;
	bool				MajorChanged = false;
	bool				MinorChanged = false;
	int					Index;
	vector<vector<int>> LookUp;
    vector<vector<float>> OpacityArray;
	QTreeWidgetItem* 	MajorClass;
	QTreeWidgetItem* 	MinorClass;
	QTreeWidgetItem* 	SubClass;
	int					MajorClassSize;
	int					MinorClassSize;
	int					SubClassSize;
	float*				Buffer;
	int 				max;
	vector<int>			Segments;
};