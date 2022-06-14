/*
	Copyright (c) 2011, T. Kroes <t.kroes@tudelft.nl>
	All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

	- Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
	- Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
	- Neither the name of the <ORGANIZATION> nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
	
	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <cuda_runtime.h>

DEV float Rx, Ry, Rz;
DEV float Rox, Roy, Roz;
DEV float RSx, RSy, RSz;
DEV float DensityScale;
DEV float isRGBA, SegmentAvailable;
DEV float weight[7] = {0.006, 0.061, 0.242, 0.383, 0.242, 0.061, 0.006};

DEV inline Vec3f ToVec3f(const float3& V)
{
	return Vec3f(V.x, V.y, V.z);
}

DEV float pi;
DEV const int samples = 3;
DEV float sigma;

DEV float pow3(float p) {
	return p*p*p;
}

DEV float gaussian(float x, float y, float z) {
    return 1.0 / sqrt(pow3(2.0 * pi * (sigma*sigma))) * exp(-(((x*x) + (y*y) + (z*z)) / (2.0 * (sigma*sigma))));
}

DEV float GetNormalizedIntensity(const Vec3f& P)
{
	if (isRGBA) {
		float4 sample = tex3D(gTexDensityRGBA, P.x * gInvAaBbMax.x, P.y * gInvAaBbMax.y, P.z * gInvAaBbMax.z);
		const float Intensity = ((float)SHRT_MAX * ((sample.x + sample.y + sample.z)/3.0f));
		//printf("%f\n", (float)tex3D(gTexDensity2, P.x, P.y, P.z));
		return (Intensity - gIntensityMin) * gIntensityInvRange;
	}
	else {
		const float Intensity = ((float)SHRT_MAX * tex3D(gTexDensity, P.x * gInvAaBbMax.x, P.y * gInvAaBbMax.y, P.z * gInvAaBbMax.z));
		return (Intensity - gIntensityMin) * gIntensityInvRange;
	}
}

DEV void SetResolution(float gRx, float gRy, float gRz, float gRSx, float gRSy, float gRSz) {
	Rx = gRx;
	Ry = gRy;
	Rz = gRz;
	RSx = gRSx;
	RSy = gRSy;
	RSz = gRSz;
	Rox = 1.0f;
	Roy = 1.0f;
	Roz = 1.0f;

	pi = atan(1.0) * 4.0;
	sigma = float(samples) * 0.25;
}

DEV void SetSegmentAvailable(bool gSegmentAvailable) {
	SegmentAvailable = gSegmentAvailable;
}

DEV void SetRGBA(bool gisRGBA) {
	isRGBA = gisRGBA;
}

DEV float GetDensityScale () {
	return DensityScale;
}

DEV float GetOpacity(const float& NormalizedIntensity, float3 P)
{
	DensityScale = gDensityScale;
	if (isRGBA) {
		uchar4 BG = tex3D(gTexOpacityRGBA, (P.x*1.37f)*Rx, (P.y*1.11f)*Ry, (P.z)*Rz); 
		float3 Pn;
		Pn.x = P.x*1.37f;
		Pn.y = P.y*1.1f;
		Pn.z = P.z;
		//printf("DIFF - %f\n", 1.0f - P.y);
		if (SegmentAvailable) {
			if ((float)(BG.w) == 2.0f) {
				return 0.0f;
			}
			
			float op = 0.0f;
			float accum = 0.0f;

			for (int x = -samples / 2; x < samples / 2; ++x) {
				for (int y = -samples / 2; y < samples / 2; ++y) {
					for (int z = -samples / 2; z < samples / 2; ++z) {
						float weight = gaussian((x), (y), (z));
						short SegmentColor = tex3D(gTexOpacityRGB, Pn.x*Rx + (x), Pn.y*Ry + (y), Pn.z*Rz + (z));
						op = op + (tex1D(gTexSelectiveOpacity, (float)(SegmentColor - 1.0f)) * weight);		
						accum += weight;
					}
				}
			}

			//printf("OP - %f\n", op);

			op = op/accum;

			//short SegmentColor = tex3D(gTexOpacityRGB, Pn.x*Rx, Pn.y*Ry, Pn.z*Rz);

			//float op = tex1D(gTexSelectiveOpacity, (float)(SegmentColor) - 1.0f);
			
			return (op);
		}
		else {
			DensityScale = gDensityScale;
			return 1.0f;
		}
	}
	else {
		DensityScale = gDensityScale;
		return tex1D(gTexOpacity, NormalizedIntensity);
	}
}

DEV CColorRgbHdr GetDiffuse(const float& NormalizedIntensity, Vec3f P)
{
	float3 Pn;
	Pn.x = P.x*1.37f;
	Pn.y = P.y*1.1f;
	Pn.z = P.z;
	if (isRGBA) {
		uchar4 Diffuse = tex3D(gTexDiffuseRGBA, Pn.x*Rx, Pn.y*Ry, Pn.z*Rz);
		//printf("COLOR - %f : %f : %f\n", (float)Diffuse.x, (float)Diffuse.y, (float)Diffuse.z);
		return CColorRgbHdr((float)Diffuse.x/255.0f, (float)Diffuse.y/255.0f, (float)Diffuse.z/255.0f);
		//return CColorRgbHdr(0, 255, 0);
	}
	else  {
		float4 Diffuse = tex1D(gTexDiffuse, NormalizedIntensity);
		//printf("COLOR - %f : %f : %f\n", (float)Diffuse.x, (float)Diffuse.y, (float)Diffuse.z);
		return CColorRgbHdr((float)Diffuse.x, (float)Diffuse.y, (float)Diffuse.z);
	}
}

DEV CColorRgbHdr GetSpecular(const float& NormalizedIntensity)
{
	float4 Specular = tex1D(gTexSpecular, NormalizedIntensity);
	return CColorRgbHdr(Specular.x, Specular.y, Specular.z);
}

DEV float GetRoughness(const float& NormalizedIntensity)
{
	return tex1D(gTexRoughness, NormalizedIntensity);
}

DEV CColorRgbHdr GetEmission(const float& NormalizedIntensity)
{
	float4 Emission = tex1D(gTexEmission, NormalizedIntensity);
	return CColorRgbHdr(Emission.x, Emission.y, Emission.z);
}

DEV inline Vec3f NormalizedGradient(const Vec3f& P)
{
	Vec3f Gradient;

	Gradient.x = (GetNormalizedIntensity(P + ToVec3f(gGradientDeltaX)) - GetNormalizedIntensity(P - ToVec3f(gGradientDeltaX))) * gInvGradientDelta;
	Gradient.y = (GetNormalizedIntensity(P + ToVec3f(gGradientDeltaY)) - GetNormalizedIntensity(P - ToVec3f(gGradientDeltaY))) * gInvGradientDelta;
	Gradient.z = (GetNormalizedIntensity(P + ToVec3f(gGradientDeltaZ)) - GetNormalizedIntensity(P - ToVec3f(gGradientDeltaZ))) * gInvGradientDelta;

	//printf("%f : %f\n", GetNormalizedIntensity(P + ToVec3f(gGradientDeltaX)), GetNormalizedIntensity(P - ToVec3f(gGradientDeltaX)));

	return Normalize(Gradient);
}

DEV float GradientMagnitude(const Vec3f& P)
{
	return ((float)SHRT_MAX * tex3D(gTexGradientMagnitude, P.x * gInvAaBbMax.x, P.y * gInvAaBbMax.y, P.z * gInvAaBbMax.z));
}

DEV bool NearestLight(CScene* pScene, CRay R, CColorXyz& LightColor, Vec3f& Pl, CLight*& pLight, float* pPdf = NULL)
{
	bool Hit = false;
	
	float T = 0.0f;

	CRay RayCopy = R;

	float Pdf = 0.0f;

	for (int i = 0; i < pScene->m_Lighting.m_NoLights; i++)
	{
		if (pScene->m_Lighting.m_Lights[i].Intersect(RayCopy, T, LightColor, NULL, &Pdf))
		{
			Pl		= R(T);
			pLight	= &pScene->m_Lighting.m_Lights[i];
			Hit		= true;
		}
	}
	
	if (pPdf)
		*pPdf = Pdf;

	return Hit;
}

DEV bool IntersectBox(const CRay& R, float* pNearT, float* pFarT)
{
	const Vec3f InvR		= Vec3f(1.0f, 1.0f, 1.0f) / R.m_D;
	const Vec3f BottomT		= InvR * (Vec3f(gAaBbMin.x, gAaBbMin.y, gAaBbMin.z) - R.m_O);
	const Vec3f TopT		= InvR * (Vec3f(gAaBbMax.x, gAaBbMax.y, gAaBbMax.z) - R.m_O);
	const Vec3f MinT		= MinVec3f(TopT, BottomT);
	const Vec3f MaxT		= MaxVec3f(TopT, BottomT);
	const float LargestMinT = fmaxf(fmaxf(MinT.x, MinT.y), fmaxf(MinT.x, MinT.z));
	const float LargestMaxT = fminf(fminf(MaxT.x, MaxT.y), fminf(MaxT.x, MaxT.z));

	*pNearT = LargestMinT;
	*pFarT	= LargestMaxT;

	return LargestMaxT > LargestMinT;
}

DEV CColorXyza CumulativeMovingAverage(const CColorXyza& A, const CColorXyza& Ax, const int& N)
{
//	if (gNoIterations == 0)
//		return CColorXyza(0.0f);

	 return A + ((Ax - A) / max((float)N, 1.0f));
}
