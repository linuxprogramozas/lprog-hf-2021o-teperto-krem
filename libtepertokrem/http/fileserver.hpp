#pragma once
#include <functional>
#include <filesystem>
#include "handle.hpp"

namespace tepertokrem::http {
std::function<Handle(ResponseWriter, Request*)> FileServer(std::filesystem::path base_dir);
}