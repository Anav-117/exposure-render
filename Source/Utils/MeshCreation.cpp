#include <vtkMetaImageReader.h>
#include <vtkImageThreshold.h>
#include <vtkImageMarchingCubes.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkBooleanOperationPolyDataFilter.h>
#include <vtkAppendPolyData.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkCamera.h>
#include <vtkProperty.h>
#include <vtkQuadricDecimation.h>
#include <vtkTriangleFilter.h>
#include <vtkSTLWriter.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkNamedColors.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>

int main (int argc, char* argv[]) { //(string Input_FileName, string Output_FileName, string LabelSetFile)
    vtkSmartPointer<vtkMetaImageReader> ImageReader = vtkSmartPointer<vtkMetaImageReader>::New();
    ImageReader->SetFileName(argv[1]);
    ImageReader->SetNumberOfScalarComponents(1);
	ImageReader->Update();

	fstream Labels;
	Labels.open(argv[3], ios::in);
	std::string rawline;

	std::cout<<"INPUT - "<<argv[1]<<"\n";
	std::cout<<"OUTPUT - "<<argv[2]<<"\n";
	std::cout<<"LABELS - "<<argv[3]<<"\n";
	//std::cout<<"START - "<<argv[3]<<"\n";
	//std::cout<<"END - "<<argv[4]<<"\n";

	vtkSmartPointer<vtkSphereSource> Sphere = vtkSmartPointer<vtkSphereSource>::New();
	vtkSmartPointer<vtkImageThreshold> VolumeThreshold = vtkSmartPointer<vtkImageThreshold>::New();
	vtkSmartPointer<vtkPolyData> Poly;
	Sphere->Update();
	Poly = Sphere->GetOutput(0);

	vtkSmartPointer<vtkBooleanOperationPolyDataFilter> BooleanOperation = vtkSmartPointer<vtkBooleanOperationPolyDataFilter>::New();
	vtkSmartPointer<vtkAppendPolyData> Append = vtkSmartPointer<vtkAppendPolyData>::New();
	BooleanOperation->SetOperationToUnion();
	//std::string start = argv[3];
	//std::string end = argv[4];

	while (getline(Labels, rawline)) {
		VolumeThreshold->SetInputConnection(ImageReader->GetOutputPort());
		VolumeThreshold->SetInValue(255);
		VolumeThreshold->SetOutValue(0);
		VolumeThreshold->ThresholdBetween(std::stoi(rawline)-0.5, std::stoi(rawline)+0.5);

		vtkSmartPointer<vtkImageMarchingCubes> IExtractor = vtkSmartPointer<vtkImageMarchingCubes>::New();
		IExtractor->SetInputConnection(VolumeThreshold->GetOutputPort());
		IExtractor->ComputeNormalsOn();
		IExtractor->ComputeGradientsOn();
		IExtractor->SetValue(0, 200);
		IExtractor->Update();

		Append->AddInputData(Poly);
		Append->AddInputData(IExtractor->GetOutput(0));
		Append->Update();

		Poly = Append->GetOutput(0);
	}

	// vtkSmartPointer<vtkPolyDataMapper> Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	// Mapper->SetInputData(Poly);
	// Mapper->ScalarVisibilityOff();
	// Mapper->Update();

	std::cout<<"Number of Polygons - "<<Append->GetOutput(0)->GetNumberOfPolys()<<"\n";

	vtkSmartPointer<vtkTriangleFilter> Triangluation = vtkSmartPointer<vtkTriangleFilter>::New();
	Triangluation->SetInputData(Poly);
	Triangluation->Update();

	vtkSmartPointer<vtkQuadricDecimation> Decimate = vtkSmartPointer<vtkQuadricDecimation>::New();
	Decimate->SetInputConnection(Triangluation->GetOutputPort());
	Decimate->VolumePreservationOn();
	Decimate->SetTargetReduction(0.01);
	Decimate->Update();

	std::cout<<"Number of Polygons after Decimation - "<<Poly->GetNumberOfPolys()<<"\n";

	vtkSmartPointer<vtkSmoothPolyDataFilter> SmoothVolume = vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
	SmoothVolume->SetInputConnection(Decimate->GetOutputPort());
	SmoothVolume->SetNumberOfIterations(10);
	SmoothVolume->FeatureEdgeSmoothingOn();
	SmoothVolume->BoundarySmoothingOn();
	SmoothVolume->Update();

	vtkSmartPointer<vtkSTLWriter> writer = vtkSmartPointer<vtkSTLWriter>::New();
	writer->SetInputConnection(SmoothVolume->GetOutputPort());
	writer->SetFileName(argv[2]);
	writer->Write();

	vtkSmartPointer<vtkPolyDataMapper> Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	Mapper->SetInputConnection(SmoothVolume->GetOutputPort());
	Mapper->ScalarVisibilityOff();
	Mapper->Update();

	vtkSmartPointer<vtkNamedColors> colors = vtkSmartPointer<vtkNamedColors>::New();

	vtkSmartPointer<vtkActor> MeshActor = vtkSmartPointer<vtkActor>::New();
	MeshActor->SetMapper(Mapper);
	MeshActor->GetProperty()->SetDiffuseColor(colors->GetColor3d("Mint").GetData());

	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
	renderer->AddViewProp(MeshActor);
	renderer->SetBackground(colors->GetColor3d("Silver").GetData());
	
	vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);
	renderWindow->SetSize(640, 480);
	renderWindow->SetWindowName("MeshCreator");

	double viewUp[3] = {0.0, 0.0, 1.0};
	double position[3] = {0.0, -1.0, 0.0};
	renderer->GetActiveCamera()->SetFocalPoint(0.0, 0.0, 0.0);
	renderer->GetActiveCamera()->SetViewUp(viewUp);
	renderer->GetActiveCamera()->SetPosition(position);
	renderer->ResetCamera();
	renderer->GetActiveCamera()->Dolly(1.4);
	renderer->ResetCameraClippingRange();

	vtkNew<vtkRenderWindowInteractor> renWinInteractor;
	renWinInteractor->SetRenderWindow(renderWindow);

	renderWindow->Render();
	renWinInteractor->Start();

    return 0;
}