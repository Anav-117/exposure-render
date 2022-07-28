#include "PoseTraceWidget.h"
#include "RenderThread.h"
#include "Scene.h"
#include "Camera.h"
#include <vector>
#include <fstream>
#include <unistd.h>

QPoseTraceWidget::QPoseTraceWidget(QWidget* pParent) :
	m_Layout(), 
	m_RenderButton(),
	m_FileName(),
    m_Progress()
{
	m_Layout.setAlignment(Qt::AlignTop);
	setLayout(&m_Layout);

	m_RenderButton.setText(QString::fromStdString("Render"));
    m_FileName.setText(QString::fromStdString("FILENAME"));

	QObject::connect(&m_RenderButton, SIGNAL(clicked()), this, SLOT(OnRenderButtonClicked()));
    QObject::connect(&gStatus, SIGNAL(RenderBegin()), this, SLOT(OnRenderBegin()));

    m_Layout.addWidget(&m_FileName, 0, 0);
    m_Layout.addWidget(&m_RenderButton, 1, 0);
    m_Layout.addWidget(&m_Progress, 2, 0);

    if (gCamera.GetPoseFileDir() == "POSE FILE UNAVAILABLE") {
        m_RenderButton.setEnabled(false);
    }
}

QPoseTraceWidget::~QPoseTraceWidget() {}

void QPoseTraceWidget::OnRenderBegin() {
    m_FileName.setText(QString::fromStdString(gCamera.GetPoseFileDir()));
    
    if (gCamera.GetPoseFileDir() == "POSE FILE UNAVAILABLE") {
        m_RenderButton.setEnabled(false);
    }
    else {
        m_Progress.setMaximum(gCamera.GetNumPoses());
    }
}

void QPoseTraceWidget::OnRenderButtonClicked() {
    int i = 0;
    string OutputPath = gCamera.GetPoseFileDir() + "/Renders/";
    if (!QDir(QString::fromStdString(OutputPath)).exists())
        QDir().mkdir(QString::fromStdString(OutputPath));
    while (gCamera.CycleCameraParams()) {				
        sleep(10);
        SaveImage((unsigned char*)gpRenderThread->GetRenderImage(), gScene.m_Camera.m_Film.m_Resolution.GetResX(), gScene.m_Camera.m_Film.m_Resolution.GetResY(), QString::fromStdString(OutputPath + std::to_string(i) + ".png"));
        m_Progress.setValue(i+1);
        i++;
    }
}