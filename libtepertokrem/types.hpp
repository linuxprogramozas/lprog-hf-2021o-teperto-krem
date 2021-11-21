/*! @file
 * @author Ondrejó András
 * @date 2021.11.21
 */
#pragma once
#include "utility/named_type.hpp"

namespace tepertokrem {
using ServerSocket = NamedType<int, struct ServerSocketTag>;
using ClientSocket = NamedType<int, struct ClientSocketTag>;
}