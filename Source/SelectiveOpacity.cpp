#include "SelectiveOpacity.h"

SelectiveOpacity::SelectiveOpacity()
{
}

void SelectiveOpacity::InitOpacityArray(float* Opacity, int num) {
    OpacityArray = new float[num];
    OpacityArray = Opacity;
}

void SelectiveOpacity::InitDensityScaleArray(float* DensityScale, int num) {
    DensityScaleArray = new float[num];
    DensityScaleArray = DensityScale;
}

void SelectiveOpacity::SetOpacityArray(float* Opacity) {
    OpacityArray = Opacity;
}

void SelectiveOpacity::SetDensityScaleArray(float* DensityScale) {
    DensityScaleArray = DensityScale;
}

float* SelectiveOpacity::GetOpacityArray(void) {
    return OpacityArray;
}

float* SelectiveOpacity::GetDensityScaleArray(void) {
    return DensityScaleArray;
}

void SelectiveOpacity::SetNumSegments(int num) {
    NumSegments = num;
}

int SelectiveOpacity::GetNumSegments(void) {
    return NumSegments;
}