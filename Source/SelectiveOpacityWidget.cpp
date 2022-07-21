#include "Stable.h"
#include <string>
#include "SelectiveOpacityWidget.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include "MeshRendering.h"

#define NUM_SEGMENTS 3

QSelectiveOpacityWidget::QSelectiveOpacityWidget(QWidget* pParent) :
	QGroupBox(pParent),
	m_MainLayout(),
    m_OpacitySlider(),
    m_OpacitySpinnerWidget(),
    m_RenderWindow(),
    m_Tree(), 
    m_Button()
{
    m_MainLayout.setAlignment(Qt::AlignTop);
	setLayout(&m_MainLayout);

    vector<string> MajorClassVector;
    vector<vector<string>> MinorClassVector;
    vector<vector<string>> SubClassVector;
    vector<string> temp;
    vector<string> temp2;

    File.open("./OutlinedStructure.csv", ios::in);
    string rawline;
    string line;

    int CurrentMajorClass = 1;
    int NewMajorClass = 1;
    MinorClassSize = 0;
    int CurrentMinorClass = 1;
    int NewMinorClass = 0;
    SubClassSize = 0;

    while(getline(File, rawline)) {
        //getline(File, line);
        
        //stringstream s(line);
        size_t last = rawline.find_last_of(",");
        line = rawline.substr(0, last);
        string segmentNum = rawline.substr(last+1, rawline.length());
        Segments.push_back(stoi(segmentNum));

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

    QObject::connect(&m_Tree, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(OnSelection(QTreeWidgetItem*, int)));
    
    MajorClassSize = MajorClassVector.size();
    MajorClass = new QTreeWidgetItem[MajorClassSize];
    MinorClass = new QTreeWidgetItem[MinorClassSize];
    SubClass = new QTreeWidgetItem[SubClassSize];
    int testIndex=0;
    int MinorClassIndex=0;
    int SubClassIndex=0;
    for (int i=0; i<MajorClassVector.size(); i++) {
        //LookUp.push_back({0, -1, 1});
        MajorClass[i].setText(0, tr(((string)MajorClassVector[i]).c_str()));
        MajorClass[i].setCheckState(0, Qt::Checked);
        for (int j=0; j<MinorClassVector[i].size(); j++) {
            MinorClass[j+MinorClassIndex].setText(0, tr(((string)MinorClassVector[i][j]).c_str()));
            MinorClass[j+MinorClassIndex].setCheckState(0, Qt::Checked);
            if (MinorClassVector[i][j].length() > 0) {
                //LookUp.push_back({1, -1, 1});
                for (int k=0; k<SubClassVector[j+MinorClassIndex].size(); k++) {
                    SubClass[k+SubClassIndex].setText(0, tr(((string)SubClassVector[j+MinorClassIndex][k]).c_str()));
                    SubClass[k+SubClassIndex].setCheckState(0, Qt::Checked);
                    //std::cout<<(string)SubClassVector[j+MinorClassIndex][k];
                    if (SubClassVector[j+MinorClassIndex][k].length() > 0) {
                        MinorClass[j+MinorClassIndex].addChild((SubClass + k + SubClassIndex));
                        //LookUp.push_back({2, Segments[k+SubClassIndex], 1});
                        OpacityArray.push_back({Segments[k+SubClassIndex], 1});
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

    QLabel* label = new QLabel("Opacity");

	m_OpacitySlider.setOrientation(Qt::Horizontal);
	m_OpacitySlider.setRange(0.0, 1.0);
	m_OpacitySlider.setValue(0.01);

	m_OpacitySpinnerWidget.setRange(0.0, 1.0);
	m_OpacitySpinnerWidget.setDecimals(3);

    QObject::connect(&m_OpacitySlider, SIGNAL(valueChanged(double)), &m_OpacitySpinnerWidget, SLOT(setValue(double)));
	QObject::connect(&m_OpacitySpinnerWidget, SIGNAL(valueChanged(double)), &m_OpacitySlider, SLOT(setValue(double)));
	QObject::connect(&m_OpacitySlider, SIGNAL(valueChanged(double)), this, SLOT(OnSetOpacity(double)));
    QObject::connect(&gStatus, SIGNAL(RenderBegin()), this, SLOT(OnRenderBegin()));
    QObject::connect(this, SIGNAL(CheckUpdated()), this, SLOT(OnCheckUpdated()));

    m_Button.setText(QString::fromStdString("Refresh"));

    QObject::connect(&m_Button, SIGNAL(clicked()), this, SLOT(OnButtonClick()));    
    QObject::connect(&gMeshRendering, SIGNAL(MajorClassChanged()), this, SLOT(OnMajorClassChanged(void)));

    max = OpacityArray[0][1];
    for(int i=1; i<OpacityArray.size(); i++) {
        if (OpacityArray[i][0] > max) {
            max = OpacityArray[i][0];
        }
    }

    Buffer = new float[max];
    
    m_MainLayout.addWidget(&m_Tree, 0, 0);
    m_MainLayout.addWidget(label, 1, 0);
    m_MainLayout.addWidget(&m_OpacitySlider, 2, 0);
    m_MainLayout.addWidget(&m_OpacitySpinnerWidget, 3, 0);
    m_MainLayout.addWidget(&m_Button, 4, 0);

    delete label;
}

QSelectiveOpacityWidget::~QSelectiveOpacityWidget() {
    delete Buffer;
    delete MajorClass;
    delete MinorClass;
    delete SubClass;
}

bool operator==(QTreeWidgetItem A, QTreeWidgetItem* B) {
    return A.text(0) == B->text(0);
}

void QSelectiveOpacityWidget::OnCheckUpdated() {
    for (int i=0; i<SubClassSize; i++) {
        OpacityArray[i][1] = SubClass[i].checkState(0)/2.0f;
    }
}

void QSelectiveOpacityWidget::OnMajorClassChanged() {
    string Name = gMeshRendering.GetMajorClass();

    for (int i=0; i<MajorClassSize; i++) {
        if (Name == MajorClass[i].text(0).toStdString()) {
            MajorClass[i].setCheckState(0, Qt::Checked);
            UpdateCheckBox(&MajorClass[i]);
            int NumChildren = 0;
            for (int j=0; j<i; j++) {
                NumChildren += MajorClass[j].childCount();
            }
            for(int j=NumChildren; j<(NumChildren+MajorClass[i].childCount()); j++) {
                int NumGChildren = 0;
                for (int k=0; k<j; k++) {
                    NumGChildren += MinorClass[k].childCount();
                }
                for(int k=NumGChildren; k<(NumGChildren+MinorClass[j].childCount()); k++) {
                    OpacityArray[k][1] = 1.0f;
                }
            }
        }
        else {
            MajorClass[i].setCheckState(0, Qt::Unchecked);
            UpdateCheckBox(&MajorClass[i]);
            int NumChildren = 0;
            for (int j=0; j<i; j++) {
                NumChildren += MajorClass[j].childCount();
            }
            for(int j=NumChildren; j<(NumChildren+MajorClass[i].childCount()); j++) {
                int NumGChildren = 0;
                for (int k=0; k<j; k++) {
                    NumGChildren += MinorClass[k].childCount();
                }
                for(int k=NumGChildren; k<(NumGChildren+MinorClass[j].childCount()); k++) {
                    OpacityArray[k][1] = 0.0f;
                }
            }
        }
    }

    emit CheckUpdated();

    ResetTex();
}

void QSelectiveOpacityWidget::OnRenderBegin(void)
{
    ResetTex();
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
}

void QSelectiveOpacityWidget::UpdateCheckBox(QTreeWidgetItem* Item) {
    if (Item->childCount() > 0) {
        for (int i=0; i<Item->childCount(); i++) {
            Item->child(i)->setCheckState(0, Item->checkState(0));
            UpdateCheckBox(Item->child(i));
        }
    }

    if (Item->parent()) {
        if (Item->checkState(0) == Qt::Checked && Item->parent()->checkState(0) == Qt::Unchecked) {
            Item->parent()->setCheckState(0, Qt::PartiallyChecked);
            if (Item->parent()->parent()){
                if (Item->parent()->parent()->checkState(0) == Qt::Unchecked) {
                    Item->parent()->parent()->setCheckState(0, Qt::PartiallyChecked);
                }
            }
        }
        else if (Item->checkState(0) == Qt::Checked && Item->parent()->checkState(0) == Qt::PartiallyChecked) {
            bool partial = false;
            for (int i=0; i<Item->parent()->childCount(); i++) {
                if (Item->parent()->child(i)->checkState(0) == Qt::Unchecked) 
                    partial = true;
            }
            Item->parent()->setCheckState(0, (partial) ? Qt::PartiallyChecked : Qt::Checked);
        }
        else if (Item->checkState(0) == Qt::Unchecked && Item->parent()->checkState(0) == Qt::PartiallyChecked) {
            bool partial = false;
            for (int i=0; i<Item->parent()->childCount(); i++) {
                if (Item->parent()->child(i)->checkState(0) == Qt::Checked) 
                    partial = true;
            }
            Item->parent()->setCheckState(0, (partial) ? Qt::PartiallyChecked : Qt::Unchecked);
        }
    }

    emit CheckUpdated();

    ResetTex();
}

void QSelectiveOpacityWidget::OnSelection(QTreeWidgetItem* Item, int col) {
    if (Item->checkState(0) == Qt::Unchecked) {
        Item->setCheckState(0, Qt::Checked);
    }
    else {
        Item->setCheckState(0, Qt::Unchecked);
    }

    UpdateCheckBox(Item);

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
        OpacityArray[Index][1] = (float)m_OpacitySpinnerWidget.value();
        ResetTex();
    }
    if (MinorChanged) {
        int NumChildren = 0;
        for (int i=0; i<Index; i++) {
            NumChildren += MinorClass[i].childCount();
        }
        for(int i=NumChildren; i<(NumChildren+MinorClass[Index].childCount()); i++) {
            OpacityArray[i][1] = (float)m_OpacitySpinnerWidget.value();
        }
        ResetTex();
    }
    if (MajorChanged) {
        int NumChildren = 0;
        for (int i=0; i<Index; i++) {
            NumChildren += MajorClass[i].childCount();
        }
        for(int i=NumChildren; i<(NumChildren+MajorClass[Index].childCount()); i++) {
            int NumGChildren = 0;
            for (int j=0; j<i; j++) {
                NumGChildren += MinorClass[j].childCount();
            }
            for(int j=NumGChildren; j<(NumGChildren+MinorClass[i].childCount()); j++) {
                OpacityArray[j][1] = (float)m_OpacitySpinnerWidget.value();
            }
        }
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
        Buffer[(int)OpacityArray[i][0]-1] = OpacityArray[i][1];
    }

    gSelectiveOpacity.SetSize(max);
    gSelectiveOpacity.SetOpacityBuffer(Buffer);

    // for (int i = 0; i < max; i++) {
    //     if (Buffer[i] > 0)
    //         std::cout<<Buffer[i]<<"\n";
    // }

    //BindTextureSelectiveOpacity(Buffer, max);
}
