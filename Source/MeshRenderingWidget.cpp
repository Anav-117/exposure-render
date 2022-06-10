#include "MeshRenderingWidget.h"
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkAxes.h>
#include <vtkCamera.h>

QMeshRenderingWidget::QMeshRenderingWidget(QWidget* pParent) :
	QVTKOpenGLNativeWidget(pParent)    
{
   vtkSmartPointer<vtkAxes> centerAxes = vtkAxes::New();
    centerAxes->SetOrigin(0, 0, 0);
    centerAxes->SetSymmetric(1);
    centerAxes->SetComputeNormals(1);
    vtkSmartPointer<vtkPolyDataMapper> axesMapper = vtkPolyDataMapper::New();
    axesMapper->SetInputConnection(centerAxes->GetOutputPort());
    vtkSmartPointer<vtkActor> centerAxesActor = vtkSmartPointer<vtkActor>::New();
    centerAxesActor->SetMapper(axesMapper);
    centerAxesActor->GetProperty()->SetLighting(false);
    centerAxesActor->PickableOff();
    centerAxesActor->SetScale(0.4);

    vtkSmartPointer<vtkRenderer> renderer = vtkRenderer::New();
    renderer->AddActor(centerAxesActor);
    renderer->SetBackground(0.6, 0.2, 0.5);
    
    vtkSmartPointer<vtkCamera> aCamera = vtkCamera::New();
  	aCamera->SetViewUp(0, 0, -1);
	aCamera->SetPosition(0, -1, 0);
	aCamera->SetFocalPoint(0, 0, 0);
	aCamera->ComputeViewPlaneNormal();
	aCamera->Azimuth(30.0);
	aCamera->Elevation(30.0);
    renderer->SetActiveCamera(aCamera);
	renderer->ResetCamera();
	aCamera->Dolly(1.5);

    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow = vtkGenericOpenGLRenderWindow::New();
    this->SetRenderWindow(renderWindow);
    //this->setFixedHeight(380);
	this->GetRenderWindow()->AddRenderer(renderer);
    //std::cout<<"PIXELS!!!! - "<<renderWindow->ReadPixels()<<"\n";
	//this->show();

}

QMeshRenderingWidget::~QMeshRenderingWidget() {}

void QMeshRenderingWidget::OnRenderBegin() {

}