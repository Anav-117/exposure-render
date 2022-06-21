#include "MeshRendering.h"
#include <iostream>

QMeshRendering gMeshRendering;

QMeshRendering::QMeshRendering(QObject* pParent) :
    QObject(pParent)
{
}

QMeshRendering::~QMeshRendering() {}

void QMeshRendering::SetMajorClass(string Name) {
    MajorClass = Name;
    emit MajorClassChanged();
}

string QMeshRendering::GetMajorClass() {
    return MajorClass;
}

void QMeshRendering::SetScalarRange(double* Range) {
    ScalarRange = Range;
    emit ScalarRangeChanged();
}

double* QMeshRendering::GetScalarRange() {
    return ScalarRange;
}