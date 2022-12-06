#include <vtkMetaImageReader.h>
#include <vtkMetaImageWriter.h>
#include <vtkImageThreshold.h>
#include <vtkSmartPointer.h>
#include <vtkImageIterator.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkUnsignedShortArray.h>
#include <vtkImageCast.h>
#include <fstream>
#include <string>
#include <sstream>
#include <bitset>
#include <vector>

void FinalizeVolume(vtkSmartPointer<vtkImageData>, std::string);

int main (int argc, char* argv[]) { 
	/*
	INPUT FORMAT
		(String Single/Multi, string Input_FileName, string Segment_FileName, string Output_FileName, string LabelSetFile, String Level)
			Input File - MHD
			Output File - MHD
			LabelSetFile - TXT file with each line containing exactly one label
			Level - L1/L2/L3 
			Single/Multi - Whether to create one single mesh from all labels or make individual meshes
	*/

    vtkSmartPointer<vtkMetaImageReader> ImageReader = vtkSmartPointer<vtkMetaImageReader>::New();
	
	//Reading Segment File
	ImageReader->SetFileName(argv[2]);
    ImageReader->SetNumberOfScalarComponents(1);
	ImageReader->Update();

	std::cout<<argv[2]<<std::endl;

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
	std::string NumVol = "Single";
	
    //Setting Level of Extraction

	if (strcmp(argv[1], "Multi") == 0) {
		NumVol = "Multi";
	}
	else if (strcmp(argv[1], "Single") == 0) {
		NumVol = "Single";
	}
	else {
		std:cout<<"Incorrect number of meshes";
		return -1;
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
	


	//Image Threshold filter for creating Binary volume
	vtkSmartPointer<vtkImageThreshold> VolumeThreshold = vtkSmartPointer<vtkImageThreshold>::New();

	std::vector<std::string> line;
	while (getline(Labels, rawline)) {
		std::stringstream ss(rawline);
		std::cout<<rawline<<std::endl;
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
		VolumeThreshold->SetInputConnection(SegmentImageReader->GetOutputPort());
		VolumeThreshold->SetInValue(255);
		VolumeThreshold->SetOutValue(0);
		VolumeThreshold->ThresholdBetween(start, end);
		VolumeThreshold->Update();

		int* SegmentVolumeResolution = SegmentImageReader->GetOutput()->GetExtent();
        const int SegmentBufferSize = (SegmentVolumeResolution[1] + 1) * (SegmentVolumeResolution[3] + 1) * (SegmentVolumeResolution[5] + 1) * sizeof(short);
		short* SegmentBuffer = (short*)malloc(SegmentBufferSize);
		memcpy(SegmentBuffer, SegmentImageReader->GetOutput()->GetScalarPointer(), SegmentBufferSize);

		int* VolumeResolution = ImageReader->GetOutput()->GetExtent();
        const int BufferSize = (VolumeResolution[1] + 1) * (VolumeResolution[3] + 1) * (VolumeResolution[5] + 1) * sizeof(short);
		short* Buffer = (short*)malloc(BufferSize);
		memcpy(Buffer, ImageReader->GetOutput()->GetScalarPointer(), BufferSize);

		for (int i=0; i<SegmentBufferSize/sizeof(short); i++) {
			if (SegmentBuffer[i] != 0) {
				Buffer[i] = SegmentBuffer[i];
			}
			std::string Segment = std::bitset<16>(Buffer[i]).to_string();
			std::string L1 = Segment.substr(1, 4);
			std::string L2 = Segment.substr(5, 8);
			std::string NewLabel = L1+L2;
			Buffer[i] = (unsigned short) std::bitset<8>(NewLabel).to_ulong();
		}

		std::cout<<"Loop Complete\n";

		vtkSmartPointer<vtkUnsignedShortArray> usarray = vtkSmartPointer<vtkUnsignedShortArray>::New( );
		usarray->SetNumberOfComponents( 1 );
		usarray->SetArray( ( ushort* )( Buffer ), BufferSize, 1 );
	
		vtkSmartPointer<vtkImageData> ImageData = vtkSmartPointer<vtkImageData>::New();
		
		ImageData->SetDimensions(ImageReader->GetOutput()->GetDimensions());
		ImageData->SetSpacing(ImageReader->GetOutput()->GetSpacing());

		ImageData->GetPointData()->SetScalars(usarray);
		ImageData->Modified();
		
		usarray->Delete();
		

		vtkSmartPointer<vtkImageCast> ImageCast = vtkImageCast::New();
		ImageCast->SetInputData(ImageData);
		ImageCast->SetOutputScalarTypeToUnsignedShort();
		ImageCast->Update();

		if (NumVol == "Multi") {
			FinalizeVolume(ImageCast->GetOutput(), line[1]);
			line.clear();
			continue;
		}
		line.clear();
		if (NumVol == "Single") {
			std::cout<<"Finalizing Volume\n";
			FinalizeVolume(ImageCast->GetOutput(), argv[4]);
			break;
		}
    }
}

void FinalizeVolume(vtkSmartPointer<vtkImageData> Volume, std::string OutputFile) {
	std::string filename = OutputFile;
	std::cout<<filename<<std::endl;
	vtkSmartPointer<vtkMetaImageWriter> writer = vtkSmartPointer<vtkMetaImageWriter>::New();
	writer->SetInputData(Volume);
	writer->SetFileName(filename.c_str());
	writer->SetRAWFileName((filename.substr(0, filename.length()-3) + "raw").c_str());
	std::cout<<writer->GetRAWFileName()<<std::endl;
	writer->Update();
	writer->Write();
	std::cout<<"Volume Saved\n";
}