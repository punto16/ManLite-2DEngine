#ifndef __CANVAS_H__
#define __CANVAS_H__
#pragma once

#include "Component.h"

#include "Defs.h"

#include "memory"
#include "vector"

class UIElement;

class Canvas : public Component  {
public:
    Canvas(std::weak_ptr<GameObject> container_go, std::string name = "Canvas", bool enable = true);
    Canvas(const Canvas& component_to_copy, std::shared_ptr<GameObject> container_go);
    ~Canvas();

    bool Init() override;

    bool Update(float dt) override;
    void Draw() override;

    bool Pause() override;
    bool Unpause() override;

    int GetUIElementOrderPosition(UIElement* ui_element);

    //ui elements 
    template <typename TUI>
    unsigned int AddUIElement(std::string file_path)
    {
        std::unique_ptr<UIElement> new_uielement = std::make_unique<TUI>(container_go.lock(), file_path);
        unsigned int tempID = new_uielement->GetID();
        ui_elements.push_back(std::move(new_uielement));
    
        return tempID;
    }

    template <typename TUI>
    unsigned int AddCopiedUIElement(const TUI& ref)
    {
        std::unique_ptr<UIElement> new_uielement = std::make_unique<TUI>(ref, container_go.lock());
        unsigned int tempID = new_uielement->GetID();
        ui_elements.push_back(std::move(new_uielement));

        return tempID;
    }

    template <typename TUI>
    TUI* GetUIElement(unsigned int id)
    {
        for (const auto& ui_element : ui_elements)
        {
            if (dynamic_cast<TUI*>(ui_element.get()))
            {
                if (ui_element.get()->GetID() == id)
                {
                    return static_cast<TUI*>(ui_element.get());
                }
            }
        }
        return nullptr;
    }

    bool RemoveItemUI(unsigned int id);

    //serialization
    nlohmann::json SaveComponent() override;
    void LoadComponent(const nlohmann::json& componentJSON) override;

    //utils
    static ML_Rect GetUVs(ML_Rect section, int w, int h);

    //getters // setters
    std::vector<std::unique_ptr<UIElement>>& GetUIElements() { return ui_elements; }

private:
    std::vector<std::unique_ptr<UIElement>> ui_elements;
};

#endif // !__CANVAS_H__