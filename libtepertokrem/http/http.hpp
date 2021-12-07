/*! @file
 * @author Ondrejó András
 * @date 2021.11.21
 */
#pragma once
#include "../stream/stream2.hpp"
#include "../types.hpp"

namespace tepertokrem {
/**
 * Http kapcsolat coroutine
 * Http/1.1-nek annyiban megfelel, hogy a kapcsolatokat eletben tartja, ha
 * ezt a kliens igenyli (Connection: keep-alive)
 * @param ssock server socket amin ahol a bejovo kapcsolat van
 * @return client socket coroutineba agyazva
 */
Stream2 Http(ServerSocket ssock);
}