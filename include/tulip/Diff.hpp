#pragma once

#include <stdint.h>
#include <vector>

namespace tulip::compat {
	struct Diff {
		void* m_address;
		size_t m_size;
		std::vector<uint8_t> m_original;
		std::vector<uint8_t> m_modified;
	};
}