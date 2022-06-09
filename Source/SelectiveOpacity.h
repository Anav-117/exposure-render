#include "Stable.h"

#pragma once

class QSelectiveOpacity : public QObject
{
    Q_OBJECT

public:
    QSelectiveOpacity(QObject* pParent = NULL);
    void SetOpacityBuffer(float *Buffer);
    QSelectiveOpacity& operator = (const QSelectiveOpacity& Other);
    float* GetOpacityBuffer();
    virtual ~QSelectiveOpacity(void);

signals:
	void	Changed(void);
	
private:
    float*  OpacityBuffer;
};

// Transfer function singleton
extern QSelectiveOpacity gSelectiveOpacity;