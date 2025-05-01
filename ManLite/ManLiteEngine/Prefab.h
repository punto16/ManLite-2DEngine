#ifndef __PREFAB_H__
#define __PREFAB_H__

#include "GameObject.h"
#include <string>
#include "nlohmann/json.hpp"

class Prefab
{
public:
    static bool SaveAsPrefab(std::shared_ptr<GameObject> gameObject, const std::string& filePath);
    static std::shared_ptr<GameObject> Instantiate(const std::string& filePath, std::shared_ptr<GameObject> parent);
    static void RemoveIDs(nlohmann::json& json);

private:
};

#endif // __PREFAB_H__