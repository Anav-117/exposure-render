#include <vtkMetaImageReader.h>
#include <vtkImageThreshold.h>
#include <vtkImageMarchingCubes.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkAppendPolyData.h>
#include <vtkPolyData.h>
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
#include <sstream>
#include <bitset>
#include <vector>

void FinalizeMesh(vtkSmartPointer<vtkPolyData>, std::string);

int main (int argc, char* argv[]) { 
	/*
	INPUT FORMAT
		(String Single/Multi, string Input_FileName, string Output_FileName, string LabelSetFile, String Level)
			Input File - MHD
			Output File - STL
			LabelSetFile - TXT file with each line containing exactly one label
			Level - L1/L2/L3 
			Single/Multi - Whether to create one single mesh from all labels or make individual meshes
	*/

    vtkSmartPointer<vtkMetaImageReader> ImageReader = vtkSmartPointer<vtkMetaImageReader>::New();
	
	//Reading Input File
	ImageReader->SetFileName(argv[2]);
    ImageReader->SetNumberOfScalarComponents(1);
	ImageReader->Update(); 

	fstream Labels;
	Labels.open(argv[4], ios::in);
	std::string rawline;

	std::cout<<"INPUT - "<<argv[2]<<"\n";
	std::cout<<"OUTPUT - "<<argv[3]<<"\n";
	std::cout<<"LABELS - "<<argv[4]<<"\n";
	if (argc == 5)
		std::cout<<"LEVEL - "<<argv[5]<<"\n";
	
	std::cout<<"SAVE TYPE - "<<argv[1]<<"\n";

	std::string Level = "L1";
	std::string NumMesh = "Single";

	//Setting Level of Extraction
	if (argc >= 5) {
		if (argc == 6) {
			if (strcmp(argv[1], "Multi") == 0) {
				NumMesh = "Multi";
			}
			else if (strcmp(argv[1], "Single") == 0) {
				NumMesh = "Single";
			}
			else {
				std:cout<<"Incorrect number of meshes";
				return -1;
			}
		}
		if (strcmp(argv[5],"L1") == 0) {
			Level = "L1";
		}
		else if (strcmp(argv[5],"L2") == 0) {
			Level = "L2";
		}
		else if (strcmp(argv[5],"L3") == 0) {
			Level = "L3";
		}
		else {
			std::cout<<"Incorrect Segment Level\n";
			return -1;
		}
	}

	//Image Threshold filter for creating Binary volume
	vtkSmartPointer<vtkImageThreshold> VolumeThreshold = vtkSmartPointer<vtkImageThreshold>::New();

	vtkSmartPointer<vtkPolyData> Poly;

	vtkSmartPointer<vtkAppendPolyData> Append = vtkSmartPointer<vtkAppendPolyData>::New();

	//Boolean to check for first iteration. During first iteration, no appending is done as Poly is empty.
	bool firstIter = true;
	std::vector<std::string> line;

	while (getline(Labels, rawline)) {
		std::stringstream ss(rawline);
		while (ss.good()) {
			std::string substr;
			getline(ss, substr, ',');
			line.push_back(substr);
		}

		//Convert Label to Bitmask
		std::string Segment = std::bitset<16>(std::stoi(line[0])).to_string();

		float start = 0;
		float end = 0;

		//Set thresholding range according to level of extraction
		if (Level == "L1") {
			std::string offset = "00000000000";
			Segment = Segment.substr(1, 4) + offset;
			start = (float) std::bitset<15>(Segment).to_ulong();
			end = start + 2047;
		}
		else if (Level == "L2") {
			std::string offset = "0000000"; 
			Segment = Segment.substr(1, 8) + offset;
			start = (float) std::bitset<15>(Segment).to_ulong();
			end = start + 127;
		}
		else if (Level == "L3") {
			std::string offset = "0";
			Segment = Segment.substr(1, 15) + offset;
			start = (float) std::bitset<16>(Segment).to_ulong();
			end = start + 0.5f;
		}

		std::cout<<"START - "<<start<<'\n';
		std::cout<<"END - "<<end<<'\n';

		//Creating Binary Volume
		VolumeThreshold->SetInputConnection(ImageReader->GetOutputPort());
		VolumeThreshold->SetInValue(255);
		VolumeThreshold->SetOutValue(0);
		VolumeThreshold->ThresholdBetween(start, end);

		//Extracting Mesh
		vtkSmartPointer<vtkImageMarchingCubes> IExtractor = vtkSmartPointer<vtkImageMarchingCubes>::New();
		IExtractor->SetInputConnection(VolumeThreshold->GetOutputPort());
		IExtractor->ComputeNormalsOn();
		IExtractor->ComputeGradientsOn();
		IExtractor->SetValue(0, 200);
		IExtractor->Update();

		if (NumMesh == "Multi") {
			Poly = IExtractor->GetOutput(0);
			FinalizeMesh(Poly, line[1]);
			line.clear();
			continue;
		}

		if (firstIter) {
			Poly = IExtractor->GetOutput(0);
			firstIter = false;
			line.clear();
			continue;
		}

		//Appending Mesh to Poly
		Append->AddInputData(Poly);
		Append->AddInputData(IExtractor->GetOutput(0));
		Append->Update();

		Poly = Append->GetOutput(0);
		line.clear();
	}

	std::cout<<"Number of Polygons - "<<Poly->GetNumberOfPolys()<<"\n";

	if (NumMesh == "Single") {
		FinalizeMesh(Poly, argv[3]);
	}

    return 0;
}

