#pragma once
#include <string>
#include <vector>
#include <map>
#include "imgui.h"

enum class PlanPriority { Low = 0, Medium, High };

struct PlanItem {
    std::string Title;
    std::string Description;
    std::string Category;
    PlanPriority Priority = PlanPriority::Medium;
    bool Completed = false;
};

class PlanPanel {
public:
    PlanPanel();
    ~PlanPanel() = default;

    void OnImGuiRender(bool* p_open = nullptr);

    void Save();
    void Load();

private:
    void DrawProgressBar(float fraction, const char* label);
    const char* PriorityToString(PlanPriority p);
    ImVec4 PriorityToColor(PlanPriority p);

private:
    std::vector<PlanItem> m_Items;
    
    // UI State
    char m_NewItemTitle[128] = "";
    char m_NewItemDescription[256] = "";
    char m_NewItemCategory[128] = "General";
    int m_NewItemPriority = 1; // Medium
    
    bool m_FilterCompleted = false;
    char m_SearchBuffer[128] = "";
};
