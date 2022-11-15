#include <vtkMetaImageReader.h>
#include <vtkMetaImageWriter.h>
#include <vtkImageThreshold.h>
#include <vtkSmartPointer.h>
#include <vtkImageIterator.h>
#include <vtkImageData.h>
#include <fstream>
#include <string>
#include <sstream>
#include <bitset>
#include <vector>

int main (int argc, char* argv[]) { 
	/*
	INPUT FORMAT
		(String Single/Multi, string Input_FileName, string Segment_FileName, string Output_FileName, string LabelSetFile, String Level)
			Input File - MHD
			Output File - STL
			LabelSetFile - TXT file with each line containing exactly one label
			Level - L1/L2/L3 
			Single/Multi - Whether to create one single mesh from all labels or make individual meshes
	*/

    vtkSmartPointer<vtkMetaImageReader> ImageReader = vtkSmartPointer<vtkMetaImageReader>::New();
	
	//Reading Segment File
	ImageReader->SetFileName(argv[2]);
    ImageReader->SetNumberOfScalarComponents(1);
	ImageReader->Update();

    vtkSmartPointer<vtkMetaImageReader> SegmentImageReader = vtkSmartPointer<vtkMetaImageReader>::New();
	
	//Reading Segment File
	SegmentImageReader->SetFileName(argv[3]);
    SegmentImageReader->SetNumberOfScalarComponents(1);
	SegmentImageReader->Update(); 

	fstream Labels;
	Labels.open(argv[5], ios::in);
	std::string rawline;

	std::cout<<"INPUT - "<<argv[3]<<"\n";
	std::cout<<"OUTPUT - "<<argv[4]<<"\n";
	std::cout<<"LABELS - "<<argv[5]<<"\n";
	if (argc == 5)
		std::cout<<"LEVEL - "<<argv[6]<<"\n";
	
	std::cout<<"SAVE TYPE - "<<argv[1]<<"\n";

	std::string Level = "L1";
	std::string NumMesh = "Single";
    
    std::cout<<"1\n";
	
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
		if (strcmp(argv[6],"L1") == 0) {
			Level = "L1";
		}
		else if (strcmp(argv[6],"L2") == 0) {
			Level = "L2";
		}
		else if (strcmp(argv[6],"L3") == 0) {
			Level = "L3";
		}
		else {
			std::cout<<"Incorrect Segment Level\n";
			return -1;
		}
	}

    std::cout<<"2\n";

	//Image Threshold filter for creating Binary volume
	vtkSmartPointer<vtkImageThreshold> VolumeThreshold = vtkSmartPointer<vtkImageThreshold>::New();

	std::vector<std::string> line;
    std::cout<<"2.1\n";
	while (getline(Labels, rawline)) {
        std::cout<<"2.2\n";
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

        std::cout<<"3\n";

		//Creating Binary Volume
		VolumeThreshold->SetInputConnection(SegmentImageReader->GetOutputPort());
		VolumeThreshold->SetInValue(255);
		VolumeThreshold->SetOutValue(0);
		VolumeThreshold->ThresholdBetween(start, end);

        int* Extent= ImageReader->GetOutput()->GetExtent();

        std::cout<<"4\n";

        vtkImageIterator<double> it(ImageReader->GetOutput(), Extent);
        while (!it.IsAtEnd()){
            double* valIt = it.BeginSpan();
            double* valEnd = it.EndSpan();
            while (valIt != valEnd){
                // Increment for each component
                double x = *valIt++;
                double y = *valIt++;
                double z = *valIt++;
                if(VolumeThreshold->GetOutput()->GetScalarComponentAsFloat(x, y, z, 0.0) == 0.0) {
                    ImageReader->GetOutput()->SetScalarComponentFromDouble(x, y, z, 0, 0.0);    
                }
            }
            std::cout << std::endl;
            it.NextSpan();
        }

        std::cout<<"5\n";

        vtkSmartPointer<vtkMetaImageWriter> writer = vtkSmartPointer<vtkMetaImageWriter>::New();
        writer->SetInputConnection(ImageReader->GetOutputPort());
        writer->SetFileName(argv[4]);
        writer->Write();

    }
}