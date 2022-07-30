/*
	Copyright (c) 2011, T. Kroes <t.kroes@tudelft.nl>
	All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

	- Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
	- Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
	- Neither the name of the <ORGANIZATION> nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
	
	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Stable.h"

#include "RenderThread.h"
#include "CudaUtilities.h"
#include "MainWindow.h"
#include "LoadSettingsDialog.h"
#include "Lighting.h"
#include "Timing.h"
#include <iostream>

// CUDA kernels
#include "Core.cuh"

// VTK
#include <vtkSmartPointer.h>
#include <vtkMetaImageReader.h>
#include <vtkImageCast.h>
#include <vtkImageResample.h>
#include <vtkImageData.h>
#include <vtkImageGradientMagnitude.h>
#include <vtkCallbackCommand.h>
#include <vtkImageAccumulate.h>
#include <vtkIntArray.h>
#include <vtkImageShiftScale.h>
#include <vtkErrorCode.h>
#include <vtkImageGradient.h>
#include <vtkExtractVectorComponents.h>
#include <vtkImageExtractComponents.h>
#include <vtkImageAppendComponents.h>
#include <vtkImageReader2Factory.h>
#include <vtkImageRGBToHSI.h>

//for creating new window
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleImage.h>
#include <array>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkOutlineFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkCamera.h>
#include <vtkImageMarchingCubes.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkNamedColors.h>
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkImageGaussianSmooth.h>
#include <vtkImageThreshold.h>
#include <vtkNrrdReader.h>

// Render thread
QRenderThread* gpRenderThread = NULL;
QFrameBuffer gFrameBuffer;

QMutex gSceneMutex;

int gCurrentDeviceID = 0;

QFrameBuffer::QFrameBuffer(void) :
	m_pPixels(NULL),
	m_Width(0),
	m_Height(0),
	m_NoPixels(0),
	m_Mutex()
{
}

QFrameBuffer::QFrameBuffer(const QFrameBuffer& Other)
{
	*this = Other;
}

QFrameBuffer& QFrameBuffer::operator=(const QFrameBuffer& Other)
{
	const bool Dirty = m_Width != Other.m_Width || m_Height != Other.m_Height;

	m_Width		= Other.m_Width;
	m_Height	= Other.m_Height;
	m_NoPixels	= Other.m_NoPixels;

	if (Other.m_pPixels != NULL)
	{
		const int Size = 3 * m_NoPixels * sizeof(unsigned char);

		if (Dirty)
		{
			free(m_pPixels);
			m_pPixels = (unsigned char*)malloc(Size);
		}

		memcpy(m_pPixels, Other.m_pPixels, Size); 
	}
	else
	{
		m_pPixels = NULL;
	}

	return *this;
}

QFrameBuffer::~QFrameBuffer(void)
{
	free(m_pPixels);
}

void QFrameBuffer::Set(unsigned char* pPixels, const int& Width, const int& Height)
{
	const bool Dirty = Width != m_Width || Height != m_Height;

	m_Width		= Width;
	m_Height	= Height;
	m_NoPixels	= m_Width * m_Height;

	if (m_NoPixels <= 0)
		return;

	const int Size = 3 * m_NoPixels * sizeof(unsigned char);

	if (Dirty)
	{
		free(m_pPixels);
		m_pPixels = (unsigned char*)malloc(Size);
	}

	memcpy(m_pPixels, pPixels, Size); 
}

QRenderThread::QRenderThread(const QString& FileName, QObject* pParent /*= NULL*/) :
	QThread(pParent),
	m_FileName(FileName),
	m_pRenderImage(NULL),
	m_pDensityBuffer(NULL),
	m_pDensityBufferSeg(NULL),
	m_pDensityBufferRGBA(NULL),
	m_pGradientMagnitudeBuffer(NULL),
	m_Abort(false),
	m_Pause(false),
	m_SaveFrames(),
	m_SaveBaseName("phase_function")
{
//	m_SaveFrames << 0 << 100 << 200;
}

QRenderThread::QRenderThread(const QRenderThread& Other)
{
	*this = Other;
}

QRenderThread::~QRenderThread(void)
{
	if (!gScene.m_RGBA) {
		free(m_pDensityBuffer);
	}
	else {
	free(m_pDensityBufferSeg);
	free(m_pDensityBufferRGBA);
	}
}

QRenderThread& QRenderThread::operator=(const QRenderThread& Other)
{
	m_FileName					= Other.m_FileName;
	m_pRenderImage				= Other.m_pRenderImage;
	m_pDensityBuffer			= Other.m_pDensityBuffer;
	m_pDensityBufferSeg			= Other.m_pDensityBufferSeg;
	m_pDensityBufferRGBA		= Other.m_pDensityBufferRGBA;
	RGBAVolume					= Other.RGBAVolume;
	m_pGradientMagnitudeBuffer	= Other.m_pGradientMagnitudeBuffer;
	m_Abort						= Other.m_Abort;
	m_Pause						= Other.m_Pause;
	m_SaveFrames				= Other.m_SaveFrames;
	m_SaveBaseName				= Other.m_SaveBaseName;

	return *this;
}

