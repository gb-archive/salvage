#pragma once

#include "StdAfx.h"

class CProgressBar {
    CProgressCtrl m_progress;
public:
    void Set(unsigned int percent);
    CProgressBar(
        CFrameWnd *pF,
        LPCTSTR szMessage = NULL,
        unsigned int min = 0,
        unsigned int max = 100
    );
};

