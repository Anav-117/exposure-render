#pragma once

#include <fstream>
#include "SelectiveOpacity.h"
#include "MeshRenderingWidget.h"

class QSelectiveOpacityWidget : public QGroupBox
{
	Q_OBJECT

public:
    QSelectiveOpacityWidget(QWidget* pParent = NULL);
	void ResetTex();
	void UpdateCheckBox(QTreeWidgetItem* Item);
	void OnMajorChanged(int);
	void OnMinorChanged(int);
	void OnSubChanged(int);

public slots:
	void OnMajorClassChanged();
	void OnRenderBegin(void);
	void OnSelection(QTreeWidgetItem* Item, int col);
	void OnButtonClick();
	void OnCheckUpdated();
    
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

signals:
	void CheckUpdated(void);	
};