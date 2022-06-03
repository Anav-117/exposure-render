#include "MeshRenderingWidget.h"
// #include <vtkRenderWindow.h>
// #include <vtkRenderer.h>
// #include <vtkRenderWindowInteractor.h>

QMeshRenderingWidget::QMeshRenderingWidget(QWidget* pParent) :
	QGroupBox(pParent),
	m_MainLayout(),
    //m_RenderWindow()
{
    // m_MainLayout.setAlignment(Qt::AlignTop);
	// setLayout(&m_MainLayout);

    // vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();

    // vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    // renderWindow->AddRenderer(renderer);

    // m_RenderWindow.SetRenderWindow(renderWindow); 

    // vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    // /*
    // vtkSmartPointer<vtkUserInteractionStyle> inter = vtkUserInteractionStyle::New ();
    // inter->SetDefaultRenderer(renderer);
    // renderWindowInteractor->SetInteractorStyle( inter );
    // */

    // renderWindowInteractor->SetRenderWindow ( renderWindow );    

    // m_RenderWindow.show();
    // renderWindow->Render ();
    // renderWindowInteractor->Initialize();

    // m_MainLayout.addWidget(&m_RenderWindow, 1, 0);
}