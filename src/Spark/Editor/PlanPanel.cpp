#include "PlanPanel.h"
#include <imgui.h>
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <filesystem>

static std::string GetPlanPath() {
    if (std::filesystem::exists("assets/plan.yaml"))
        return "assets/plan.yaml";
    if (std::filesystem::exists("../assets/plan.yaml"))
        return "../assets/plan.yaml";
    return "assets/plan.yaml"; // Default fall-back
}

PlanPanel::PlanPanel() {
    Load();
}

void PlanPanel::OnImGuiRender(bool* p_open) {
    if (p_open && !*p_open) return; // Not active
    
    ImGui::Begin("Plan & Roadmap", p_open);

    // Toolbar
    if (ImGui::Button("Save Plan")) Save();
    ImGui::SameLine();
    if (ImGui::Button("Reload Plan")) Load();
    ImGui::SameLine();
    ImGui::Checkbox("Hide Completed", &m_FilterCompleted);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(150.0f);
    ImGui::InputText("Search", m_SearchBuffer, 128);

    ImGui::Separator();

    // Overall Progress
    int totalItems = (int)m_Items.size();
    int completedItems = 0;
    for(const auto& item : m_Items) if(item.Completed) completedItems++;
    float overallProgress = totalItems > 0 ? (float)completedItems / (float)totalItems : 0.0f;
    DrawProgressBar(overallProgress, "Overall Project Progress");

    ImGui::Separator();

    // Add New Feature Section (Collapsing for space)
    if (ImGui::CollapsingHeader("Add New Task")) {
        ImGui::Columns(2, "AddTaskCols", false);
        ImGui::SetColumnWidth(0, 80);
        
        ImGui::Text("Title"); ImGui::NextColumn();
        ImGui::InputText("##title", m_NewItemTitle, 128); ImGui::NextColumn();
        
        ImGui::Text("Desc"); ImGui::NextColumn();
        ImGui::InputText("##desc", m_NewItemDescription, 256); ImGui::NextColumn();
        
        ImGui::Text("Category"); ImGui::NextColumn();
        ImGui::InputText("##cat", m_NewItemCategory, 128); ImGui::NextColumn();
        
        ImGui::Text("Priority"); ImGui::NextColumn();
        const char* priorities[] = { "Low", "Medium", "High" };
        ImGui::Combo("##prio", &m_NewItemPriority, priorities, IM_ARRAYSIZE(priorities)); ImGui::NextColumn();
        
        ImGui::Columns(1);
        if (ImGui::Button("Add Task", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
            if (strlen(m_NewItemTitle) > 0) {
                m_Items.push_back({ 
                    m_NewItemTitle, 
                    m_NewItemDescription, 
                    m_NewItemCategory, 
                    (PlanPriority)m_NewItemPriority, 
                    false 
                });
                m_NewItemTitle[0] = '\0';
                m_NewItemDescription[0] = '\0';
                // Category stays the same for batch entry
            }
        }
    }

    ImGui::Separator();

    // Grouping by Category
    std::map<std::string, std::vector<int>> groupedItems;
    for (int i = 0; i < (int)m_Items.size(); i++) {
        // Apply Filters
        if (m_FilterCompleted && m_Items[i].Completed) continue;
        if (strlen(m_SearchBuffer) > 0) {
            std::string search = m_SearchBuffer;
            std::transform(search.begin(), search.end(), search.begin(), ::tolower);
            std::string title = m_Items[i].Title;
            std::transform(title.begin(), title.end(), title.begin(), ::tolower);
            if (title.find(search) == std::string::npos) continue;
        }
        groupedItems[m_Items[i].Category].push_back(i);
    }

    for (auto& [category, indices] : groupedItems) {
        // Category Header with mini-progress
        int catTotal = (int)indices.size();
        int catDone = 0;
        for(int idx : indices) if(m_Items[idx].Completed) catDone++;
        
        char catHeader[256];
        sprintf(catHeader, "%s (%d/%d)###%s", category.c_str(), catDone, catTotal, category.c_str());
        
        if (ImGui::CollapsingHeader(catHeader, ImGuiTreeNodeFlags_DefaultOpen)) {
            for (int idx : indices) {
                ImGui::PushID(idx);
                
                // Priority Indicator
                ImVec4 pColor = PriorityToColor(m_Items[idx].Priority);
                ImGui::ColorButton("##prio_color", pColor, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, ImVec2(10, 20));
                ImGui::SameLine();

                if (ImGui::Checkbox("##completed", &m_Items[idx].Completed)) {
                    // Auto-save?
                }
                ImGui::SameLine();
                
                bool treeOpen = ImGui::TreeNodeEx(m_Items[idx].Title.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth | (m_Items[idx].Completed ? ImGuiTreeNodeFlags_Leaf : 0));
                
                if (treeOpen) {
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Priority: %s", PriorityToString(m_Items[idx].Priority));
                    ImGui::TextWrapped("%s", m_Items[idx].Description.c_str());
                    
                    if (ImGui::Button("Remove")) {
                        m_Items.erase(m_Items.begin() + idx);
                        ImGui::TreePop();
                        ImGui::PopID();
                        ImGui::End(); // End this frame to avoid iterator issues
                        return;
                    }
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
        }
    }

    ImGui::End();
}

void PlanPanel::DrawProgressBar(float fraction, const char* label) {
    ImGui::Text("%s", label);
    ImGui::SameLine();
    char overlay[32];
    sprintf(overlay, "%.0f%%", fraction * 100.0f);
    ImGui::ProgressBar(fraction, ImVec2(-1.0f, 0.0f), overlay);
}

const char* PlanPanel::PriorityToString(PlanPriority p) {
    switch (p) {
        case PlanPriority::Low: return "Low";
        case PlanPriority::Medium: return "Medium";
        case PlanPriority::High: return "High";
    }
    return "Unknown";
}

ImVec4 PlanPanel::PriorityToColor(PlanPriority p) {
    switch (p) {
        case PlanPriority::Low: return ImVec4(0.2f, 0.8f, 0.2f, 1.0f); // Green
        case PlanPriority::Medium: return ImVec4(0.8f, 0.8f, 0.2f, 1.0f); // Yellow
        case PlanPriority::High: return ImVec4(0.8f, 0.2f, 0.2f, 1.0f); // Red
    }
    return ImVec4(1, 1, 1, 1);
}

void PlanPanel::Save() {
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "Plan" << YAML::Value << YAML::BeginSeq;

    for (const auto& item : m_Items) {
        out << YAML::BeginMap;
        out << YAML::Key << "Title" << YAML::Value << item.Title;
        out << YAML::Key << "Description" << YAML::Value << item.Description;
        out << YAML::Key << "Category" << YAML::Value << item.Category;
        out << YAML::Key << "Priority" << YAML::Value << (int)item.Priority;
        out << YAML::Key << "Completed" << YAML::Value << item.Completed;
        out << YAML::EndMap;
    }

    out << YAML::EndSeq;
    out << YAML::EndMap;

    std::ofstream fout(GetPlanPath());
    fout << out.c_str();
}

void PlanPanel::Load() {
    std::string path = GetPlanPath();
    std::ifstream stream(path);
    if (!stream.is_open()) {
        std::cerr << "Could not open plan file at: " << path << std::endl;
        return;
    }

    std::stringstream strStream;
    strStream << stream.rdbuf();

    try {
        YAML::Node data = YAML::Load(strStream.str());
        if (!data["Plan"]) return;

        m_Items.clear();
        auto plan = data["Plan"];
        for (auto item : plan) {
            PlanItem pi;
            pi.Title = item["Title"].as<std::string>();
            pi.Description = item["Description"].as<std::string>();
            pi.Category = item["Category"] ? item["Category"].as<std::string>() : "General";
            pi.Priority = item["Priority"] ? (PlanPriority)item["Priority"].as<int>() : PlanPriority::Medium;
            pi.Completed = item["Completed"].as<bool>();
            m_Items.push_back(pi);
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to load plan: " << e.what() << std::endl;
    }
}
