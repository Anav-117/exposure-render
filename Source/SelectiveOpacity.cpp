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

QSelectiveOpacity::~QSelectiveOpacity() {}

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