void QRenderThread::run()
{
	if (!SetCudaDevice(gCurrentDeviceID))
		return;

	ResetDevice();

	CScene SceneCopy;
	
 	gScene.m_Camera.m_SceneBoundingBox = gScene.m_BoundingBox;
	gScene.m_Camera.SetViewMode(ViewModeFront);
 	gScene.m_Camera.Update();

	// Force the render thread to allocate the necessary buffers, do not remove this line
	gScene.m_DirtyFlags.SetFlag(FilmResolutionDirty | CameraDirty);

	gStatus.SetStatisticChanged("Memory", "CUDA Memory", "", "", "memory");
	gStatus.SetStatisticChanged("Memory", "Host Memory", "", "", "memory");

	cudaExtent Res;
	Res.width = gScene.m_Resolution[0];
	Res.height = gScene.m_Resolution[1];
	Res.depth = gScene.m_Resolution[2];

	cudaExtent ResRGB;
	ResRGB.width = gScene.m_ResolutionSegment[0];
	ResRGB.height = gScene.m_ResolutionSegment[1];
	ResRGB.depth = gScene.m_ResolutionSegment[2];

	// Bind density buffer to texture
	Log("Copying density volume to device", "grid");
	
	if (!RGBAVolume) {
		BindDensityBuffer((short*)m_pDensityBuffer, Res);
		gStatus.SetStatisticChanged("CUDA Memory", "Density Buffer", QString::number(gScene.m_Resolution.GetNoElements() * sizeof(short) / MB, 'f', 2), "MB");
	}
	else {
		BindDensityBufferRGBA((uchar4*)m_pDensityBufferRGBA, (short*)m_pDensityBufferSeg, Res, ResRGB);
		gStatus.SetStatisticChanged("CUDA Memory", "Density Buffer", QString::number(gScene.m_Resolution.GetNoElements() * sizeof(uchar4) / MB, 'f', 2), "MB");
	}

	// Bind gradient magnitude buffer to texture
	Log("Copying gradient magnitude to device", "grid");
	gStatus.SetStatisticChanged("CUDA Memory", "Gradient Magnitude Buffer", QString::number(gScene.m_Resolution.GetNoElements() * sizeof(short) / MB, 'f', 2), "MB");
	BindGradientMagnitudeBuffer((short*)m_pGradientMagnitudeBuffer, Res);

	gStatus.SetStatisticChanged("Performance", "Timings", "");
	gStatus.SetStatisticChanged("Camera", "", "");
	gStatus.SetStatisticChanged("CUDA Memory", "Scene", QString::number(sizeof(CScene) / MB, 'f', 2), "MB");
	gStatus.SetStatisticChanged("CUDA Memory", "Frame Buffers", "", "");

	// Let others know that we are starting with rendering
	gStatus.SetRenderBegin();
	
	Log("Device memory: " + QString::number(GetUsedCudaMemory() / MB, 'f', 2) + "/" + QString::number(GetTotalCudaMemory() / MB, 'f', 2) + " MB", "memory");

	QObject::connect(&gSelectiveOpacity, SIGNAL(Changed()), this, SLOT(OnUpdateSelectiveOpacity()));
	QObject::connect(&gTransferFunction, SIGNAL(Changed()), this, SLOT(OnUpdateTransferFunction()));
	QObject::connect(&gCamera, SIGNAL(Changed()), this, SLOT(OnUpdateCamera()));
	QObject::connect(&gLighting, SIGNAL(Changed()), this, SLOT(OnUpdateLighting()));
	QObject::connect(&gLighting.Background(), SIGNAL(Changed()), this, SLOT(OnUpdateLighting()));

	QObject::connect(&gStatus, SIGNAL(RenderPause(const bool&)), this, SLOT(OnRenderPause(const bool&)));

	// Try to load appearance/lighting/camera presets with the same name as the loaded file
	gStatus.SetLoadPreset(QFileInfo(m_FileName).baseName());

	// Keep track of frames/second
	CTiming FPS, RenderImage, BlurImage, PostProcessImage, DenoiseImage;

	ResetRenderCanvasView();


	// const int OpacityBufferSize = gSelectiveOpacity.GetNumSegments() * sizeof(float);
	// m_pOpacityBuffer = (float*)malloc(OpacityBufferSize);
	// memcpy(m_pOpacityBuffer, gSelectiveOpacity.GetOpacityArray(), OpacityBufferSize);

	try
	{
		while (!m_Abort)
		{
			if (m_Pause)
				continue;

			gStatus.SetPreRenderFrame();

			// CUDA time for profiling
 			CCudaTimer TmrFps;

			SceneCopy = gScene;

			gStatus.SetStatisticChanged("Camera", "Position", FormatVector(SceneCopy.m_Camera.m_From));
			gStatus.SetStatisticChanged("Camera", "Target", FormatVector(SceneCopy.m_Camera.m_Target));
			gStatus.SetStatisticChanged("Camera", "Up Vector", FormatVector(SceneCopy.m_Camera.m_Up));

			// Resizing the image canvas requires special attention
			if (SceneCopy.m_DirtyFlags.HasFlag(FilmResolutionDirty))
			{
				// Allocate host image buffer, this thread will blit it's frames to this buffer
				free(m_pRenderImage);
				m_pRenderImage = NULL;

				m_pRenderImage = (CColorRgbLdr*)malloc(SceneCopy.m_Camera.m_Film.m_Resolution.GetNoElements() * sizeof(CColorRgbLdr));

				if (m_pRenderImage)
					memset(m_pRenderImage, 0, SceneCopy.m_Camera.m_Film.m_Resolution.GetNoElements() * sizeof(CColorRgbLdr));
			
				gStatus.SetStatisticChanged("Host Memory", "LDR Frame Buffer", QString::number(3 * SceneCopy.m_Camera.m_Film.m_Resolution.GetNoElements() * sizeof(CColorRgbLdr) / MB, 'f', 2), "MB");

				SceneCopy.SetNoIterations(0);

				Log("Render canvas resized to: " + QString::number(SceneCopy.m_Camera.m_Film.m_Resolution.GetResX()) + " x " + QString::number(SceneCopy.m_Camera.m_Film.m_Resolution.GetResY()) + " pixels", "application-resize");
			}

			// Restart the rendering when when the camera, lights and render params are dirty
			if (SceneCopy.m_DirtyFlags.HasFlag(CameraDirty | LightsDirty | RenderParamsDirty | TransferFunctionDirty))
			{
				ResetRenderCanvasView();

				// Reset no. iterations
				gScene.SetNoIterations(0);
			}

			// At this point, all dirty flags should have been taken care of, since the flags in the original scene are now cleared
			gScene.m_DirtyFlags.ClearAllFlags();

			SceneCopy.m_DenoiseParams.SetWindowRadius(3.0f);
			SceneCopy.m_DenoiseParams.m_LerpC = 0.33f * (max((float)gScene.GetNoIterations(), 1.0f) * 0.035f);//1.0f - powf(1.0f / (float)gScene.GetNoIterations(), 15.0f);//1.0f - expf(-0.01f * (float)gScene.GetNoIterations());
//			SceneCopy.m_DenoiseParams.m_Enabled = false;

			SceneCopy.m_Camera.Update();

			BindConstants(&SceneCopy);

			BindTextureSelectiveOpacity(SceneCopy.m_SelectiveOpacity.OpacityBuffer, SceneCopy.m_SelectiveOpacity.Size);
			BindTransferFunctionOpacity(SceneCopy.m_TransferFunctions.m_Opacity);
			BindTransferFunctionDiffuse(SceneCopy.m_TransferFunctions.m_Diffuse);
			BindTransferFunctionSpecular(SceneCopy.m_TransferFunctions.m_Specular);
			BindTransferFunctionRoughness(SceneCopy.m_TransferFunctions.m_Roughness);
			BindTransferFunctionEmission(SceneCopy.m_TransferFunctions.m_Emission);

			BindRenderCanvasView(SceneCopy.m_Camera.m_Film.m_Resolution);

  			Render(0, SceneCopy, RenderImage, BlurImage, PostProcessImage, DenoiseImage);
		
			gScene.SetNoIterations(gScene.GetNoIterations() + 1);

			gStatus.SetStatisticChanged("Timings", "Render Image", QString::number(RenderImage.m_FilteredDuration, 'f', 2), "ms.");
			gStatus.SetStatisticChanged("Timings", "Blur Estimate", QString::number(BlurImage.m_FilteredDuration, 'f', 2), "ms.");
			gStatus.SetStatisticChanged("Timings", "Post Process Estimate", QString::number(PostProcessImage.m_FilteredDuration, 'f', 2), "ms.");
			gStatus.SetStatisticChanged("Timings", "De-noise Image", QString::number(DenoiseImage.m_FilteredDuration, 'f', 2), "ms.");

			FPS.AddDuration(1000.0f / TmrFps.ElapsedTime());

 			gStatus.SetStatisticChanged("Performance", "FPS", QString::number(FPS.m_FilteredDuration, 'f', 2), "Frames/Sec.");
 			gStatus.SetStatisticChanged("Performance", "No. Iterations", QString::number(SceneCopy.GetNoIterations()), "Iterations");

			HandleCudaError(cudaMemcpy(m_pRenderImage, GetDisplayEstimate(), SceneCopy.m_Camera.m_Film.m_Resolution.GetNoElements() * sizeof(CColorRgbLdr), cudaMemcpyDeviceToHost));

			gFrameBuffer.Set((unsigned char*)m_pRenderImage, SceneCopy.m_Camera.m_Film.GetWidth(), SceneCopy.m_Camera.m_Film.GetHeight());

			if (m_SaveFrames.indexOf(SceneCopy.GetNoIterations()) > 0)
			{
				const QString ImageFilePath = QApplication::applicationDirPath() + "/Output/" + m_SaveBaseName + "_" + QString::number(SceneCopy.GetNoIterations()) + ".png";

				SaveImage((unsigned char*)m_pRenderImage, SceneCopy.m_Camera.m_Film.m_Resolution.GetResX(), SceneCopy.m_Camera.m_Film.m_Resolution.GetResY(), ImageFilePath);
			}

 			gStatus.SetPostRenderFrame();
		}
	}
	catch (QString* pMessage)
	{
//		Log(*pMessage + ", rendering will be aborted");

		free(m_pRenderImage);
		m_pRenderImage = NULL;

		gStatus.SetRenderEnd();

		return;
	}

	free(m_pRenderImage);
	m_pRenderImage = NULL;

	if (!gScene.m_RGBA) {
		UnbindDensityBuffer();
	}
	else {
		UnbindDensityBufferRGBA();
	}

	UnbindTextureSelectiveOpacity();
	UnbindTransferFunctionOpacity();
	UnbindTransferFunctionDiffuse();
	UnbindTransferFunctionSpecular();
	UnbindTransferFunctionRoughness();
	UnbindTransferFunctionEmission();
	
	FreeRenderCanvasView();

	// Let others know that we have stopped rendering
	gStatus.SetRenderEnd();

	Log("Device memory: " + QString::number(GetUsedCudaMemory() / MB, 'f', 2) + "/" + QString::number(GetTotalCudaMemory() / MB, 'f', 2) + " MB", "memory");

	// Clear the histogram
	gHistogram.Reset();

	ResetDevice();
}

