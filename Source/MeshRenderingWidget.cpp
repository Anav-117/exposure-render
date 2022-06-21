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
#include <vtkPropPicker.h>
#include <QVTKInteractor.h>

void SaveMeshFile(int startValue, int endValue, string fileName);

vector<string> FileNames = {"Skeletal system", "Lymphoid system", "Nervous system", "Sensory organs", "The Integument", "Articular system", "Muscular system", "Alimentary system", "Respiratory system", "Urinary system", "Genital system", "Endocrine glands", "Cardiovascular system"};
vector<vector<int>> colors = {{47, 79, 79}, {139, 69, 19}, {0, 128, 0}, {75, 0, 130}, {128, 0, 0}, {128, 128, 0}, {0, 128, 128}, {0, 0, 128}, {238, 232, 170}, {100, 149, 237}, {255, 105, 180}, {0, 255, 0}, {128, 0, 128}};
vector<vtkSmartPointer<vtkXMLPolyDataReader>> polyReader;
vector<vtkSmartPointer<vtkPolyDataMapper>> Mapper;
vector<vtkSmartPointer<vtkActor>> surface;
vector<vtkSmartPointer<vtkOutlineFilter>> outlineData;
vector<vtkSmartPointer<vtkPolyDataMapper>> mapOutline;
vector<vtkSmartPointer<vtkActor>> outline;
vtkSmartPointer<vtkRenderer> renderer = vtkRenderer::New();

QMeshRenderingWidget::QMeshRenderingWidget(QWidget* pParent) :
	QVTKOpenGLNativeWidget(pParent)    
{
	//SaveMeshFile(4000, 6000, "../MajorClassMeshes/Lymphoid system.vtp");

	for (int i=0; i<13; i++) {
		polyReader.push_back(vtkXMLPolyDataReader::New());
		Mapper.push_back(vtkPolyDataMapper::New());
		surface.push_back(vtkActor::New());
		outlineData.push_back(vtkOutlineFilter::New());
		mapOutline.push_back(vtkPolyDataMapper::New());
		outline.push_back(vtkActor::New());
	}

	string dir = "../MajorClassMeshes/";
	string ext = ".vtp";

	for (int i=0; i<13; i++) {
		polyReader[i]->SetFileName((dir+FileNames[i]+ext).c_str());
		polyReader[i]->Update();
		Mapper[i]->SetInputConnection(polyReader[i]->GetOutputPort());
		Mapper[i]->ScalarVisibilityOff();
		surface[i]->SetMapper(Mapper[i]);
		surface[i]->GetProperty()->SetDiffuseColor(colors[i][0], colors[i][1], colors[i][2]);
		if (i == 4) {
			surface[i]->GetProperty()->SetOpacity(0);
		}
		else {
			vtkSmartPointer<vtkProperty> backProp = vtkProperty::New();
			backProp->SetDiffuseColor(50, 50, 50);
			surface[i]->SetBackfaceProperty(backProp);
		}
		outlineData[i]->SetInputConnection(polyReader[i]->GetOutputPort());
		mapOutline[i]->SetInputConnection(outlineData[i]->GetOutputPort());
		outline[i]->SetMapper(mapOutline[i]);
		outline[i]->GetProperty()->SetColor(0, 0, 0);
	}
	
	this->SetupRenderer();
}

void QMeshRenderingWidget::SetupRenderer() {

    // vtkSmartPointer<vtkMetaImageReader> ImageReader = vtkMetaImageReader::New();
    // ImageReader->SetFileName("/home/anav/ExposureRenderer/preprocessed/Cropped/Isosurface.mhd");
    // ImageReader->SetNumberOfScalarComponents(1);
	// ImageReader->SetDataScalarTypeToUnsignedChar();
	// ImageReader->Update();

 	// An outline provides context around the data.

	for (int i=0; i<13; i++) {
		//renderer->AddActor(outline[i]);
    	renderer->AddActor(surface[i]);
	}
    renderer->SetBackground(0.6, 0.2, 0.5);
    
    vtkSmartPointer<vtkCamera> aCamera = vtkCamera::New();
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

    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow = vtkGenericOpenGLRenderWindow::New();
    this->SetRenderWindow(renderWindow);
	vtkSmartPointer<QVTKInteractor> interactor = QVTKInteractor::New();
	interactor->SetRenderWindow(this->GetRenderWindow());
    //this->setFixedHeight(380);
	this->GetRenderWindow()->AddRenderer(renderer);
	vtkSmartPointer<MouseInteractorHighLightActor> ClassPicker = MouseInteractorHighLightActor::New();
	ClassPicker->SetDefaultRenderer(renderer);
	interactor->SetInteractorStyle(ClassPicker);

	interactor->Initialize();

    //std::cout<<"PIXELS!!!! - "<<renderWindow->ReadPixels()<<"\n";
	//this->show();
}


void QMeshRenderingWidget::OnScalarRangeChanged(void) {
	double* Range = gMeshRendering.GetScalarRange();

	for (int i=0; i<13; i++) {
		if (Range == Mapper[i]->GetScalarRange()) {
			gMeshRendering.SetMajorClass(FileNames[i]);
		}
	}
}

void SaveMeshFile(int startValue, int endValue, string fileName) {
    vtkSmartPointer<vtkMetaImageReader> ImageReader = vtkMetaImageReader::New();
    ImageReader->SetFileName("/home/anav/ExposureRenderer/preprocessed/Cropped/Isosurface.mhd");
    ImageReader->SetNumberOfScalarComponents(1);
	ImageReader->SetDataScalarTypeToUnsignedChar();
	ImageReader->Update();

	vtkSmartPointer<vtkImageThreshold> VolumeThreshold = vtkImageThreshold::New();
	VolumeThreshold->SetInputConnection(ImageReader->GetOutputPort());
	VolumeThreshold->SetInValue(255);
	VolumeThreshold->SetOutValue(0);
	VolumeThreshold->ThresholdBetween(startValue, endValue);
	//threshold->AllScalarsOff();

	vtkSmartPointer<vtkImageGaussianSmooth> smoothVolume = vtkImageGaussianSmooth::New();
	smoothVolume->SetDimensionality(3);
	smoothVolume->SetInputConnection(VolumeThreshold->GetOutputPort());
	smoothVolume->SetStandardDeviations(1.75, 1.75, 0);
	smoothVolume->SetRadiusFactor(2);

	vtkSmartPointer<vtkImageMarchingCubes> IExtractor = vtkImageMarchingCubes::New();
	IExtractor->SetInputConnection(smoothVolume->GetOutputPort());
	IExtractor->ComputeNormalsOn();
	IExtractor->ComputeGradientsOn();
	IExtractor->SetValue(0, 200);

	vtkSmartPointer<vtkXMLPolyDataWriter> writer = vtkXMLPolyDataWriter::New();
	writer->SetInputConnection(IExtractor->GetOutputPort());
	writer->SetFileName(fileName.c_str());
	writer->Write();
}


QMeshRenderingWidget::~QMeshRenderingWidget() {}

void QMeshRenderingWidget::OnRenderBegin() {

}