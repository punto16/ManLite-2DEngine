#include "PanelProject.h"

#include "GUI.h"
#include "Log.h"

#include <imgui.h>
#include "FilesManager.h"


PanelProject::PanelProject(PanelType type, std::string name, bool enabled) : Panel(type, name, enabled)
{
}

PanelProject::~PanelProject()
{
}

bool PanelProject::Update()
{
	if (bringToFront)
	{
		ImGui::SetNextWindowFocus();
		bringToFront = false;
	}

	bool ret = true;

	if (ImGui::Begin(name.c_str(), &enabled))
    {
        ImGui::BeginChild("LeftPanel", ImVec2(250, 0), true);
        FileData& root = FilesManager::GetInstance().GetFileData();
        RenderFileTree(root);
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("RightPanel");
        RenderBreadcrumbs();
        ImGui::Separator();
        RenderContentGrid();
        ImGui::EndChild();
    }
    ImGui::End();

    return ret;
}

void PanelProject::RenderFileTree(const FileData& file_data, int depth)
{
    const ImVec4 folder_color = ImVec4(0.8f, 0.8f, 0.0f, 1.0f);
    const ImVec4 file_color = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);

    // Determinar icono
    const char* icon = UNKNOWN_ICON;
    switch (file_data.type) {
    case FOLDER:        icon = FOLDER_ICON; break;
    case IMAGE:         icon = IMAGE_ICON; break;
    case ANIMATION:     icon = ANIMATION_ICON; break;
    case AUDIO:         icon = AUDIO_ICON; break;
    case FONT:          icon = FONT_ICON; break;
    case PARTICLES:     icon = PARTICLES_ICON; break;
    case SCENE:         icon = SCENE_ICON; break;
    case SCRIPT:        icon = SCRIPT_ICON; break;
    case TILED:         icon = TILED_ICON; break;
    }

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
    if (depth == 0) {
        flags |= ImGuiTreeNodeFlags_DefaultOpen;
    }

    if (file_data.type != FOLDER) {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }

    if (file_data.type == FOLDER) {
        ImGui::PushStyleColor(ImGuiCol_Text, folder_color);
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Text, file_color);
    }

    bool is_open = ImGui::TreeNodeEx(file_data.name.c_str(), flags, "%s%s", icon, file_data.name.c_str());
    ImGui::PopStyleColor();

    if (ImGui::IsItemClicked() && file_data.type == FOLDER) {
        UpdateCurrentDirectory(&file_data);
    }

    if (is_open && file_data.type == FOLDER) {
        for (const auto& child : file_data.children) {
            RenderFileTree(child, depth + 1);
        }
        ImGui::TreePop();
    }
}

void PanelProject::RenderBreadcrumbs()
{
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));

    std::string full_path;
    for (size_t i = 0; i < current_path.size(); ++i) {
        if (i > 0) ImGui::SameLine();

        ImGui::Text(current_path[i].c_str());
        if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(7);
        if (ImGui::IsItemHovered() && ImGui::IsItemClicked()) {
            current_path.resize(i + 1);
            UpdateCurrentDirectory(nullptr);
        }

        if (i < current_path.size() - 1) {
            ImGui::SameLine();
            ImGui::TextUnformatted("/");
        }
    }

    ImGui::PopStyleVar();
}

void PanelProject::RenderContentGrid()
{
    const float icon_size = 64.0f;
    const float padding = 8.0f;

    ImGui::BeginChild("ContentArea");

    float panel_width = ImGui::GetContentRegionAvail().x;
    int columns = static_cast<int>(panel_width / (icon_size + padding));
    columns = columns < 1 ? 1 : columns;

    ImGui::Columns(columns, NULL, false);

    if (current_directory) {
        for (const auto& item : current_directory->children) {
            // Icono
            ImGui::PushID(item.relative_path.c_str());

            ImGui::Button(item.type == FOLDER ? FOLDER_ICON : IMAGE_ICON,
                ImVec2(icon_size, icon_size));

            // Manejar clicks
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                if (item.type == FOLDER) {
                    UpdateCurrentDirectory(&item);
                }
                else
                {
                    LOG(LogType::LOG_INFO, "File %s Clicked", item.name.c_str());
                }
            }
            ImGui::PopID();
            // Texto
            ImGui::TextWrapped("%s", item.name.c_str());
            ImGui::NextColumn();
        }
    }

    ImGui::Columns(1);
    ImGui::EndChild();
}

void PanelProject::UpdateCurrentDirectory(const FileData* new_dir)
{
    if (new_dir) {
        current_path.clear();

        // Usar la ruta relativa desde Assets
        std::string path = new_dir->relative_path;

        // Dividir usando '/' como separador
        size_t start = 0;
        size_t end = path.find('/');

        while (end != std::string::npos) {
            current_path.push_back(path.substr(start, end - start));
            start = end + 1;
            end = path.find('/', start);
        }
        current_path.push_back(path.substr(start));

        current_directory = new_dir;
    }
    else {
        // Navegar desde la raíz usando el path relativo
        const FileData* current = &FilesManager::GetInstance().GetFileData();
        for (const auto& path_segment : current_path) {
            if (path_segment == "Assets") continue; // Saltar el root

            bool found = false;
            for (const auto& child : current->children) {
                if (child.name == path_segment && child.type == FOLDER) {
                    current = &child;
                    found = true;
                    break;
                }
            }
            if (!found) break;
        }
        current_directory = current;
    }
}