bool QRenderThread::Load(QString& FileName)
{
	m_FileName = FileName;

	// Create meta image reader
	vtkSmartPointer<vtkMetaImageReader> MetaImageReader = vtkMetaImageReader::New();

	QFileInfo FileInfo(FileName);

	if (!FileInfo.exists())
	{
		Log(QString(QFileInfo(FileName).filePath().replace("//", "/")).toLatin1() + "  does not exist!", QLogger::Critical);
		return false;
	}

	Log(QString("Loading " + QFileInfo(FileName).fileName()).toLatin1());

	// Exit if the reader can't read the file
	if (!MetaImageReader->CanReadFile(m_FileName.toLatin1()))
	{
		Log(QString("Meta image reader can't read file " + QFileInfo(FileName).fileName()).toLatin1(), QLogger::Critical);
		return false;
	}

	MetaImageReader->SetFileName(m_FileName.toLatin1());

	MetaImageReader->Update();

	if (MetaImageReader->GetErrorCode() != vtkErrorCode::NoError)
	{
		Log("Error loading file " + QString(vtkErrorCode::GetStringFromErrorCode(MetaImageReader->GetErrorCode())));
		return false;
	}

	vtkSmartPointer<vtkImageCast> ImageCast = vtkImageCast::New();
	
	Log("Casting volume data type to short", "grid");

	ImageCast->SetInputConnection(MetaImageReader->GetOutputPort());
	ImageCast->SetOutputScalarTypeToShort();
	ImageCast->Update();

	if (ImageCast->GetErrorCode() != vtkErrorCode::NoError)
	{
		Log("vtkImageCast error: " + QString(vtkErrorCode::GetStringFromErrorCode(MetaImageReader->GetErrorCode())));
		return false;
	}
	
	// Volume resolution
	int* pVolumeResolution = ImageCast->GetOutput()->GetExtent();
	gScene.m_Resolution.SetResXYZ(Vec3i(pVolumeResolution[1] + 1, pVolumeResolution[3] + 1, pVolumeResolution[5] + 1));

	Log("Resolution: " + FormatSize(gScene.m_Resolution.GetResXYZ()) + "", "grid");
	gScene.m_RGBA = false;

	// Intensity range
	double* pIntensityRange = ImageCast->GetOutput()->GetScalarRange();
	gScene.m_IntensityRange.SetMin((float)pIntensityRange[0]);
	gScene.m_IntensityRange.SetMax((float)pIntensityRange[1]);

	Log("Intensity range: [" + QString::number(gScene.m_IntensityRange.GetMin()) + ", " + QString::number(gScene.m_IntensityRange.GetMax()) + "]", "grid");

	// Spacing
	double* pSpacing = ImageCast->GetOutput()->GetSpacing();

	gScene.m_Spacing.x = (float)pSpacing[0];
	gScene.m_Spacing.y = (float)pSpacing[1];
	gScene.m_Spacing.z = (float)pSpacing[2];

	Log("Spacing: " + FormatSize(gScene.m_Spacing, 2), "grid");

	// Compute physical size
	const Vec3f PhysicalSize(Vec3f(gScene.m_Spacing.x * (float)gScene.m_Resolution.GetResX(), gScene.m_Spacing.y * (float)gScene.m_Resolution.GetResY(), gScene.m_Spacing.z * (float)gScene.m_Resolution.GetResZ()));

	// Compute the volume's bounding box
	gScene.m_BoundingBox.m_MinP	= Vec3f(0.0f);
	gScene.m_BoundingBox.m_MaxP	= PhysicalSize / PhysicalSize.Max();

	gScene.m_GradientDelta = 1.0f / (float)gScene.m_Resolution.GetMax();
	
	Log("Bounding box: " + FormatVector(gScene.m_BoundingBox.m_MinP, 2) + " - " + FormatVector(gScene.m_BoundingBox.m_MaxP), "grid");
	
	const int DensityBufferSize = gScene.m_Resolution.GetNoElements() * sizeof(short);

 	m_pDensityBuffer = (short*)malloc(DensityBufferSize);
  	memcpy(m_pDensityBuffer, ImageCast->GetOutput()->GetScalarPointer(), DensityBufferSize);

	// Gradient magnitude volume
	vtkSmartPointer<vtkImageGradientMagnitude> GradientMagnitude = vtkImageGradientMagnitude::New();
	
	Log("Creating gradient magnitude volume", "grid");
		
	GradientMagnitude->SetDimensionality(3);
	GradientMagnitude->SetInputConnection(ImageCast->GetOutputPort());
	GradientMagnitude->Update();

	vtkImageData* GradientMagnitudeBuffer = GradientMagnitude->GetOutput();
	
	// Scalar range of the gradient magnitude
	double* pGradientMagnitudeRange = GradientMagnitudeBuffer->GetScalarRange();
	
	gScene.m_GradientMagnitudeRange.SetMin((float)pGradientMagnitudeRange[0]);
	gScene.m_GradientMagnitudeRange.SetMax((float)pGradientMagnitudeRange[1]);
	
	Log("Gradient magnitude range: [" + QString::number(gScene.m_GradientMagnitudeRange.GetMin(), 'f', 2) + " - " + QString::number(gScene.m_GradientMagnitudeRange.GetMax(), 'f', 2) + "]", "grid");
	
	const int GradientMagnitudeBufferSize = gScene.m_Resolution.GetNoElements() * sizeof(short);
	
	m_pGradientMagnitudeBuffer = (short*)malloc(GradientMagnitudeBufferSize);
	memcpy(m_pGradientMagnitudeBuffer, GradientMagnitudeBuffer->GetScalarPointer(), GradientMagnitudeBufferSize);

	// Build the histogram
	Log("Creating gradient magnitude histogram", "grid");

	vtkSmartPointer<vtkImageAccumulate> GradMagHistogram = vtkSmartPointer<vtkImageAccumulate>::New();

	GradMagHistogram->SetInputConnection(GradientMagnitude->GetOutputPort());
	GradMagHistogram->SetComponentExtent(0, 255, 0, 0, 0, 0);
	GradMagHistogram->SetComponentOrigin(0, 0, 0);
	GradMagHistogram->SetComponentSpacing(gScene.m_GradientMagnitudeRange.GetRange() / 256.0f, 0, 0);
//	GradMagHistogram->IgnoreZeroOn();
	GradMagHistogram->Update();

	gScene.m_GradMagMean = (float)GradMagHistogram->GetMean()[0];
	gScene.m_GradientFactor = gScene.m_GradMagMean;

	Log("Mean gradient magnitude: " + QString::number(gScene.m_GradMagMean, 'f', 2), "grid");

	Log("Creating density histogram", "grid");

	// Build the histogram
	vtkSmartPointer<vtkImageAccumulate> Histogram = vtkSmartPointer<vtkImageAccumulate>::New();

	Log("Creating histogram", "grid");

 	Histogram->SetInputConnection(ImageCast->GetOutputPort());
 	Histogram->SetComponentExtent(0, 256, 0, 0, 0, 0);
 	Histogram->SetComponentOrigin(gScene.m_IntensityRange.GetMin(), 0, 0);
 	Histogram->SetComponentSpacing(gScene.m_IntensityRange.GetRange() / 256.0f, 0, 0);
 	Histogram->IgnoreZeroOn();
 	Histogram->Update();
 
	// Update the histogram in the transfer function
	gHistogram.SetBins((int*)Histogram->GetOutput()->GetScalarPointer(), 256);
	
	gStatus.SetStatisticChanged("Volume", "File", QFileInfo(m_FileName).fileName(), "");
	gStatus.SetStatisticChanged("Volume", "Bounding Box", "", "");
	gStatus.SetStatisticChanged("Bounding Box", "Min", FormatVector(gScene.m_BoundingBox.m_MinP, 2), "m");
	gStatus.SetStatisticChanged("Bounding Box", "Max", FormatVector(gScene.m_BoundingBox.m_MaxP, 2), "m");
	gStatus.SetStatisticChanged("Volume", "Physical Size", FormatSize(PhysicalSize, 2), "mm");
	gStatus.SetStatisticChanged("Volume", "Resolution", FormatSize(gScene.m_Resolution.GetResXYZ()), "Voxels");
	gStatus.SetStatisticChanged("Volume", "Spacing", FormatSize(gScene.m_Spacing, 2), "mm");
	gStatus.SetStatisticChanged("Volume", "No. Voxels", QString::number(gScene.m_Resolution.GetNoElements()), "Voxels");
	gStatus.SetStatisticChanged("Volume", "Density Range", "[" + QString::number(gScene.m_IntensityRange.GetMin()) + ", " + QString::number(gScene.m_IntensityRange.GetMax()) + "]", "");
	
	return true;
}

