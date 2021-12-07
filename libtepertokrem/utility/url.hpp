#pragma once
#include <string_view>

namespace tepertokrem {
/*
 * Szazalek kodolt url dekodolasa
 */
std::string DecodeUrl(std::string_view view);
}