#include "MeshRenderingWidget.h"
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkNamedColors.h>
#include <vtkImageGaussianSmooth.h>
#include <vtkImageMarchingCubes.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkOutlineFilter.h>
#include <vtkImageThreshold.h>
#include <vtkCamera.h>
#include <vtkMetaImageReader.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLPolyDataReader.h>

vtkSmartPointer<vtkXMLPolyDataReader> polyReader = vtkXMLPolyDataReader::New();
vtkSmartPointer<vtkPolyDataMapper> Mapper = vtkPolyDataMapper::New();
vtkSmartPointer<vtkActor> surface = vtkActor::New();
vtkSmartPointer<vtkOutlineFilter> outlineData = vtkOutlineFilter::New();
vtkSmartPointer<vtkPolyDataMapper> mapOutline = vtkPolyDataMapper::New();
vtkSmartPointer<vtkActor> outline = vtkActor::New();
vtkSmartPointer<vtkRenderer> renderer = vtkRenderer::New();

QMeshRenderingWidget::QMeshRenderingWidget(QWidget* pParent) :
	QVTKOpenGLNativeWidget(pParent)    
{
	polyReader->SetFileName("../MajorClassMeshes/The Integument.vtp");
	polyReader->Update();
	this->SetupRenderer();
}

void QMeshRenderingWidget::SetupRenderer() {
	vtkSmartPointer<vtkNamedColors> colors = vtkNamedColors::New();
	
	std::array<unsigned char, 4> skinColor{{255, 0, 0, 255}};
  	colors->SetColor("SkinColor", skinColor.data());
  	std::array<unsigned char, 4> backColor{{255, 229, 200, 255}};
  	colors->SetColor("BackfaceColor", backColor.data());
  	std::array<unsigned char, 4> bkg{{51, 77, 102, 255}};
  	colors->SetColor("BkgColor", bkg.data());

    // vtkSmartPointer<vtkMetaImageReader> ImageReader = vtkMetaImageReader::New();
    // ImageReader->SetFileName("/home/anav/ExposureRenderer/preprocessed/Cropped/Isosurface.mhd");
    // ImageReader->SetNumberOfScalarComponents(1);
	// ImageReader->SetDataScalarTypeToUnsignedChar();
	// ImageReader->Update();
	
	Mapper->SetInputConnection(polyReader->GetOutputPort());
	Mapper->ScalarVisibilityOff();

	surface->SetMapper(Mapper);
	surface->GetProperty()->SetDiffuseColor(colors->GetColor3d("SkinColor").GetData());

	vtkSmartPointer<vtkProperty> backProp = vtkProperty::New();
 	backProp->SetDiffuseColor(colors->GetColor3d("BackfaceColor").GetData());
  	surface->SetBackfaceProperty(backProp);

 	// An outline provides context around the data.

	outlineData->SetInputConnection(polyReader->GetOutputPort());

  	mapOutline->SetInputConnection(outlineData->GetOutputPort());

  	outline->SetMapper(mapOutline);
  	outline->GetProperty()->SetColor(colors->GetColor3d("Black").GetData());

    renderer->AddActor(outline);
    renderer->AddActor(surface);
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

	QObject::connect(&gMeshRendering, SIGNAL(MajorClassChanged()), this, SLOT(OnMajorClassChanged(void)));

    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow = vtkGenericOpenGLRenderWindow::New();
    this->SetRenderWindow(renderWindow);
    //this->setFixedHeight(380);
	this->GetRenderWindow()->AddRenderer(renderer);
    //std::cout<<"PIXELS!!!! - "<<renderWindow->ReadPixels()<<"\n";
	//this->show();
}

void QMeshRenderingWidget::OnMajorClassChanged(void) {
	string dir = "../MajorClassMeshes/";
	string ext = ".vtp";
	polyReader->SetFileName((dir+gMeshRendering.GetMajorClass()+ext).c_str());
	polyReader->Update();
	Mapper->Update();
	outlineData->Update();
	mapOutline->Update();
	renderer->ResetCamera();
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