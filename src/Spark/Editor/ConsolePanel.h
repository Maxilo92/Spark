#pragma once
#include "Log.h"

namespace Spark {

    class ConsolePanel {
    public:
        ConsolePanel();
        void OnImGuiRender(bool* p_open = nullptr);

    private:
        bool m_AutoScroll = true;
        bool m_FilterInfo = true;
        bool m_FilterWarning = true;
        bool m_FilterError = true;
        char m_SearchFilter[128] = "";
    };

}
