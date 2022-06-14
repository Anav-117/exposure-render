#include "Stable.h"

#pragma once

class QMeshRendering : public QObject
{
    Q_OBJECT

public:
    QMeshRendering(QObject* pParent = NULL);
    void SetMajorClass(string Name);
    string GetMajorClass();
    virtual ~QMeshRendering(void);

signals:
	void	MajorClassChanged(void);
	
private:
    string  MajorClass;
};

// Transfer function singleton
extern QMeshRendering gMeshRendering;