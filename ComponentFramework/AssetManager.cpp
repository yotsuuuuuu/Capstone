#include "AssetManager.h"

using json = nlohmann::json;

bool AssetManager::Write(const std::string& name_)
{
    std::ofstream jsontest(name_);
    if (!jsontest.is_open())
    {
        return false;
    }
    jsontest << "this is a test for json";

    jsontest.close();
    return true;
}

