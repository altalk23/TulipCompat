#pragma once

#include "Diff.hpp"
#include "HookDiff.hpp"

#include <memory>
#include <stdint.h>
#include <vector>

namespace tulip::compat {
	class MappingTracker final {
		class Impl;
		std::unique_ptr<Impl> m_impl;

	public:
		MappingTracker(void* address, size_t size);
		~MappingTracker();

		void finalize();

		std::vector<Diff> getDifferences();

		std::vector<HookDiff> getHookDifferences();
	};
}