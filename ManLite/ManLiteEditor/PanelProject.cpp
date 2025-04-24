#include "PanelProject.h"

#include "GUI.h"
#include "Log.h"
#include "ResourceManager.h"
#include "FilesManager.h"

#include <imgui.h>


PanelProject::PanelProject(PanelType type, std::string name, bool enabled) : Panel(type, name, enabled)
{
    current_path.push_back("Assets");
    current_directory = &FilesManager::GetInstance().GetFileData();

    int w = 0, h = 0;
    unknown_icon        = ResourceManager::GetInstance().LoadTexture("Config\\Icons\\unknown_icon.png"      , w, h);
    folder_icon         = ResourceManager::GetInstance().LoadTexture("Config\\Icons\\folder_icon.png"       , w, h);
    image_icon          = ResourceManager::GetInstance().LoadTexture("Config\\Icons\\image_icon.png"        , w, h);
    animation_icon      = ResourceManager::GetInstance().LoadTexture("Config\\Icons\\animation_icon.png"    , w, h);
    audio_icon          = ResourceManager::GetInstance().LoadTexture("Config\\Icons\\audio_icon.png"        , w, h);
    font_icon           = ResourceManager::GetInstance().LoadTexture("Config\\Icons\\font_icon.png"         , w, h);
    particles_icon      = ResourceManager::GetInstance().LoadTexture("Config\\Icons\\particles_icon.png"    , w, h);
    scene_icon          = ResourceManager::GetInstance().LoadTexture("Config\\Icons\\icon.png"              , w, h);
    script_icon         = ResourceManager::GetInstance().LoadTexture("Config\\Icons\\script_icon.png"       , w, h);
    tiled_icon          = ResourceManager::GetInstance().LoadTexture("Config\\Icons\\tiled_icon.png"        , w, h);
}

PanelProject::~PanelProject()
{
    if (unknown_icon    != 0) ResourceManager::GetInstance().ReleaseTexture("Config\\Icons\\unknown_icon.png"   );
    if (folder_icon     != 0) ResourceManager::GetInstance().ReleaseTexture("Config\\Icons\\folder_icon.png"    );
    if (image_icon      != 0) ResourceManager::GetInstance().ReleaseTexture("Config\\Icons\\image_icon.png"     );
    if (animation_icon  != 0) ResourceManager::GetInstance().ReleaseTexture("Config\\Icons\\animation_icon.png" );
    if (audio_icon      != 0) ResourceManager::GetInstance().ReleaseTexture("Config\\Icons\\audio_icon.png"     );
    if (font_icon       != 0) ResourceManager::GetInstance().ReleaseTexture("Config\\Icons\\font_icon.png"      );
    if (particles_icon  != 0) ResourceManager::GetInstance().ReleaseTexture("Config\\Icons\\particles_icon.png" );
    if (scene_icon      != 0) ResourceManager::GetInstance().ReleaseTexture("Config\\Icons\\icon.png"           );
    if (script_icon     != 0) ResourceManager::GetInstance().ReleaseTexture("Config\\Icons\\script_icon.png"    );
    if (tiled_icon      != 0) ResourceManager::GetInstance().ReleaseTexture("Config\\Icons\\tiled_icon.png"     );
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
    uint texture_id = unknown_icon;
    switch (file_data.type) {
    case FOLDER:        texture_id = folder_icon;       break;
    case IMAGE:         texture_id = image_icon;        break;
    case ANIMATION:     texture_id = animation_icon;    break;
    case AUDIO:         texture_id = audio_icon;        break;
    case FONT:          texture_id = font_icon;         break;
    case PARTICLES:     texture_id = particles_icon;    break;
    case SCENE:         texture_id = scene_icon;        break;
    case SCRIPT:        texture_id = script_icon;       break;
    case TILED:         texture_id = tiled_icon;        break;
    case PREFAB:        texture_id = prefab_icon;       break;
    default:            texture_id = unknown_icon;      break;
    }

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (depth == 0) flags |= ImGuiTreeNodeFlags_DefaultOpen;
    if (file_data.type != FOLDER) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    ImGui::PushID(file_data.relative_path.c_str());

    bool is_open = ImGui::TreeNodeEx("##node", flags, "");

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        if (file_data.type == FOLDER) {
            UpdateCurrentDirectory(&file_data);
        }
    }

    ImGui::SameLine();

    const float icon_size = ImGui::GetTextLineHeight() * 1.2f;
    ImGui::Image(
        (ImTextureID)(intptr_t)texture_id,
        ImVec2(icon_size, icon_size),
        ImVec2(0, 1),
        ImVec2(1, 0),
        ImVec4(1, 1, 1, 1)
    );

    ImGui::SameLine();
    ImGui::TextUnformatted(file_data.name.c_str());

    if (is_open && file_data.type == FOLDER) {
        for (const auto& child : file_data.children) {
            RenderFileTree(child, depth + 1);
        }
        ImGui::TreePop();
    }

    ImGui::PopID();
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
        for (const auto& item : current_directory->children)
        {
            uint texture_id = unknown_icon;
            switch (item.type) {
            case FOLDER:        texture_id = folder_icon;       break;
            case IMAGE:         texture_id = image_icon;        break;
            case ANIMATION:     texture_id = animation_icon;    break;
            case AUDIO:         texture_id = audio_icon;        break;
            case FONT:          texture_id = font_icon;         break;
            case PARTICLES:     texture_id = particles_icon;    break;
            case SCENE:         texture_id = scene_icon;        break;
            case SCRIPT:        texture_id = script_icon;       break;
            case TILED:         texture_id = tiled_icon;        break;
            case PREFAB:        texture_id = prefab_icon;       break;
            default:            texture_id = unknown_icon;      break;
            }

            ImGui::PushID(item.relative_path.c_str());

            ImGui::ImageButton("",
                (ImTextureID)(intptr_t)texture_id,
                ImVec2(icon_size - padding, icon_size - padding),
                ImVec2(0, 1),
                ImVec2(1, 0)
            );

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                if (item.type == FOLDER) {
                    UpdateCurrentDirectory(&item);
                }
                else
                {

                }
            }

            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + icon_size);
            ImGui::TextWrapped("%s", item.name.c_str());
            ImGui::PopTextWrapPos();

            ImGui::PopID();
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
