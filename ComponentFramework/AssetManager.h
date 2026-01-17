#pragma once
#include <fstream>
#include <memory>
#include <map>
#include <string>
#include <json.hpp>
// link to examples and documantation https://github.com/nlohmann/json?tab=readme-ov-file#examples
//link to exception list https://json.nlohmann.me/home/exceptions/
//helpful video https://www.youtube.com/watch?v=NuWQp_uAvwo&t=119s
class AssetManager
{
public:
	bool Write(const std::string& name_);
	//bool Write(const std::string& name_, Matrix4* mat_);
};

