#include "PanelAbout.h"

#include "GUI.h"
#include "App.h"

#include <imgui.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <gl/GL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <box2d/box2d.h>
#include <ImGuizmo.h>

#define IMGUIZMO_VERSION "v 1.89 WIP"

PanelAbout::PanelAbout(PanelType type, std::string name, bool enabled) : Panel(type, name, enabled)
{
}

PanelAbout::~PanelAbout()
{
}

bool PanelAbout::Update()
{
	if (bringToFront)
	{
		ImGui::SetNextWindowFocus();
		bringToFront = false;
	}

	bool ret = true;

	ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoCollapse;

	ImVec2 center(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.6f);
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(520, 765));
	if (ImGui::Begin("About", nullptr, window_flags)) {
		{
			// name engine + version
			ImGui::SeparatorText("ManLite 2D Engine v1.0");

			// made by
			ImGui::Text("Made by: ");
			ImGui::Text("   Luis Gonzalez (Github:");
			ImGui::SameLine();
			ImGui::TextLinkOpenURL("punto16", "https://github.com/punto16");
			ImGui::SameLine();
			ImGui::Text(")");

			ImGui::TextLinkOpenURL("Inspiration Project", "https://github.com/Shadow-Wizard-Games/TheOneEngine");


			// external libraries
			{
				ImGui::SeparatorText("External Libraries Used: ");

				// SDL2
				SDL_version sdlVersion;
				SDL_GetVersion(&sdlVersion);
				ImGui::Bullet(); ImGui::Text("SDL %d.%d.%d", sdlVersion.major, sdlVersion.minor, sdlVersion.patch);

				// SDL2_mixer
				const SDL_version* mixVersion = Mix_Linked_Version();
				ImGui::Bullet(); ImGui::Text("SDL2_mixer %d.%d.%d",
					mixVersion->major, mixVersion->minor, mixVersion->patch);

				// SDL2_ttf
				const SDL_version* ttfVersion = TTF_Linked_Version();
				ImGui::Bullet(); ImGui::Text("SDL2_ttf %d.%d.%d",
					ttfVersion->major, ttfVersion->minor, ttfVersion->patch);

				// OpenGL
				ImGui::Bullet(); ImGui::Text("OpenGL %s", glGetString(GL_VERSION));

				// Box2D
				ImGui::Bullet(); ImGui::Text("Box2D %d.%d.%d",
					b2_version.major, b2_version.minor, b2_version.revision);

				// ImGui
				ImGui::Bullet(); ImGui::Text("ImGui %s", IMGUI_VERSION);

				// ImGuizmo
				ImGui::Bullet(); ImGui::Text("ImGuizmo %s", IMGUIZMO_VERSION);
			}

			{
				ImGui::SeparatorText("License");

				// --TODO-- read directly the license document (?)
				ImGui::Text("MIT License");
				ImGui::Text("");
				ImGui::Text("Copyright(c) 2025 Luis Gonzalez (punto16)");
				ImGui::Text("");
				ImGui::Text("Permission is hereby granted, free of charge, to any person obtaining a copy");
				ImGui::Text("of this software and associated documentation files(the \"Software\"), to deal");
				ImGui::Text("in the Software without restriction, including without limitation the rights");
				ImGui::Text("to use, copy, modify, merge, publish, distribute, sublicense, and /or sell");
				ImGui::Text("copies of the Software, and to permit persons to whom the Software is");
				ImGui::Text("furnished to do so, subject to the following conditions :");
				ImGui::Text("");
				ImGui::Text("The above copyright notice and this permission notice shall be included in all");
				ImGui::Text("copies or substantial portions of the Software.");
				ImGui::Text("");
				ImGui::Text("THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR");
				ImGui::Text("IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,");
				ImGui::Text("FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE");
				ImGui::Text("AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER");
				ImGui::Text("LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,");
				ImGui::Text("OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE");
				ImGui::Text("SOFTWARE.");
			}

			ImGui::Separator();

			if (ImGui::Button("Close")) {
				this->SwitchState();
			}
		}
	}
	ImGui::End();

	return ret;
}