bool QRenderThread::LoadRGBA(QString& FileName)
{
	m_FileName = QString(QFileInfo(FileName).filePath().replace("\\\\", "/"));

	RGBAVolume = true;

	//checking for Pose Trace file

	size_t lastindex = m_FileName.toStdString().find_last_of(".");
	string rawname = m_FileName.toStdString().substr(0, lastindex);

	string PoseTraceFile = rawname + "_PoseTrace.txt";
	gCamera.LoadCameraPoses(PoseTraceFile);
	
	//Segment Files
	string SegmentFile = rawname + "_Segments.mhd";
	string SegmentFileSkin = rawname + "_Segments_BG.mhd";

	vtkSmartPointer<vtkMetaImageReader> reader = vtkSmartPointer<vtkMetaImageReader>::New();
	vtkSmartPointer<vtkMetaImageReader> readerSeg = vtkSmartPointer<vtkMetaImageReader>::New();

	if (!reader->CanReadFile(QString::fromStdString(SegmentFileSkin).toLatin1()) || !readerSeg->CanReadFile(QString::fromStdString(SegmentFile).toLatin1()))
	{
		std::cout << "Cannot read Segment Volume\n";
		gScene.m_SegmentAvailable = false;
	}
	else {
		gScene.m_SegmentAvailable = true;
	}

	//Loading Segment Volumes
	if (gScene.m_SegmentAvailable) {
		reader->SetFileName(QString::fromStdString(SegmentFileSkin).toLatin1());
		reader->SetNumberOfScalarComponents(1);
		reader->SetDataScalarTypeToUnsignedChar();
		reader->Update();

		readerSeg->SetFileName(QString::fromStdString(SegmentFile).toLatin1());
		readerSeg->SetNumberOfScalarComponents(1);
		readerSeg->SetDataScalarTypeToUnsignedShort();
		readerSeg->Update();

		if (reader->GetErrorCode() != vtkErrorCode::NoError)
		{
			std::cout<<"Error loading file "<<QString(vtkErrorCode::GetStringFromErrorCode(reader->GetErrorCode())).toStdString()<<"\n";
		}
	}

	// Create meta image reader
	vtkSmartPointer<vtkMetaImageReader> MetaImageReader = vtkSmartPointer<vtkMetaImageReader>::New();

	QFileInfo FileInfo(QString(QFileInfo(FileName).filePath().replace("\\\\", "/")));

	if (!FileInfo.exists())
	{
		Log(QString(QFileInfo(FileName).filePath().replace("//", "/")).toLatin1() + "  does not exist!", QLogger::Critical);
		return false;
	}

	Log(QString("Loading " + QFileInfo(FileName).fileName()).toLatin1());

	// Exit if the reader can't read the file
	if (!MetaImageReader->CanReadFile(m_FileName.toLatin1()))
	{
		Log(QString("Meta image reader can't read file " + QFileInfo(FileName).fileName()).toLatin1(), QLogger::Critical);
		return false;
	}

	//Loading RGB Volume

	MetaImageReader->SetFileName(m_FileName.toLatin1());
	MetaImageReader->SetNumberOfScalarComponents(3);
	MetaImageReader->SetDataScalarTypeToUnsignedChar();

	MetaImageReader->Update();

	if (MetaImageReader->GetErrorCode() != vtkErrorCode::NoError)
	{
		Log("Error loading file " + QString(vtkErrorCode::GetStringFromErrorCode(MetaImageReader->GetErrorCode())));
		return false;
	}

	vtkSmartPointer<vtkImageCast> ImageCast = vtkSmartPointer<vtkImageCast>::New();
	
	Log("Casting volume data type to short", "grid");

	ImageCast->SetInputConnection(MetaImageReader->GetOutputPort());
	ImageCast->SetOutputScalarTypeToShort();
	ImageCast->Update();
	if (ImageCast->GetErrorCode() != vtkErrorCode::NoError)
	{
		Log("vtkImageCast error: " + QString(vtkErrorCode::GetStringFromErrorCode(MetaImageReader->GetErrorCode())));
		return false;
	}

	//Appending Skin Segment to RGB Volume to form a uchar4 volume
	vtkSmartPointer<vtkImageAppendComponents> appendRGBA = vtkSmartPointer<vtkImageAppendComponents>::New();
	appendRGBA->SetInputConnection(MetaImageReader->GetOutputPort());
	appendRGBA->AddInputConnection(reader->GetOutputPort());
	appendRGBA->Update();

	// Volume resolution
	int* pVolumeResolution = appendRGBA->GetOutput()->GetExtent();
	gScene.m_Resolution.SetResXYZ(Vec3i(pVolumeResolution[1] + 1, pVolumeResolution[3] + 1, pVolumeResolution[5] + 1));
	gCamera.SetResolution(Vec3f((float)gScene.m_Resolution.GetResX()*4.0f, (float)gScene.m_Resolution.GetResY(), (float)gScene.m_Resolution.GetResZ()*4.0f));
	Log("Resolution: " + FormatSize(gScene.m_Resolution.GetResXYZ()) + "", "grid");
	gScene.m_RGBA = true;

	const long DensityBufferSize = gScene.m_Resolution.GetNoElements() * sizeof(uchar4);
 	m_pDensityBufferRGBA = (uchar4*)malloc(DensityBufferSize);
	memcpy(m_pDensityBufferRGBA, appendRGBA->GetOutput()->GetScalarPointer(), DensityBufferSize);

	// Intensity range
	double* pIntensityRange = MetaImageReader->GetOutput()->GetScalarRange();
	gScene.m_IntensityRange.SetMin((float)pIntensityRange[0]);
	gScene.m_IntensityRange.SetMax((float)pIntensityRange[1]);

	Log("Intensity range: [" + QString::number(gScene.m_IntensityRange.GetMin()) + ", " + QString::number(gScene.m_IntensityRange.GetMax()) + "]", "grid");
	// Spacing
	double* pSpacing = MetaImageReader->GetOutput()->GetSpacing();

	gScene.m_Spacing.x = (float)pSpacing[0];
	gScene.m_Spacing.y = (float)pSpacing[1];
	gScene.m_Spacing.z = (float)pSpacing[2];

	Log("Spacing: " + FormatSize(gScene.m_Spacing, 2), "grid");

	// Compute physical size
	const Vec3f PhysicalSize(Vec3f(gScene.m_Spacing.x * (float)gScene.m_Resolution.GetResX(), gScene.m_Spacing.y * (float)gScene.m_Resolution.GetResY(), gScene.m_Spacing.z * (float)gScene.m_Resolution.GetResZ()));
	// Compute the volume's bounding box
	gScene.m_BoundingBox.m_MinP	= Vec3f(0.0f);
	gScene.m_BoundingBox.m_MaxP	= PhysicalSize / PhysicalSize.Max();
	gScene.m_GradientDelta = 1.0f / (float)gScene.m_Resolution.GetMax();

	Log("Bounding box: " + FormatVector(gScene.m_BoundingBox.m_MinP, 2) + " - " + FormatVector(gScene.m_BoundingBox.m_MaxP), "grid");

	vtkSmartPointer<vtkImageRGBToHSI> HSIVolume = vtkSmartPointer<vtkImageRGBToHSI>::New();
	HSIVolume->SetInputConnection(ImageCast->GetOutputPort());
	HSIVolume->Update();

	vtkSmartPointer<vtkImageExtractComponents> IntensityVolume = vtkSmartPointer<vtkImageExtractComponents>::New();
	IntensityVolume->SetInputConnection(HSIVolume->GetOutputPort());
	IntensityVolume->SetComponents(2);
	IntensityVolume->Update();

	//Setting up Segment Volume
	if (gScene.m_SegmentAvailable) {	

		int* pVolumeResolutionSegment = readerSeg->GetOutput()->GetExtent();
		gScene.m_ResolutionSegment.SetResXYZ(Vec3i(pVolumeResolutionSegment[1] + 1, pVolumeResolutionSegment[3] + 1, pVolumeResolutionSegment[5] + 1));

		const long DensityBufferSizeRGB = gScene.m_ResolutionSegment.GetNoElements() * sizeof(short);
		m_pDensityBufferSeg = (short*)malloc(DensityBufferSizeRGB);
		memcpy(m_pDensityBufferSeg, readerSeg->GetOutput()->GetScalarPointer(), DensityBufferSizeRGB);
	}
	else {
		int* pVolumeResolutionSegment = MetaImageReader->GetOutput()->GetExtent();
		gScene.m_ResolutionSegment.SetResXYZ(Vec3i(pVolumeResolutionSegment[1] + 1, pVolumeResolutionSegment[3] + 1, pVolumeResolutionSegment[5] + 1));

		const long DensityBufferSizeRGB = gScene.m_ResolutionSegment.GetNoElements() * sizeof(short);
		m_pDensityBufferSeg = (short*)malloc(DensityBufferSizeRGB);
		memcpy(m_pDensityBufferSeg, MetaImageReader->GetOutput()->GetScalarPointer(), DensityBufferSizeRGB);
	}

	// Gradient magnitude volume
	vtkSmartPointer<vtkImageGradientMagnitude> GradientMagnitude = vtkSmartPointer<vtkImageGradientMagnitude>::New();
	
	Log("Creating gradient magnitude volume", "grid");
		
	GradientMagnitude->SetDimensionality(3);
	GradientMagnitude->SetInputConnection(ImageCast->GetOutputPort());
	GradientMagnitude->Update();

	vtkImageData* GradientMagnitudeBuffer = GradientMagnitude->GetOutput();
	// Scalar range of the gradient magnitude
	double* pGradientMagnitudeRange = GradientMagnitudeBuffer->GetScalarRange();

	gScene.m_GradientMagnitudeRange.SetMin((float)pGradientMagnitudeRange[0]);
	gScene.m_GradientMagnitudeRange.SetMax((float)pGradientMagnitudeRange[1]);
	
	Log("Gradient magnitude range: [" + QString::number(gScene.m_GradientMagnitudeRange.GetMin(), 'f', 2) + " - " + QString::number(gScene.m_GradientMagnitudeRange.GetMax(), 'f', 2) + "]", "grid");
	
	const long GradientMagnitudeBufferSize = gScene.m_Resolution.GetNoElements() * sizeof(short);
	
	m_pGradientMagnitudeBuffer = (short*)malloc(GradientMagnitudeBufferSize);
	memcpy(m_pGradientMagnitudeBuffer, GradientMagnitudeBuffer->GetScalarPointer(), GradientMagnitudeBufferSize);
	// Build the histogram
	Log("Creating gradient magnitude histogram", "grid");

	vtkSmartPointer<vtkImageAccumulate> GradMagHistogram = vtkSmartPointer<vtkImageAccumulate>::New();

	GradMagHistogram->SetInputConnection(GradientMagnitude->GetOutputPort());
	GradMagHistogram->SetComponentExtent(0, 255, 0, 0, 0, 0);
	GradMagHistogram->SetComponentOrigin(0, 0, 0);
	GradMagHistogram->SetComponentSpacing(gScene.m_GradientMagnitudeRange.GetRange() / 256.0f, 0, 0);
	GradMagHistogram->Update();

	gScene.m_GradMagMean = (float)GradMagHistogram->GetMean()[0];
	gScene.m_GradientFactor = gScene.m_GradMagMean;

	Log("Mean gradient magnitude: " + QString::number(gScene.m_GradMagMean, 'f', 2), "grid");

	Log("Creating density histogram", "grid");
	// Build the histogram
	vtkSmartPointer<vtkImageAccumulate> Histogram = vtkSmartPointer<vtkImageAccumulate>::New();

	Log("Creating histogram", "grid");

 	Histogram->SetInputConnection(IntensityVolume->GetOutputPort());
 	Histogram->SetComponentExtent(0, 256, 0, 0, 0, 0);
 	Histogram->SetComponentOrigin(gScene.m_IntensityRange.GetMin(), 0, 0);
 	Histogram->SetComponentSpacing(gScene.m_IntensityRange.GetRange() / 256.0f, 0, 0);
 	Histogram->IgnoreZeroOn();
 	Histogram->Update();
 
	// Update the histogram in the transfer function
	gHistogram.SetBins((int*)Histogram->GetOutput()->GetScalarPointer(), 256);
	gStatus.SetStatisticChanged("Volume", "File", QFileInfo(m_FileName).fileName(), "");
	gStatus.SetStatisticChanged("Volume", "Bounding Box", "", "");
	gStatus.SetStatisticChanged("Bounding Box", "Min", FormatVector(gScene.m_BoundingBox.m_MinP, 2), "m");
	gStatus.SetStatisticChanged("Bounding Box", "Max", FormatVector(gScene.m_BoundingBox.m_MaxP, 2), "m");
	gStatus.SetStatisticChanged("Volume", "Physical Size", FormatSize(PhysicalSize, 2), "mm");
	gStatus.SetStatisticChanged("Volume", "Resolution", FormatSize(gScene.m_Resolution.GetResXYZ()), "Voxels");
	gStatus.SetStatisticChanged("Volume", "Spacing", FormatSize(gScene.m_Spacing, 2), "mm");
	gStatus.SetStatisticChanged("Volume", "No. Voxels", QString::number(gScene.m_Resolution.GetNoElements()), "Voxels");
	gStatus.SetStatisticChanged("Volume", "Density Range", "[" + QString::number(gScene.m_IntensityRange.GetMin()) + ", " + QString::number(gScene.m_IntensityRange.GetMax()) + "]", "");

	return true;
}

