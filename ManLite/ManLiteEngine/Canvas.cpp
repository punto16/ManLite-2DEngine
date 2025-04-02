#include "Canvas.h"

#include "UIElement.h"
#include "GameObject.h"

#include "ImageUI.h"
#include "ButtonImageUI.h"
#include "SliderUI.h"
#include "CheckBoxUI.h"
#include "TextUI.h"

Canvas::Canvas(std::weak_ptr<GameObject> container_go, std::string name, bool enable) : 
	Component(container_go, ComponentType::Canvas, name, enable)
{
}

Canvas::Canvas(const Canvas& component_to_copy, std::shared_ptr<GameObject> container_go) :
	Component(component_to_copy, container_go)
{
	for (auto& ui_element : component_to_copy.ui_elements)
	{
		switch (ui_element->GetType())
		{
		case UIElementType::Image:
		{
			AddCopiedUIElement<ImageUI>(*dynamic_cast<const ImageUI*>(ui_element.get()));
			break;
		}
		case UIElementType::ButtonImage:
		{
			AddCopiedUIElement<ButtonImageUI>(*dynamic_cast<const ButtonImageUI*>(ui_element.get()));
			break;
		}
		case UIElementType::Slider:
		{
			AddCopiedUIElement<SliderUI>(*dynamic_cast<const SliderUI*>(ui_element.get()));
			break;
		}
		case UIElementType::CheckBox:
		{
			AddCopiedUIElement<CheckBoxUI>(*dynamic_cast<const CheckBoxUI*>(ui_element.get()));
			break;
		}
		case UIElementType::Text:
		{
			AddCopiedUIElement<TextUI>(*dynamic_cast<const TextUI*>(ui_element.get()));
			break;
		}
		case UIElementType::Unkown:
			break;
		default:
			break;
		}
	}
}

Canvas::~Canvas()
{
}

bool Canvas::Init()
{
	bool ret = true;

	for (const auto& item : ui_elements)
		if (item->IsEnabled())
			if (!item->Init()) return false;

	return ret;
}

bool Canvas::Update(float dt)
{
	bool ret = true;

	for (const auto& item : ui_elements)
		if (item->IsEnabled())
			if (!item->Update(dt)) return false;

	return ret;
}

void Canvas::Draw()
{
	for (const auto& item : ui_elements)
		if (item->IsEnabled()) item->Draw();
}

bool Canvas::Pause()
{
	bool ret = true;

	for (const auto& item : ui_elements)
		if (item->IsEnabled())
			if (!item->Pause()) return false;

	return ret;
}

bool Canvas::Unpause()
{
	bool ret = true;

	for (const auto& item : ui_elements)
		if (item->IsEnabled())
			if (!item->Unpause()) return false;

	return ret;
}

bool Canvas::RemoveItemUI(unsigned int id)
{
	bool ret = true;

	for (auto it = ui_elements.begin(); it != ui_elements.end(); ++it)
	{
		if ((*it)->GetID() == id)
		{
			it = ui_elements.erase(it);
			break;
		}
	}

	return ret;
}

nlohmann::json Canvas::SaveComponent()
{
	nlohmann::json componentJSON;
	//component generic
	componentJSON["ContainerGOID"] = this->container_go.lock()->GetID();
	componentJSON["ComponentID"] = component_id;
	componentJSON["ComponentName"] = name;
	componentJSON["ComponentType"] = (int)type;
	componentJSON["Enabled"] = enabled;

	//component spcecific
	if (!this->ui_elements.empty())
	{
		nlohmann::json uielementsJSON;
		for (const auto& ui_element : this->ui_elements)
		{
			uielementsJSON.push_back(ui_element->SaveUIElement());
		}
		componentJSON["UIElements"] = uielementsJSON;
	}

	return componentJSON;
}

void Canvas::LoadComponent(const nlohmann::json& componentJSON)
{
	if (componentJSON.contains("ComponentID")) component_id = componentJSON["ComponentID"];
	if (componentJSON.contains("ComponentName")) name = componentJSON["ComponentName"];
	if (componentJSON.contains("ComponentType")) type = (ComponentType)componentJSON["ComponentType"];
	if (componentJSON.contains("Enabled")) enabled = componentJSON["Enabled"];

	if (componentJSON.contains("UiElements"))
	{
		const nlohmann::json& uiElementsJSON = componentJSON["UiElements"];

		for (auto& item : uiElementsJSON)
		{
			if (item["UIElementType"] == (int)UIElementType::Image)
			{
				GetUIElement<ImageUI>(AddUIElement<ImageUI>(""))->LoadUIElement(item);
			}
			if (item["UIElementType"] == (int)UIElementType::ButtonImage)
			{
				GetUIElement<ButtonImageUI>(AddUIElement<ButtonImageUI>(""))->LoadUIElement(item);
			}
			if (item["UIElementType"] == (int)UIElementType::Slider)
			{
				GetUIElement<SliderUI>(AddUIElement<SliderUI>(""))->LoadUIElement(item);
			}
			if (item["UIElementType"] == (int)UIElementType::CheckBox)
			{
				GetUIElement<CheckBoxUI>(AddUIElement<CheckBoxUI>(""))->LoadUIElement(item);
			}
			if (item["UIElementType"] == (int)UIElementType::Text)
			{
				GetUIElement<TextUI>(AddUIElement<TextUI>(""))->LoadUIElement(item);
			}
			if (item["UIElementType"] == (int)UIElementType::Unkown)
			{
				//default
			}
		}
	}
}

ML_Rect Canvas::GetUVs(ML_Rect section, int w, int h)
{
	if (w > 0 && h > 0)
	{
		float u1 = static_cast<float>(section.x) / w;
		float u2 = static_cast<float>(section.x + section.w) / w;
		float v1 = static_cast<float>(h - (section.y + section.h)) / h;
		float v2 = static_cast<float>(h - section.y) / h;
		return ML_Rect(u1, v1, u2, v2);
	}
	else
	{
		return ML_Rect(0, 0, 1, 1);
	}
}
