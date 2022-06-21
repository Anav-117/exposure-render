#include "Stable.h"

#pragma once

class QMeshRendering : public QObject
{
    Q_OBJECT

public:
    QMeshRendering(QObject* pParent = NULL);
    void SetMajorClass(string Name);
    string GetMajorClass();
    void SetScalarRange(double* Range);
    double* GetScalarRange();
    virtual ~QMeshRendering(void);

signals:
	void	MajorClassChanged(void);
    void    ScalarRangeChanged(void);
	
private:
    string  MajorClass;
    double* ScalarRange;
};

// Transfer function singleton
extern QMeshRendering gMeshRendering;