void QRenderThread::OnUpdateSelectiveOpacity(void) {
	QMutexLocker Locker(&gSceneMutex);

	QSelectiveOpacity SelectiveOpacity;
	SelectiveOpacity.SetOpacityBuffer(gSelectiveOpacity.GetOpacityBuffer());
	gScene.m_SelectiveOpacity.SetSize(gSelectiveOpacity.GetSize());
	gScene.m_SelectiveOpacity.SetOpacityBuffer(SelectiveOpacity.GetOpacityBuffer());

	//gScene.m_SelectiveOpacity.printACK();

	gScene.m_DirtyFlags.SetFlag(TransferFunctionDirty);
}

void QRenderThread::OnUpdateTransferFunction(void)
{
	QMutexLocker Locker(&gSceneMutex);

	QTransferFunction TransferFunction = gTransferFunction;

	gScene.m_TransferFunctions.m_Opacity.m_NoNodes		= TransferFunction.GetNodes().size();
	gScene.m_TransferFunctions.m_Diffuse.m_NoNodes		= TransferFunction.GetNodes().size();
	gScene.m_TransferFunctions.m_Specular.m_NoNodes		= TransferFunction.GetNodes().size();
	gScene.m_TransferFunctions.m_Emission.m_NoNodes		= TransferFunction.GetNodes().size();
	gScene.m_TransferFunctions.m_Roughness.m_NoNodes	= TransferFunction.GetNodes().size();

	for (int i = 0; i < TransferFunction.GetNodes().size(); i++)
	{
		QNode& Node = TransferFunction.GetNode(i);

		const float Intensity = Node.GetIntensity();

		// Positions
		gScene.m_TransferFunctions.m_Opacity.m_P[i]		= Intensity;
		gScene.m_TransferFunctions.m_Diffuse.m_P[i]		= Intensity;
		gScene.m_TransferFunctions.m_Specular.m_P[i]	= Intensity;
		gScene.m_TransferFunctions.m_Emission.m_P[i]	= Intensity;
		gScene.m_TransferFunctions.m_Roughness.m_P[i]	= Intensity;

		// Colors
		gScene.m_TransferFunctions.m_Opacity.m_C[i]		= CColorRgbHdr(Node.GetOpacity());
		gScene.m_TransferFunctions.m_Diffuse.m_C[i]		= CColorRgbHdr(Node.GetDiffuse().redF(), Node.GetDiffuse().greenF(), Node.GetDiffuse().blueF());
		gScene.m_TransferFunctions.m_Specular.m_C[i]	= CColorRgbHdr(Node.GetSpecular().redF(), Node.GetSpecular().greenF(), Node.GetSpecular().blueF());
		gScene.m_TransferFunctions.m_Emission.m_C[i]	= 500.0f * CColorRgbHdr(Node.GetEmission().redF(), Node.GetEmission().greenF(), Node.GetEmission().blueF());

		const float Roughness = 1.0f - expf(-Node.GetGlossiness());

		gScene.m_TransferFunctions.m_Roughness.m_C[i] = CColorRgbHdr(Roughness * 250.0f);
	}

	gScene.m_DensityScale	= TransferFunction.GetDensityScale();
	gScene.m_ShadingType	= TransferFunction.GetShadingType();
	gScene.m_GradientFactor	= TransferFunction.GetGradientFactor();

	gScene.m_DirtyFlags.SetFlag(TransferFunctionDirty);

	/*
	FILE * pFile;
	int n;
	char name [100];

	pFile = fopen ("c:\\tf.txt","w");

	if (pFile)
	{
		for (int i = 0; i < 255; i++)
		{
			fprintf(pFile, "%0.2f\n", gScene.m_TransferFunctions.m_Roughness.F((float)i / 255.0f));
		}
	}

	fclose (pFile);
	*/
}

