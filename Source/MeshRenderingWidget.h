#pragma once

class QMeshRenderingWidget : public QGroupBox
{
	Q_OBJECT

public:
    QMeshRenderingWidget(QWidget* pParent = NULL);

public slots:
	void OnRenderBegin(void);

private:
	QGridLayout		m_MainLayout;
};