#pragma once

#include "Stable.h"

class QSelectiveOpacity : public QObject
{
    Q_OBJECT

public:
    QSelectiveOpacity(QObject* pParent = NULL);
    void SetOpacityBuffer(float *Buffer);
    QSelectiveOpacity& operator = (const QSelectiveOpacity& Other);
    float* GetOpacityBuffer();
    void SetSize(int);
    int GetSize();

signals:
	void	Changed(void);
	
private:
    float*  OpacityBuffer;
    int     Size;
};

// Transfer function singleton
extern QSelectiveOpacity gSelectiveOpacity;