/*

Selective Opacity Singleton to hold Opacity Buffer for 1D Opacity Texture

*/

#include "SelectiveOpacity.h"

QSelectiveOpacity gSelectiveOpacity;

QSelectiveOpacity::QSelectiveOpacity(QObject* pParent) :
    QObject(pParent)
{
}

QSelectiveOpacity& QSelectiveOpacity::operator = (const QSelectiveOpacity& Other)			
{
    OpacityBuffer = Other.OpacityBuffer;
}

void QSelectiveOpacity::SetOpacityBuffer(float* Buffer) {
    OpacityBuffer = Buffer;
    emit Changed();
}

float* QSelectiveOpacity::GetOpacityBuffer() {
    return OpacityBuffer;
}

void QSelectiveOpacity::SetSize(int mSize) {
    Size = mSize;
}

int QSelectiveOpacity::GetSize() {
    return Size;
}