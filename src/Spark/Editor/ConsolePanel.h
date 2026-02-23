#pragma once
#include "Log.h"

namespace Spark {

    class ConsolePanel {
    public:
        ConsolePanel();
        void OnImGuiRender(bool* p_open = nullptr);

    private:
        bool m_AutoScroll = true;
    };

}
