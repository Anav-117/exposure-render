#pragma once

#include "Stable.h"

class QMeshRendering : public QObject
{
    Q_OBJECT

public:
    QMeshRendering(QObject* pParent = NULL);
    void SetMajorClass(string Name);
    string GetMajorClass();
    void SetScalarRange(double* Range);
    double* GetScalarRange();

signals:
	void	MajorClassChanged(void);
    void    ScalarRangeChanged(void);
	
private:
    string  MajorClass;
    double* ScalarRange;
};

// Transfer function singleton
extern QMeshRendering gMeshRendering;