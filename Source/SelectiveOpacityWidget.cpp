#include "Stable.h"
#include <string>
#include "SelectiveOpacityWidget.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkNamedColors.h>
#include <vtkActor.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <algorithm>

#define NUM_SEGMENTS 3

vector<int> SubTest{32, 48, 56, 64, 80};

QSelectiveOpacityWidget::QSelectiveOpacityWidget(QWidget* pParent) :
	QGroupBox(pParent),
	m_MainLayout(),
    m_OpacitySlider(),
    m_OpacitySpinnerWidget(),
    m_RenderWindow(),
    m_Tree(), 
    m_Button()
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
    MinorClassSize = 0;
    int CurrentMinorClass = 1;
    int NewMinorClass = 0;
    SubClassSize = 0;

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
    
    SubClassVector.push_back(temp2);
    temp2.clear();
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
    

    QObject::connect(&m_Tree, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(OnSelection(QTreeWidgetItem*, int)));
    
    MajorClassSize = MajorClassVector.size();
    MajorClass = new QTreeWidgetItem[MajorClassSize];
    MinorClass = new QTreeWidgetItem[MinorClassSize];
    SubClass = new QTreeWidgetItem[SubClassSize];
    int testIndex=0;
    int MinorClassIndex=0;
    int SubClassIndex=0;
    for (int i=0; i<MajorClassVector.size(); i++) {
        LookUp.push_back({0, -1, 1});
        MajorClass[i].setText(0, tr(((string)MajorClassVector[i]).c_str()));
        for (int j=0; j<MinorClassVector[i].size(); j++) {
            MinorClass[j+MinorClassIndex].setText(0, tr(((string)MinorClassVector[i][j]).c_str()));
            if (MinorClassVector[i][j].length() > 0) {
                LookUp.push_back({1, -1, 1});
                for (int k=0; k<SubClassVector[j+MinorClassIndex].size(); k++) {
                    SubClass[k+SubClassIndex].setText(0, tr(((string)SubClassVector[j+MinorClassIndex][k]).c_str()));
                    //std::cout<<(string)SubClassVector[j+MinorClassIndex][k];
                    if (SubClassVector[j+MinorClassIndex][k].length() > 0) {
                        MinorClass[j+MinorClassIndex].addChild((SubClass + k + SubClassIndex));
                        LookUp.push_back({2, SubTest[k+SubClassIndex], 1});
                        OpacityArray.push_back({SubTest[k+SubClassIndex], 1});
                    }
                }
                SubClassIndex+=SubClassVector[j+MinorClassIndex].size();
                if (MinorClassVector[i][j].length() > 0) {
                    MajorClass[i].addChild((MinorClass + j + MinorClassIndex));
                }
            }
        }
        MinorClassIndex+=MinorClassVector[i].size();
        m_Tree.addTopLevelItem((MajorClass+i));
    }

    m_MainLayout.addWidget(&m_Tree, 0, 0);

    m_MainLayout.addWidget(new QLabel("Opacity"), 1, 0);

	m_OpacitySlider.setOrientation(Qt::Horizontal);
	m_OpacitySlider.setRange(0.0, 1.0);
	m_OpacitySlider.setValue(0.01);
	m_MainLayout.addWidget(&m_OpacitySlider, 2, 0);

	m_OpacitySpinnerWidget.setRange(0.0, 1.0);
	m_OpacitySpinnerWidget.setDecimals(3);
	m_MainLayout.addWidget(&m_OpacitySpinnerWidget, 3, 0);

    QObject::connect(&m_OpacitySlider, SIGNAL(valueChanged(double)), &m_OpacitySpinnerWidget, SLOT(setValue(double)));
	QObject::connect(&m_OpacitySpinnerWidget, SIGNAL(valueChanged(double)), &m_OpacitySlider, SLOT(setValue(double)));
	QObject::connect(&m_OpacitySlider, SIGNAL(valueChanged(double)), this, SLOT(OnSetOpacity(double)));

    m_Button.setText(QString::fromStdString("Refresh"));
    m_MainLayout.addWidget(&m_Button, 4, 0);

    QObject::connect(&m_Button, SIGNAL(clicked()), this, SLOT(OnButtonClick()));    

    //ResetTex();

    // vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();

    // vtkSmartPointer<vtkNamedColors> colors = vtkNamedColors::New();

    // vtkSmartPointer<vtkSphereSource> sphereSource = vtkSphereSource::New();

    // vtkSmartPointer<vtkPolyDataMapper> sphereMapper = vtkPolyDataMapper::New();
    // sphereMapper->SetInputConnection(sphereSource->GetOutputPort());

    // vtkSmartPointer<vtkActor> sphereActor = vtkActor::New();
    // sphereActor->SetMapper(sphereMapper);
    // //sphereActor->GetProperty()->SetColor(colors->GetColor4d("Tomato").GetData());

    // vtkSmartPointer<vtkRenderer> renderer = vtkRenderer::New();
    // renderer->AddActor(sphereActor);
    // renderer->SetBackground(colors->GetColor3d("SteelBlue").GetData());

    // //m_RenderWindow.SetRenderWindow(renderWindow);


    // //vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    // //renderWindow->AddRenderer(renderer);
    // m_RenderWindow.SetRenderWindow(renderWindow); 
    // m_RenderWindow.resize(600, 600);

    // m_RenderWindow.GetRenderWindow()->AddRenderer(renderer);
    // //m_RenderWindow.show();

    // //m_RenderWindow.show();
    // //renderWindow->Render ();

    // m_MainLayout.addWidget(&m_RenderWindow, 4, 0);


    max = OpacityArray[0][1];
    for(int i=1; i<OpacityArray.size(); i++) {
        if (OpacityArray[i][0] > max) {
            max = OpacityArray[i][0];
        }
    }

    Buffer = new float[max];

}

