#ifndef __PANEL_PROJECT_H__
#define __PANEL_PROJECT_H__
#pragma once

#include "GuiPanel.h"

#include "Defs.h"

#include "imgui_internal.h"
#include "vector"
#include "string"

class FileData;

class PanelProject : public Panel
{
public:
	PanelProject(PanelType type, std::string name, bool enabled);
	~PanelProject();

	bool Update();

    void RenderFileTree(const FileData& file_data, int depth = 0);
    void RenderBreadcrumbs();
    void RenderContentGrid();
    void UpdateCurrentDirectory(const FileData* new_dir);

    //returns true if it completed a drop file
    bool DragAndDropFile(const FileData* file_data);
    //returns true if it completed a drop file
    bool DropZone(const std::string& target_path);

    const FileData* current_directory = nullptr;
    std::vector<std::string> current_path;
    const FileData* selected_item;
    std::string hovered_file_path = "";
    std::string current_path_string = "";

private:

    uint unknown_icon = 0;
    uint folder_icon = 0;
    uint image_icon = 0;
    uint animation_icon = 0;
    uint audio_icon = 0;
    uint font_icon = 0;
    uint particles_icon = 0;
    uint scene_icon = 0;
    uint script_icon = 0;
    uint tiled_icon = 0;
    uint prefab_icon = 0;

    bool is_renaming = false;
    char rename_buffer[256] = { 0 };
    const FileData* item_to_rename = nullptr;
    std::string item_to_rename_path_string = "";
};

#endif // !__PANEL_PROJECT_H__