#include "Stable.h"
#include <string>
#include "SelectiveOpacityWidget.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>

#define NUM_SEGMENTS 3

QSelectiveOpacityWidget::QSelectiveOpacityWidget(QWidget* pParent) :
	QGroupBox(pParent),
	m_MainLayout(),
    m_OpacitySlider(),
    m_OpacitySpinner(),
    m_RenderWindow(),
    m_Tree()
{
    m_MainLayout.setAlignment(Qt::AlignBottom);
	setLayout(&m_MainLayout);

    vector<string> MajorClassVector;
    vector<vector<string>> MinorClassVector;
    vector<vector<string>> SubClassVector;
    vector<string> temp;
    vector<string> temp2;

    File.open("./OutlinedStructure.csv", ios::in);
    string line;

    int CurrentMajorClass = 1;
    int NewMajorClass = 1;
    int MinorClassSize = 0;
    int CurrentMinorClass = 1;
    int NewMinorClass = 0;
    int SubClassSize = 0;

    while(getline(File, line)) {
        //getline(File, line);
        
        //stringstream s(line);

        size_t firstindex = line.find_first_of(",");
        string w = line.substr(0, firstindex);

        if (!std::count(MajorClassVector.begin(), MajorClassVector.end(), w)) {
            MajorClassVector.push_back(w);
            NewMajorClass = MajorClassVector.size();
        }

        if (CurrentMajorClass == NewMajorClass) {
            size_t lastindex = line.find_last_of(",");
            string w = line.substr(firstindex+1, lastindex-firstindex-1);
            if (!std::count(temp.begin(), temp.end(), w)) {
                temp.push_back(w);
                NewMinorClass++;
                MinorClassSize++;
            }
        }
        else {
            MinorClassVector.push_back(temp);
            temp.clear();
            CurrentMajorClass = NewMajorClass;
            size_t lastindex = line.find_last_of(",");
            string w = line.substr(firstindex+1, lastindex-firstindex-1);
            temp.push_back(w);
            MinorClassSize++;
            NewMinorClass = 0;
            CurrentMinorClass = 1;
        }

        if (CurrentMinorClass == NewMinorClass) {
            size_t lastindex = line.find_last_of(",");
            string w = line.substr(lastindex+1, line.length());
            temp2.push_back(w);
            SubClassSize++;
        }
        else {
            SubClassVector.push_back(temp2);
            temp2.clear();
            CurrentMinorClass = NewMinorClass;
            size_t lastindex = line.find_last_of(",");
            string w = line.substr(lastindex+1, line.length());
            temp2.push_back(w);
            SubClassSize++;
        }
    }

    MinorClassVector.push_back(temp);
    temp.clear();

    /*TODO
    
        Create a parallel 2D array with all the segment values for each label 
        (major, minor and sub included), Opacity values, and the type of the label (major, minor or sub).
        This array will be sampled to set the opacity. Values can be retrieved 
        using the tree defined below.
        Index for any organ would be defined as TreeIndexof(Major) + TreeIndexof(Minor) + TreeIndexof(Sub).
        Since the array division would be irregular, the third field will help in properly indexing the array.
        Array will be stored as - 
        Major 1
        Minor 1.1
        Sub   1.1.1
        Sub   1.1.2
        .
        .
        Sub   1.1.m
        Minor 1.2
        .
        .
        Minor 1.n

    */

    QTreeWidgetItem *MajorClass = new QTreeWidgetItem[MajorClassVector.size()];
    QTreeWidgetItem *MinorClass = new QTreeWidgetItem[MinorClassSize];
    QTreeWidgetItem *SubClass = new QTreeWidgetItem[SubClassSize];
    int testIndex=0;
    int MinorClassIndex=0;
    int SubClassIndex=0;
    for (int i=0; i<MajorClassVector.size(); i++) {
        MajorClass[i].setText(0, tr(((string)MajorClassVector[i]).c_str()));
        for (int j=0; j<MinorClassVector[i].size(); j++) {
            MinorClass[j+MinorClassIndex].setText(0, tr(((string)MinorClassVector[i][j]).c_str()));
            for (int k=0; k<SubClassVector[j+MinorClassIndex].size(); k++) {
                SubClass[k+SubClassIndex].setText(0, tr(((string)SubClassVector[j+MinorClassIndex][k]).c_str()));
                //std::cout<<(string)SubClassVector[j+MinorClassIndex][k];
                MinorClass[j+MinorClassIndex].addChild((SubClass + k + SubClassIndex));
            }
            SubClassIndex+=SubClassVector[j+MinorClassIndex].size();
            MajorClass[i].addChild((MinorClass + j + MinorClassIndex));
        }
        MinorClassIndex+=MinorClassVector[i].size();
        m_Tree.addTopLevelItem((MajorClass+i));
    }

    //m_Tree.setColumnCount(2);
    //MajorClass->setText(0, tr("Major Class"));
    //QTreeWidgetItem *SubClass = new QTreeWidgetItem(MajorClass);
    //SubClass->setText(0, tr("SubClass1"));
    //SubClass->setText(1, tr("SubClass2"));
    //MajorClass->addChild(SubClass);

    m_MainLayout.addWidget(&m_Tree, 0, 0);

    m_MainLayout.addWidget(new QLabel("Opacity"), 1, 0);

	m_OpacitySlider.setOrientation(Qt::Horizontal);
	m_OpacitySlider.setRange(0.001, 100.0);
	m_OpacitySlider.setValue(1.0);
	m_MainLayout.addWidget(&m_OpacitySlider, 2, 0);

	m_OpacitySpinner.setRange(0.001, 100.0);
	m_OpacitySpinner.setDecimals(3);
	m_MainLayout.addWidget(&m_OpacitySpinner, 3, 0);

    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();

    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(renderer);
    m_RenderWindow.SetRenderWindow(renderWindow); 

    //m_RenderWindow.show();
    //renderWindow->Render ();

    m_MainLayout.addWidget(&m_RenderWindow, 4, 0);
}

QSelectiveOpacityWidget::~QSelectiveOpacityWidget() {}

void QSelectiveOpacityWidget::OnRenderBegin(void)
{

}