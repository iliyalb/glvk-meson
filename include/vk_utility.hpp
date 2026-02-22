#pragma once

class QueueFamily {
public:
    int m_graphicsFamily = -1;
    bool isValid() {
        return m_graphicsFamily >= 0;
    }
};
