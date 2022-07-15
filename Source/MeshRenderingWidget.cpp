#include "MeshRenderingWidget.h"
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkImageGaussianSmooth.h>
#include <vtkImageMarchingCubes.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkOutlineFilter.h>
#include <vtkImageThreshold.h>
#include <vtkCamera.h>
#include <vtkMetaImageReader.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkSTLWriter.h>
#include <vtkSTLReader.h>
#include <vtkPropPicker.h>
#include <QVTKInteractor.h>
#include <vtkNamedColors.h>
#include <array>
#include <fstream>
#include <string>

void SaveMeshFile(int startValue, int endValue, string fileName);

vector<string> FileNames;// {"Skeletal system", "Lymphoid system", "Nervous system", "Sensory organs", "The Integument", "Articular system", "Muscular system", "Alimentary system", "Respiratory system", "Urinary system", "Genital system", "Endocrine glands", "Cardiovascular system"};
vector<std::array<unsigned char, 4>> colors = {{47, 79, 79, 255}, {139, 69, 19, 255}, {0, 128, 0, 255}, {75, 0, 130, 255}, {128, 0, 0, 255}, {128, 128, 0, 255}, {0, 128, 128, 255}, {0, 0, 128, 255}, {238, 232, 170, 255}, {100, 149, 237, 255}, {255, 105, 180, 255}, {0, 255, 0, 255}, {128, 0, 128, 255}};
vector<vtkSmartPointer<vtkSTLReader>> polyReader;
vector<vtkSmartPointer<vtkPolyDataMapper>> Mapper;
vector<vtkSmartPointer<vtkActor>> surface;
vtkSmartPointer<vtkRenderer> renderer = vtkRenderer::New();
int NumSegments;

QMeshRenderingWidget::QMeshRenderingWidget(QWidget* pParent) :
	QVTKOpenGLNativeWidget(pParent),
	m_Layout(), 
	m_DeleteButton(),
	m_ResetButton()
{
	m_Layout.setAlignment(Qt::AlignBottom);
	setLayout(&m_Layout);

	m_DeleteButton.setText(QString::fromStdString("Delete"));
	m_ResetButton.setText(QString::fromStdString("Reset"));

	QObject::connect(&m_DeleteButton, SIGNAL(clicked()), this, SLOT(OnDelete()));
	QObject::connect(&m_ResetButton, SIGNAL(clicked()), this, SLOT(OnReset()));

	fstream Names;
	Names.open("../MajorClassMeshes/ClassNames.txt", ios::in);

	string rawline;
	NumSegments=0;
	while(getline(Names, rawline)) {
		FileNames.push_back(rawline);
		NumSegments++;
	}

	Names.close();

	vtkSmartPointer<vtkNamedColors> Colors = vtkSmartPointer<vtkNamedColors>::New();

	for (int i=0; i<13; i++) {
		Colors->SetColor(to_string(i), colors[i].data());
		polyReader.push_back(vtkSmartPointer<vtkSTLReader>::New());
		Mapper.push_back(vtkSmartPointer<vtkPolyDataMapper>::New());
		surface.push_back(vtkSmartPointer<vtkActor>::New());
	}

	string dir = "../MajorClassMeshes/";
	string ext = ".stl";

	for (int i=0; i<NumSegments; i++) {
		if (i == 1 || i==9 || i==10) 
			continue;
		polyReader[i]->SetFileName((dir+FileNames[i]+ext).c_str());
		polyReader[i]->Update();
		Mapper[i]->SetInputConnection(polyReader[i]->GetOutputPort());
		Mapper[i]->ScalarVisibilityOff();
		surface[i]->SetMapper(Mapper[i]);
		surface[i]->GetProperty()->SetColor(Colors->GetColor3d(to_string(i)).GetData());
		surface[i]->GetProperty()->SetInterpolationToPhong();
	}
	
	this->SetupRenderer();
}

void QMeshRenderingWidget::OnDelete() {
	surface[Selected]->GetProperty()->SetOpacity(0);
	surface[Selected]->ApplyProperties();
}

void QMeshRenderingWidget::OnReset() {
	for (int i=0; i<NumSegments; i++) {
		surface[i]->GetProperty()->SetOpacity(1.0);
		surface[i]->ApplyProperties();
	}
}

void QMeshRenderingWidget::SetupRenderer() {

    // vtkSmartPointer<vtkMetaImageReader> ImageReader = vtkMetaImageReader::New();
    // ImageReader->SetFileName("/home/anav/ExposureRenderer/preprocessed/Cropped/Isosurface.mhd");
    // ImageReader->SetNumberOfScalarComponents(1);
	// ImageReader->SetDataScalarTypeToUnsignedChar();
	// ImageReader->Update();

 	// An outline provides context around the data.

	for (int i=0; i<NumSegments; i++) {
    	renderer->AddActor(surface[i]);
	}
    renderer->SetBackground(0.6, 0.2, 0.5);
    
    vtkSmartPointer<vtkCamera> aCamera = vtkSmartPointer<vtkCamera>::New();
  	aCamera->SetViewUp(0, 0, -1);
	aCamera->SetPosition(0, -1, 0);
	aCamera->SetFocalPoint(0, 0, 0);
	aCamera->ComputeViewPlaneNormal();
	aCamera->Azimuth(30.0);
	aCamera->Elevation(30.0);
    renderer->SetActiveCamera(aCamera);
	renderer->ResetCamera();
	aCamera->Dolly(1.5);

    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

	QObject::connect(&gMeshRendering, SIGNAL(ScalarRangeChanged()), this, SLOT(OnScalarRangeChanged(void)));

    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    this->SetRenderWindow(renderWindow);
	vtkSmartPointer<QVTKInteractor> interactor = vtkSmartPointer<QVTKInteractor>::New();
	interactor->SetRenderWindow(this->GetRenderWindow());
    //this->setFixedHeight(380);
	this->GetRenderWindow()->AddRenderer(renderer);
	vtkSmartPointer<MouseInteractorHighLightActor> ClassPicker = vtkSmartPointer<MouseInteractorHighLightActor>::New();
	ClassPicker->SetDefaultRenderer(renderer);
	interactor->SetInteractorStyle(ClassPicker);

	interactor->Initialize();

    //std::cout<<"PIXELS!!!! - "<<renderWindow->ReadPixels()<<"\n";
	//this->show();

	m_Layout.addWidget(&m_DeleteButton, 0, 0);
	m_Layout.addWidget(&m_ResetButton, 0, 1);
}

void QMeshRenderingWidget::OnScalarRangeChanged(void) {
	double* Range = gMeshRendering.GetScalarRange();

	for (int i=0; i<13; i++) {
		if (Range == Mapper[i]->GetScalarRange()) {
			gMeshRendering.SetMajorClass(FileNames[i]);
			Selected = i;
		}
	}
}

QMeshRenderingWidget::~QMeshRenderingWidget() {}

void QMeshRenderingWidget::OnRenderBegin() {

}