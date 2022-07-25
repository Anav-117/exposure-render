#include "Stable.h"

#pragma once

class QPoseTraceWidget : public QGroupBox
{
	Q_OBJECT

public:
    QPoseTraceWidget(QWidget* pParent = NULL);
	virtual ~QPoseTraceWidget();

public slots:
	void OnRenderBegin(void);
    void OnRenderButtonClicked(void);

private:
  QGridLayout	  m_Layout;
  QLabel        m_FileName;
  QPushButton	  m_RenderButton;
  QProgressBar  m_Progress;
};