void FinalizeMesh(vtkSmartPointer<vtkPolyData> Poly, std::string OutputFile) {
	//Triangulating mesh for Decimation and Smoothing
	vtkSmartPointer<vtkTriangleFilter> Triangluation = vtkSmartPointer<vtkTriangleFilter>::New();
	Triangluation->SetInputData(Poly);
	Triangluation->Update();

	//Decimating Mesh
	vtkSmartPointer<vtkQuadricDecimation> Decimate = vtkSmartPointer<vtkQuadricDecimation>::New();
	Decimate->SetInputConnection(Triangluation->GetOutputPort());
	Decimate->VolumePreservationOn();
	Decimate->SetTargetReduction(0.95);
	Decimate->Update();

	std::cout<<"Number of Polygons after Decimation - "<<Decimate->GetOutput(0)->GetNumberOfPolys()<<"\n";

	//Smoothing Mesh
	vtkSmartPointer<vtkSmoothPolyDataFilter> SmoothVolume = vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
	SmoothVolume->SetInputConnection(Decimate->GetOutputPort());
	SmoothVolume->SetNumberOfIterations(15);
	SmoothVolume->SetRelaxationFactor(0.1);
	SmoothVolume->Update();

	std::cout<< "OUTPUT " <<OutputFile<<"\n";

	//Writing Mesh to STL file
	vtkSmartPointer<vtkSTLWriter> writer = vtkSmartPointer<vtkSTLWriter>::New();
	writer->SetInputConnection(SmoothVolume->GetOutputPort());
	writer->SetFileName(OutputFile.c_str());
	writer->SetFileTypeToBinary();
	writer->Write();

	//Mapper to display mesh
	vtkSmartPointer<vtkPolyDataMapper> Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	Mapper->SetInputConnection(SmoothVolume->GetOutputPort());
	Mapper->ScalarVisibilityOff();
	Mapper->Update();

	vtkSmartPointer<vtkNamedColors> colors = vtkSmartPointer<vtkNamedColors>::New();

	vtkSmartPointer<vtkActor> MeshActor = vtkSmartPointer<vtkActor>::New();
	MeshActor->SetMapper(Mapper);
	MeshActor->GetProperty()->SetDiffuseColor(colors->GetColor3d("Mint").GetData());

	//Setting Up renderer
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
}