QSelectiveOpacityWidget::~QSelectiveOpacityWidget() {}

bool operator==(QTreeWidgetItem A, QTreeWidgetItem* B) {
    return A.text(0) == B->text(0);
}

void QSelectiveOpacityWidget::OnRenderBegin(void)
{
    //m_RenderWindow.show();
    //ResetTex();
}

void QSelectiveOpacityWidget::OnMajorChanged(int index) {
    MajorChanged = true;
    SubChanged = MinorChanged = false;
    Index = index;
}

void QSelectiveOpacityWidget::OnMinorChanged(int index) {
    MinorChanged = true;
    MajorChanged = SubChanged = false;
    Index = index;
}

void QSelectiveOpacityWidget::OnSubChanged(int index) {
    SubChanged = true;
    MajorChanged = MinorChanged = false;
    Index = index;
    m_OpacitySpinnerWidget.setValue(OpacityArray[Index][1]);
    //std::cout<<"VALUE "<<OpacityArray[Index][1]<<"\n";
}

void QSelectiveOpacityWidget::OnSelection(QTreeWidgetItem* Item, int col) {
    bool found = false;

    for (int i=0; i<MajorClassSize; i++) {
        if (MajorClass[i] == Item) {
            OnMajorChanged(i);
            found = true;
            break;
        }
    }

    if (!found) {
        for (int i=0; i<MinorClassSize; i++) {
            if (MinorClass[i] == Item) {
                OnMinorChanged(i);
                found = true;
                break;
            }
        }
    }

    if (!found) {
        for (int i=0; i<SubClassSize; i++) {
            if (SubClass[i] == Item) {
                OnSubChanged(i);
                break;
            }
        }
    }
}

void QSelectiveOpacityWidget::OnButtonClick() {
    if (SubChanged) {
        OpacityArray[Index][1] = (int)m_OpacitySpinnerWidget.value();
        //std::cout<<"CHANGED "<<OpacityArray[Index][1]<<"\n";
        ResetTex();
    }
}

void QSelectiveOpacityWidget::OnSetOpacity(double Opacity) {
    
}

void QSelectiveOpacityWidget::ResetTex() {
    for (int i = 0; i < max; i++) {
        Buffer[i] = 0.0f;
    }

    for (int i = 0; i<OpacityArray.size(); i++) {
        Buffer[OpacityArray[i][0]-1] = OpacityArray[i][1];
    }

    gSelectiveOpacity.SetOpacityBuffer(Buffer);

    // for (int i = 0; i < max; i++) {
    //     if (Buffer[i] > 0)
    //         std::cout<<Buffer[i]<<"\n";
    // }

    //BindTextureSelectiveOpacity(Buffer, max);
}
