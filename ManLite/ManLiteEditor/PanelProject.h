#ifndef __PANEL_PROJECT_H__
#define __PANEL_PROJECT_H__
#pragma once

#include "GuiPanel.h"
#include "imgui_internal.h"
#include "vector"
#include "string"

#define FOLDER_ICON		"[FOL] "
#define IMAGE_ICON		"[IMG] "
#define ANIMATION_ICON	"[ANI] "
#define AUDIO_ICON		"[AUD] "
#define FONT_ICON		"[FNT] "
#define PARTICLES_ICON  "[PRT] "
#define SCENE_ICON		"[SCN] "
#define SCRIPT_ICON		"[SCR] "
#define TILED_ICON		"[TLD] "
#define UNKNOWN_ICON	"[???] "

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

    const FileData* current_directory = nullptr;
    std::vector<std::string> current_path;

private:

};

#endif // !__PANEL_PROJECT_H__