#pragma once

#include "Stable.h"

class QPoseTraceWidget : public QGroupBox
{
	Q_OBJECT

public:
    QPoseTraceWidget(QWidget* pParent = NULL);

public slots:
	void OnRenderBegin(void);
    void OnRenderButtonClicked(void);

private:
  QGridLayout	  m_Layout;
  QLabel        m_FileName;
  QPushButton	  m_RenderButton;
  QProgressBar  m_Progress;
};