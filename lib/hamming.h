#pragma once

#include <cstdint>

namespace hamarc
{

std::uint16_t EncodeByte(unsigned char value);
unsigned char DecodeByte(std::uint16_t code);

}