void QRenderThread::OnUpdateCamera(void)
{
	QMutexLocker Locker(&gSceneMutex);

	gScene.m_Camera.m_Film.m_Exposure = 1.0f - gCamera.GetFilm().GetExposure();

	if (gCamera.GetFilm().IsDirty())
	{
		const int FilmWidth	= gCamera.GetFilm().GetWidth();
		const int FilmHeight = gCamera.GetFilm().GetHeight();

 		gScene.m_Camera.m_Film.m_Resolution.SetResX(FilmWidth);
		gScene.m_Camera.m_Film.m_Resolution.SetResY(FilmHeight);
		gScene.m_Camera.Update();
		gCamera.GetFilm().UnDirty();
// 		// 
 		gScene.m_DirtyFlags.SetFlag(FilmResolutionDirty);
	}

	gScene.m_Camera.m_From	= gCamera.GetFrom();
 	gScene.m_Camera.m_Target	= gCamera.GetTarget();
 	gScene.m_Camera.m_Up		= gCamera.GetUp();

	gScene.m_Camera.Update();

	// Aperture
	gScene.m_Camera.m_Aperture.m_Size	= gCamera.GetAperture().GetSize();

	// Projection
	gScene.m_Camera.m_FovV = gCamera.GetProjection().GetFieldOfView();

	// Focus
	gScene.m_Camera.m_Focus.m_Type			= (CFocus::EType)gCamera.GetFocus().GetType();
	gScene.m_Camera.m_Focus.m_FocalDistance = gCamera.GetFocus().GetFocalDistance();

	gScene.m_DenoiseParams.m_Enabled = gCamera.GetFilm().GetNoiseReduction();

	gScene.m_DirtyFlags.SetFlag(CameraDirty);
}

