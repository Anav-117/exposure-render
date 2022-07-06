#include <vtkMetaImageReader.h>
#include <vtkImageThreshold.h>
#include <vtkImageMarchingCubes.h>
#include <vtkSTLWriter.h>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>

int main (int argc, char* argv[]) { //(string Input_FileName, string Output_FileName, int StartValue, int EndValue)//string LabelSetFile)
    vtkSmartPointer<vtkMetaImageReader> ImageReader = vtkSmartPointer<vtkMetaImageReader>::New();
    ImageReader->SetFileName(argv[1]);
    ImageReader->SetNumberOfScalarComponents(1);
	//ImageReader->SetDataScalarTypeToUnsignedChar();
	ImageReader->Update();

	std::cout<<"INPUT - "<<argv[1]<<"\n";
	std::cout<<"OUTPUT - "<<argv[2]<<"\n";
	std::cout<<"START - "<<argv[3]<<"\n";
	std::cout<<"END - "<<argv[4]<<"\n";

    
	vtkSmartPointer<vtkImageThreshold> VolumeThreshold = vtkSmartPointer<vtkImageThreshold>::New();

	std::string start = argv[3];
	std::string end = argv[4];

	VolumeThreshold->SetInputConnection(ImageReader->GetOutputPort());
	VolumeThreshold->SetInValue(255);
	VolumeThreshold->SetOutValue(0);
	VolumeThreshold->ThresholdBetween(std::stoi(start), std::stoi(end));

	vtkSmartPointer<vtkImageMarchingCubes> IExtractor = vtkSmartPointer<vtkImageMarchingCubes>::New();
	IExtractor->SetInputConnection(VolumeThreshold->GetOutputPort());
	IExtractor->ComputeNormalsOn();
	IExtractor->ComputeGradientsOn();
	IExtractor->SetValue(0, 200);

	vtkSmartPointer<vtkSTLWriter> writer = vtkSmartPointer<vtkSTLWriter>::New();
	writer->SetInputConnection(IExtractor->GetOutputPort());
	writer->SetFileName(argv[2]);
	writer->Write();

    return 0;
}