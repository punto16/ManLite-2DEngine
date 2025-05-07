#include "Prefab.h"
#include <fstream>
#include "Log.h"

#include "EngineCore.h"
#include "SceneManagerEM.h"
#include "GameObject.h"
#include "Layer.h"

bool Prefab::SaveAsPrefab(std::shared_ptr<GameObject> gameObject, const std::string& filePath_const)
{
    std::string filePath = filePath_const;
    std::replace(filePath.begin(), filePath.end(), '/', '\\');

    nlohmann::json prefabJson = gameObject->SaveGameObject();
    RemoveIDs(prefabJson);

    std::ofstream file(filePath);
    if (!file.is_open()) {
        LOG(LogType::LOG_ERROR, "Failed to save prefab to %s", filePath.c_str());
        return false;
    }

    file << prefabJson.dump(4);
    file.close();

    return true;
}

std::shared_ptr<GameObject> Prefab::Instantiate(const std::string& filePath_const, std::shared_ptr<GameObject> parent, bool runtime)
{
    std::string filePath = filePath_const;
    std::replace(filePath.begin(), filePath.end(), '/', '\\');

    std::ifstream file(filePath);
    if (!file.is_open())
    {
        LOG(LogType::LOG_ERROR, "Failed to load prefab from %s", filePath.c_str());
        return nullptr;
    }

    nlohmann::json prefabJson;
    try
    {
        file >> prefabJson;
    }
    catch (const std::exception& e)
    {
        LOG(LogType::LOG_ERROR, "Failed to parse prefab JSON: %s", e.what());
        return nullptr;
    }

    auto instance = std::make_shared<GameObject>(parent, "PrefabInstance", true);
    if (instance)
    {
        instance->LoadGameObject(prefabJson);
        nlohmann::json cleanOriginal = prefabJson;
        RemoveIDs(cleanOriginal);
        instance->GetPrefabOriginalData() = cleanOriginal;
        instance->SetPrefabPath(filePath);
        instance->SetName(instance->GetName(), true);
        if (parent.get() != nullptr) parent->AddChild(instance);

        //layers handle
        if (!runtime)
        {
            engine->scene_manager_em->GetCurrentScene().GetSceneLayers()[0]->AddChild(instance);
            instance->SetParentLayer(engine->scene_manager_em->GetCurrentScene().GetSceneLayers()[0]);
        }
    }

    return instance;
}

void Prefab::RemoveIDs(nlohmann::json& json)
{
    if (json.contains("OriginalPrefabPath"))
    {
        json.erase("OriginalPrefabPath");
    }
    if (json.contains("ID"))
    {
        json.erase("ID");
    }
    if (json.contains("ParentID"))
    {
        json.erase("ParentID");
    }
    if (json.contains("ParentLayerID"))
    {
        json.erase("ParentLayerID");
    }

    if (json.contains("Components"))
    {
        for (auto& component : json["Components"])
        {
            if (component.contains("ComponentID"))
                component.erase("ComponentID");
            if (component.contains("ContainerGOID"))
                component.erase("ContainerGOID");

            if (component.contains("ComponentType"))
                if (component["ComponentType"] == (int)ComponentType::Canvas)
                    if (component.contains("UIElements"))
                        for (auto& ui_element : component["UIElements"])
                        {
                            if (ui_element.contains("ContainerCanvasID"))
                                ui_element.erase("ContainerCanvasID");
                            if (ui_element.contains("UIElementID"))
                                ui_element.erase("UIElementID");
                        }
        }
    }

    if (json.contains("GameObjects"))
    {
        for (auto& child : json["GameObjects"]) {
            RemoveIDs(child);
        }
    }
}