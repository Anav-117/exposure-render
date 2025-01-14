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

#include "Camera.h"

#include <fstream>
#include <sstream>
#include <iostream>

QCamera gCamera;

QCamera::QCamera(QObject* pParent /*= NULL*/) :
	QPresetXML(pParent),
	m_Film(),
	m_Aperture(),
	m_Projection(),
	m_Focus(),
	m_From(1.0f),
	m_Target(0.5f),
	m_Up(0.0f, 1.0f, 0.0f)
{
}

QCamera::~QCamera(void)
{
}

QCamera::QCamera(const QCamera& Other)
{
	*this = Other;
};

QCamera& QCamera::operator=(const QCamera& Other)
{
	QPresetXML::operator=(Other);

	blockSignals(true);

	m_Film			= Other.m_Film;
	m_Aperture		= Other.m_Aperture;
	m_Projection	= Other.m_Projection;
	m_Focus			= Other.m_Focus;
	m_From			= Other.m_From;
	m_Target		= Other.m_Target;
	m_Up			= Other.m_Up;

	blockSignals(false);

	emit Changed();

	return *this;
}

/*
Function to load Camera Poses for PosTrace Rendering
	PoseTrace file naming COnvention - VolumeName_PoseTrace.txt
	PoseTrace file contents
		1st line must contain 3 comma separated floats describing the 'UP' vector for the camera
		all sunequent lines must contain 3 comma separaed floats describing the 'FROM' (aka Position) of the Camera 
	PoseTrace file generated through Paraview spline animation, using Paraview functions cam.GetViewUp and cam.GetPosition
*/

void QCamera::LoadCameraPoses(string FileName) {
	fstream Froms;
	bool UpLoaded = false;

	ifstream f(FileName);
	if (!f.good()) {	// In case Pose Trace file is not available, an arbitrary UP and FROM value is assigned
		std::cout << "POSE FILE UNAVAILABLE\n";
		CameraUps.push_back(Vec3f(0.0f, 1.0f, 0.0f));
		CameraFroms.push_back(Vec3f(1.0f));
		CameraTargets.push_back(Vec3f(0.0f));
		return;
	}

	Froms.open(FileName, ios::in);

	const size_t last_slash_idx = FileName.rfind('/');
	if (std::string::npos != last_slash_idx)
	{
		PoseFileDir = FileName.substr(0, last_slash_idx);
	}
	string tp;
	while (getline(Froms, tp)) { 
		 if (tp.length() == 0)
		 	break;
		 stringstream ss(tp);
		 float vec[6];
		 int i = 0;
		 while (ss.good()) {
			 string substr;
			 getline(ss, substr, ',');
			 vec[i++] = stof(substr);
		 }
		 if (!UpLoaded) {
			 CameraUps.push_back(Vec3f(vec[0], vec[1], vec[2]));
			 UpLoaded = true;
		 }
		 else {
			 CameraFroms.push_back(Vec3f(vec[0], vec[1], vec[2]));
			 CameraTargets.push_back(Vec3f(vec[3], vec[4], vec[5]));
		 }
	}
	Froms.close();

	Num_Poses = CameraFroms.size();

	SetUp(CameraUps[0]);
}

// Cycles through the loaded Camera Positions

bool QCamera::CycleCameraParams(void) {
	if (poseIndex >= CameraFroms.size()) {
		poseIndex = 0;
		return false;
	}

	SetFrom(Vec3f(CameraFroms[poseIndex].x / Resolution.x, CameraFroms[poseIndex].y / Resolution.y, CameraFroms[poseIndex].z / Resolution.z));
	SetTarget(Vec3f(CameraTargets[poseIndex].x / Resolution.x, CameraTargets[poseIndex].y / Resolution.y, CameraTargets[poseIndex].z / Resolution.z));

	poseIndex++;
	return true;
}

int QCamera::GetNumPoses(void) {
	return Num_Poses;
}

string QCamera::GetPoseFileDir(void) {
	return PoseFileDir;
}

void QCamera::SetResolution(Vec3f Res) {
	Resolution = Res;
}

QFilm& QCamera::GetFilm(void)
{
	return m_Film;
}

void QCamera::SetFilm(const QFilm& Film)
{
	m_Film = Film;
}

QAperture& QCamera::GetAperture(void)
{
	return m_Aperture;
}

void QCamera::SetAperture(const QAperture& Aperture)
{
	m_Aperture = Aperture;
}

QProjection& QCamera::GetProjection(void)
{
	return m_Projection;
}

void QCamera::SetProjection(const QProjection& Projection)
{
	m_Projection = Projection;
}

QFocus& QCamera::GetFocus(void)
{
	return m_Focus;
}

void QCamera::SetFocus(const QFocus& Focus)
{
	m_Focus = Focus;
}

Vec3f QCamera::GetFrom(void) const
{
	return m_From;
}

void QCamera::SetFrom(const Vec3f& From)
{
	m_From = From;

	emit Changed();
}

Vec3f QCamera::GetTarget(void) const
{
	return m_Target;
}

void QCamera::SetTarget(const Vec3f& Target)
{
	m_Target = Target;

	emit Changed();
}

Vec3f QCamera::GetUp(void) const
{
	return m_Up;
}

void QCamera::SetUp(const Vec3f& Up)
{
	m_Up = Up;

	emit Changed();
}

void QCamera::ReadXML(QDomElement& Parent)
{
	QPresetXML::ReadXML(Parent);

	auto film = Parent.firstChildElement("Film");
	m_Film.ReadXML(film);
	auto ap = Parent.firstChildElement("Aperture");
	m_Aperture.ReadXML(ap);
	auto proj = Parent.firstChildElement("Projection");
	m_Projection.ReadXML(proj);
	auto focus = Parent.firstChildElement("Focus");
	m_Focus.ReadXML(focus);

	ReadVectorElement(Parent, "From", m_From.x, m_From.y, m_From.z);
	ReadVectorElement(Parent, "Target", m_Target.x, m_Target.y, m_Target.z);
	ReadVectorElement(Parent, "Up", m_Up.x, m_Up.y, m_Up.z);
}

QDomElement QCamera::WriteXML(QDomDocument& DOM, QDomElement& Parent)
{
	// Camera
	QDomElement Camera = DOM.createElement("Preset");
	Parent.appendChild(Camera);

	QPresetXML::WriteXML(DOM, Camera);

	m_Film.WriteXML(DOM, Camera);
	m_Aperture.WriteXML(DOM, Camera);
	m_Projection.WriteXML(DOM, Camera);
	m_Focus.WriteXML(DOM, Camera);

	WriteVectorElement(DOM, Camera, "From", m_From.x, m_From.y, m_From.z);
	WriteVectorElement(DOM, Camera, "Target", m_Target.x, m_Target.y, m_Target.z);
	WriteVectorElement(DOM, Camera, "Up", m_Up.x, m_Up.y, m_Up.z);

	return Camera;
}

QCamera QCamera::Default(void)
{
	QCamera DefaultCamera;

	DefaultCamera.SetName("Default");

	return DefaultCamera;
}

void QCamera::OnFilmChanged(void)
{
	emit Changed();
	SetDirty();
}

void QCamera::OnApertureChanged(void)
{
	emit Changed();
	SetDirty();
}

void QCamera::OnProjectionChanged(void)
{
 	emit Changed();
	SetDirty();
}

void QCamera::OnFocusChanged(void)
{
	emit Changed();
	SetDirty();
}
