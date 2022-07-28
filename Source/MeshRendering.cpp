/*

Mesh Rendering Singleton
    Contains variables shared between the Mesh Rendering Tab and Selective Rendering Tree 
        Major Class - Name of the Major class selected in Mesh View
        Scalar Range - Scalar Range of Mesh Selected in Mesh View (is unique for every mesh and is used to identify mesh)
*/

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