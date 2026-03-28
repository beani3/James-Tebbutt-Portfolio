/** @file order.hpp */
#pragma once
#include <limits>
#include <stdint.h>

struct Order
{
public:
	uint32_t order{ std::numeric_limits<uint32_t>::max() };
};