void QRenderThread::OnUpdateLighting(void)
{
	QMutexLocker Locker(&gSceneMutex);

	gScene.m_Lighting.Reset();

	if (gLighting.Background().GetEnabled())
	{
		CLight BackgroundLight;

		BackgroundLight.m_T	= 1;

		BackgroundLight.m_ColorTop		= gLighting.Background().GetIntensity() * CColorRgbHdr(gLighting.Background().GetTopColor().redF(), gLighting.Background().GetTopColor().greenF(), gLighting.Background().GetTopColor().blueF());
		BackgroundLight.m_ColorMiddle	= gLighting.Background().GetIntensity() * CColorRgbHdr(gLighting.Background().GetMiddleColor().redF(), gLighting.Background().GetMiddleColor().greenF(), gLighting.Background().GetMiddleColor().blueF());
		BackgroundLight.m_ColorBottom	= gLighting.Background().GetIntensity() * CColorRgbHdr(gLighting.Background().GetBottomColor().redF(), gLighting.Background().GetBottomColor().greenF(), gLighting.Background().GetBottomColor().blueF());
		
		BackgroundLight.Update(gScene.m_BoundingBox);

		gScene.m_Lighting.AddLight(BackgroundLight);
	}

	for (int i = 0; i < gLighting.GetLights().size(); i++)
	{
		QLight& Light = gLighting.GetLights()[i];

		CLight AreaLight;

		AreaLight.m_T			= 0;
		AreaLight.m_Theta		= Light.GetTheta() / RAD_F;
		AreaLight.m_Phi			= Light.GetPhi() / RAD_F;
		AreaLight.m_Width		= Light.GetWidth();
		AreaLight.m_Height		= Light.GetHeight();
		AreaLight.m_Distance	= Light.GetDistance();
		AreaLight.m_Color		= Light.GetIntensity() * CColorRgbHdr(Light.GetColor().redF(), Light.GetColor().greenF(), Light.GetColor().blueF());

		AreaLight.Update(gScene.m_BoundingBox);

		gScene.m_Lighting.AddLight(AreaLight);
	}

	gScene.m_DirtyFlags.SetFlag(LightsDirty);
}

void QRenderThread::OnRenderPause(const bool& Pause)
{
	m_Pause = Pause;
}

CColorRgbLdr* QRenderThread::GetRenderImage(void) const
{
	return m_pRenderImage;
}

void StartRenderThread(QString& FileName)
{
	// Create new render thread
 	gpRenderThread = new QRenderThread(FileName);

	// Load the volume
 	if (!gpRenderThread->Load(FileName))
 		return;
 
	// Start the render thread
	gpRenderThread->start();
}

void StartRenderThreadRGBA(QString& FileName)
{
	// Create new render thread
 	gpRenderThread = new QRenderThread(FileName);

	// Load the volume
 	if (!gpRenderThread->LoadRGBA(FileName))
 		return;
 
	// Start the render thread
	gpRenderThread->start();
}

void KillRenderThread(void)
{
 	if (!gpRenderThread)
 		return;
 
	// Kill the render thread
	gpRenderThread->Close();

	// Wait for thread to end
	gpRenderThread->wait();

	// Remove the render thread
	delete gpRenderThread;
	gpRenderThread = NULL;
}


void SetOpacity(float* OpacityArray) {

}

void SetDensityScale(float* DenstiyScaleArray) {

}