#include "PanelProject.h"

#include "GUI.h"
#include "Log.h"
#include "ResourceManager.h"
#include "FilesManager.h"
#include "App.h"
#include "PanelSaveScene.h"
#include "PanelAnimation.h"

#include "Prefab.h"
#include "GameObject.h"

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
    prefab_icon         = ResourceManager::GetInstance().LoadTexture("Config\\Icons\\prefab_icon.png"       , w, h);
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
    if (prefab_icon     != 0) ResourceManager::GetInstance().ReleaseTexture("Config\\Icons\\prefab_icon.png"    );
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
        if (FilesManager::GetInstance().DidFilesUpdateThisFrame())
        {
            const FileData* new_fd = FilesManager::GetInstance().GetFileDataByPath(current_path_string);
            UpdateCurrentDirectory(new_fd);
            if (!item_to_rename_path_string.empty())
                item_to_rename = FilesManager::GetInstance().GetFileDataByPath(item_to_rename_path_string);
        }
        ImGui::BeginChild("LeftPanel", ImVec2(250, 0), ImGuiChildFlags_ResizeX | ImGuiChildFlags_Borders);
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
        full_path += current_path[i] + "\\";

        ImGui::Text(current_path[i].c_str());
        DropZone(full_path);
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

    float available_height = ImGui::GetContentRegionAvail().y - 32;

    ImGui::BeginChild("ContentArea", ImVec2(0, available_height), true);

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

            if (DragAndDropFile(&item))
            {
                ImGui::PopID();
                ImGui::EndChild();
                return;
            }

            if (ImGui::BeginPopupContextItem()) {
                selected_item = &item;

                if (ImGui::MenuItem("Open")) {
                    if (selected_item->type == FOLDER) UpdateCurrentDirectory(selected_item);
                    else FilesManager::GetInstance().OpenFile(selected_item->absolute_path);
                }
                if (ImGui::MenuItem("Rename")) {
                    is_renaming = true;
                    item_to_rename = selected_item;
                    item_to_rename_path_string = item_to_rename->relative_path;

                    std::filesystem::path fullPath(selected_item->name);
                    std::string name_without_ext = fullPath.stem().string();
                    strncpy(rename_buffer, name_without_ext.c_str(), sizeof(rename_buffer));
                    rename_buffer[sizeof(rename_buffer) - 1] = '\0';
                }
                if (ImGui::MenuItem("Duplicate")) {
                    FilesManager::GetInstance().DuplicateFile(selected_item->absolute_path);
                    ImGui::EndPopup();
                    ImGui::PopID();
                    ImGui::EndChild();
                    return;
                }
                if (ImGui::MenuItem("Delete")) {
                    FilesManager::GetInstance().DeleteFile_(selected_item->absolute_path);
                    ImGui::EndPopup();
                    ImGui::PopID();
                    ImGui::EndChild();
                    return;
                }

                ImGui::EndPopup();
            }

            //renaming
            if (is_renaming && item_to_rename == &item) {
                ImGui::OpenPopup("Rename Item");
                is_renaming = false;
            }

            if (ImGui::BeginPopupModal("Rename Item", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("New name:");
                std::string extension = std::filesystem::path(item_to_rename->name).extension().string();
                std::string rename_label = extension + "##rename";
                bool confirm = ImGui::InputText(rename_label.c_str(), rename_buffer, IM_ARRAYSIZE(rename_buffer),
                    ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll);

                if (ImGui::Button("Confirm") || confirm) {
                    if (strlen(rename_buffer) > 0)
                    {
                        std::string new_name = rename_buffer;

                        if (item_to_rename->type != FOLDER && !new_name.ends_with(extension)) {
                            new_name += extension;
                        }

                        FilesManager::GetInstance().RenameFile(item_to_rename->absolute_path, new_name);

                        FilesManager::GetInstance().ProcessFromRoot();
                        UpdateCurrentDirectory(current_directory);

                        memset(rename_buffer, 0, sizeof(rename_buffer));
                        item_to_rename = nullptr;
                        item_to_rename_path_string = "";
                        ImGui::CloseCurrentPopup();
                        ImGui::EndPopup();
                        ImGui::PopID();
                        ImGui::EndChild();
                        return;
                    }
                }

                ImGui::SameLine();
                if (ImGui::Button("Cancel")) {
                    UpdateCurrentDirectory(current_directory);
                    memset(rename_buffer, 0, sizeof(rename_buffer));
                    item_to_rename = nullptr;
                    item_to_rename_path_string = "";
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }


            if (ImGui::IsItemHovered())
            {
                hovered_file_path = item.absolute_path;
            }
            else if (ImGui::IsWindowHovered()  && !ImGui::IsAnyItemHovered())
            {
                hovered_file_path = "";
            }


            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                if (item.type == FOLDER) {
                    UpdateCurrentDirectory(&item);
                }
                else if (item.type == SCENE)
                {
                    app->gui->save_scene_panel->RequestFocus();
                    app->gui->save_scene_panel->dragged_origin = true;
                    app->gui->save_scene_panel->dragged_path = item.relative_path;
                    app->gui->save_scene_panel->save_panel_action = SavePanelAction::OPEN_SCENE;
                }
                else if (item.type == ANIMATION)
                {
                    app->gui->animation_panel->RequestFocus();
                    app->gui->animation_panel->SetAnimation(item.relative_path);
                }
                else
                {
                    FilesManager::GetInstance().OpenFile(item.absolute_path);
                }
            }

            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + icon_size);
            ImGui::TextWrapped("%s", item.name.c_str());
            ImGui::PopTextWrapPos();

            ImGui::PopID();
            ImGui::NextColumn();
        }

        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(1) && !ImGui::IsAnyItemHovered())
        {
            ImGui::OpenPopup("ContextMenuEmpty");
        }

        if (ImGui::BeginPopup("ContextMenuEmpty")) {
            static char new_folder_name[128] = "";
            if (ImGui::InputText("##NewFolder", new_folder_name, IM_ARRAYSIZE(new_folder_name), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll) ||
                ImGui::Button("Create Folder")) {
                std::string new_path = current_directory->absolute_path + "\\" + new_folder_name;
                if (FilesManager::GetInstance().CreateFolder(new_path)) {
                    memset(new_folder_name, 0, sizeof(new_folder_name));
                    FilesManager::GetInstance().ProcessFromRoot();
                    UpdateCurrentDirectory(current_directory);
                    ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                    ImGui::Columns(1);
                    ImGui::EndChild();
                    return;
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    ImGui::Columns(1);
    ImGui::EndChild();
    if (ImGui::BeginDragDropTarget())
    {
        ImGui::PushStyleColor(ImGuiCol_DragDropTarget, IM_COL32(45, 85, 230, 255));
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_MULTI_NODE"))
        {
            GameObject* droppedGo = *(GameObject**)payload->Data;
            if (droppedGo)
            {
                std::string filePath = current_path_string + "\\" + droppedGo->GetName() + ".prefab";
                nlohmann::json prefab_data;
                if (Prefab::SaveAsPrefab(droppedGo->GetSharedPtr(), filePath, prefab_data))
                {
                    droppedGo->SetPrefabPath(filePath);
                    if (!droppedGo->GetPrefabPath().empty())
                    {
                        droppedGo->GetPrefabOriginalData() = prefab_data;
                        LOG(LogType::LOG_OK, "Prefab file saved to: %s", filePath.c_str());
                    }
                }
            }
        }
        ImGui::PopStyleColor();
        ImGui::EndDragDropTarget();
    }

    ImGui::Dummy({ 0, -100 });
    ImGui::Separator();
    ImGui::TextDisabled("%s", hovered_file_path.c_str());
}

void PanelProject::UpdateCurrentDirectory(const FileData* new_dir)
{
    if (new_dir) {
        current_path.clear();

        std::string path = new_dir->relative_path;
        size_t start = 0;
        size_t end = path.find('/');

        while (end != std::string::npos) {
            current_path.push_back(path.substr(start, end - start));
            start = end + 1;
            end = path.find('/', start);
        }
        current_path.push_back(path.substr(start));

        current_directory = new_dir;
        current_path_string = current_directory->relative_path;
    }
    else {
        const FileData* current = &FilesManager::GetInstance().GetFileData();
        for (const auto& path_segment : current_path) {
            if (path_segment == "Assets") continue;

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
        current_path_string = current_directory->relative_path;
    }
}

bool PanelProject::DragAndDropFile(const FileData* file_data)
{
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        std::string path = file_data->relative_path;
        std::string extension = std::filesystem::path(file_data->name).extension().string();
        std::string payload_type = "RESOURCE_FILE_" + FilesManager::GetInstance().GetFileTypeByExtension(extension);

        ImGui::SetDragDropPayload(
            payload_type.c_str(),
            path.c_str(),
            (path.length() + 1) * sizeof(char)
        );

        ImGui::Text("Moving \"%s\" file", file_data->name.c_str());
        ImGui::EndDragDropSource();
    }
    if (file_data->type == FileType::FOLDER)
    {
        return DropZone(file_data->relative_path);
    }
    return false;
}

bool PanelProject::DropZone(const std::string& target_path)
{
    if (ImGui::BeginDragDropTarget())
    {
        const ImGuiPayload* payload = nullptr;

        payload = ImGui::AcceptDragDropPayload("RESOURCE_FILE_FOLDER");
        if (payload)
        {
            const char* payload_path = static_cast<const char*>(payload->Data);
            std::string dragged_path(payload_path);

            FilesManager::GetInstance().MoveFile_(dragged_path, target_path);
            return true;
        }
        payload = ImGui::AcceptDragDropPayload("RESOURCE_FILE_IMAGE");
        if (payload)
        {
            const char* payload_path = static_cast<const char*>(payload->Data);
            std::string dragged_path(payload_path);

            FilesManager::GetInstance().MoveFile_(dragged_path, target_path);
            return true;
        }
        payload = ImGui::AcceptDragDropPayload("RESOURCE_FILE_ANIMATION");
        if (payload)
        {
            const char* payload_path = static_cast<const char*>(payload->Data);
            std::string dragged_path(payload_path);

            FilesManager::GetInstance().MoveFile_(dragged_path, target_path);
            return true;
        }
        payload = ImGui::AcceptDragDropPayload("RESOURCE_FILE_AUDIO");
        if (payload)
        {
            const char* payload_path = static_cast<const char*>(payload->Data);
            std::string dragged_path(payload_path);

            FilesManager::GetInstance().MoveFile_(dragged_path, target_path);
            return true;
        }
        payload = ImGui::AcceptDragDropPayload("RESOURCE_FILE_FONT");
        if (payload)
        {
            const char* payload_path = static_cast<const char*>(payload->Data);
            std::string dragged_path(payload_path);

            FilesManager::GetInstance().MoveFile_(dragged_path, target_path);
            return true;
        }
        payload = ImGui::AcceptDragDropPayload("RESOURCE_FILE_PARTICLES");
        if (payload)
        {
            const char* payload_path = static_cast<const char*>(payload->Data);
            std::string dragged_path(payload_path);

            FilesManager::GetInstance().MoveFile_(dragged_path, target_path);
            return true;
        }
        payload = ImGui::AcceptDragDropPayload("RESOURCE_FILE_SCENE");
        if (payload)
        {
            const char* payload_path = static_cast<const char*>(payload->Data);
            std::string dragged_path(payload_path);

            FilesManager::GetInstance().MoveFile_(dragged_path, target_path);
            return true;
        }
        payload = ImGui::AcceptDragDropPayload("RESOURCE_FILE_SCRIPT");
        if (payload)
        {
            const char* payload_path = static_cast<const char*>(payload->Data);
            std::string dragged_path(payload_path);

            FilesManager::GetInstance().MoveFile_(dragged_path, target_path);
            return true;
        }
        payload = ImGui::AcceptDragDropPayload("RESOURCE_FILE_TILED");
        if (payload)
        {
            const char* payload_path = static_cast<const char*>(payload->Data);
            std::string dragged_path(payload_path);

            FilesManager::GetInstance().MoveFile_(dragged_path, target_path);
            return true;
        }
        payload = ImGui::AcceptDragDropPayload("RESOURCE_FILE_PREFAB");
        if (payload)
        {
            const char* payload_path = static_cast<const char*>(payload->Data);
            std::string dragged_path(payload_path);

            FilesManager::GetInstance().MoveFile_(dragged_path, target_path);
            return true;
        }
        payload = ImGui::AcceptDragDropPayload("RESOURCE_FILE_");//other
        if (payload)
        {
            const char* payload_path = static_cast<const char*>(payload->Data);
            std::string dragged_path(payload_path);

            FilesManager::GetInstance().MoveFile_(dragged_path, target_path);
            return true;
        }
        ImGui::EndDragDropTarget();
    }
}
