#include "PanelConsole.h"

#include "GUI.h"
#include "../ManLiteEngine/Defs.h"
#include "../ManLiteEngine/Log.h"
#include "../ManLiteEngine/EngineCore.h"

#include <imgui.h>

PanelConsole::PanelConsole(PanelType type, std::string name, bool enabled) : Panel(type, name, enabled)
{
}

PanelConsole::~PanelConsole()
{
}

bool PanelConsole::Update()
{
	bool ret = true;

	ImGuiWindowFlags consoleFlags = 0;
	consoleFlags |= ImGuiWindowFlags_NoFocusOnAppearing;
	if (ImGui::Begin(name.c_str(), &enabled))
	{
		if (ImGui::Button("Clear"))
			engine->CleanLogs();

		ImGui::SameLine();

		static bool clearOnPlay = false;
		ImGui::Checkbox("Clear on Play", &clearOnPlay);

		ImGui::Separator();

		ImGuiWindowFlags scrollFlags = 0;
		scrollFlags |= ImGuiWindowFlags_HorizontalScrollbar;
		scrollFlags |= ImGuiWindowFlags_AlwaysVerticalScrollbar;

		if (ImGui::BeginChild("Scrollbar", ImVec2(0, 0), false, scrollFlags))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 15));
			std::string logType;
			ImVec4 color = ImVec4(1, 1, 1, 1);

			for (const auto& log : engine->GetLogs())
			{
				switch (log.type)
				{
				case LogType::LOG_INFO:
					logType = "";
					color = ImVec4(1, 1, 1, 1);
					break;

				case LogType::LOG_ASSIMP:
					logType = "[ASSIMP] ";
					color = ImVec4(1, 1, 1, 1);
					break;

				case LogType::LOG_AUDIO:
					logType = "[AUDIO] ";
					color = ImVec4(1, 1, 1, 1);
					break;

				case LogType::LOG_MONO:
					logType = "[MONO] ";
					color = ImVec4(1, 1, 1, 1);
					break;

				case LogType::LOG_OK:
					logType = "[OK] ";
					color = ImVec4(0, 1, 0, 1);
					break;

				case LogType::LOG_WARNING:
					logType = "[WARNING] ";
					color = ImVec4(1, 1, 0, 1);
					break;

				case LogType::LOG_ERROR:
					logType = "[ERROR] ";
					color = ImVec4(1, 0, 0, 1);
					break;
				}

				if (log.message[0] == '-')
					logType.insert(0, "\t");

				ImGui::PushStyleColor(ImGuiCol_Text, color);
				ImGui::Text(logType.c_str());
				ImGui::PopStyleColor();

				ImGui::SameLine();
				ImGui::Text(log.message.c_str());
			}

			ImGui::PopStyleVar();

			if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
				ImGui::SetScrollHereY(1.0f);

		}
		ImGui::EndChild();

		if (clearOnPlay) engine->CleanLogs();
	}
	ImGui::End();

	return ret;
}
