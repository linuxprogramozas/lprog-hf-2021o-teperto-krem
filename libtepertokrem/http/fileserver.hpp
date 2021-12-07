#pragma once
#include <functional>
#include <filesystem>
#include "handle.hpp"

namespace tepertokrem::http {
/**
 * Elore elkeszitett fajlkiszolgalo fuggveny
 * Nem kozvetlenul hasznalhato fel http::Handle-kent, hanem a meghivasa ad vissza
 * egy fuggvenyt ami mar http::Handle-nek jo
 * @param base_dir Directory amin belul keresse a fajlokat
 */
std::function<Handle(ResponseWriter, Request*)> FileServer(std::filesystem::path base_dir);
}