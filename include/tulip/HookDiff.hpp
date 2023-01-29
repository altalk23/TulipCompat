#pragma once

#include "Diff.hpp"

#include <tulip/TulipHook.hpp>

namespace tulip::compat {
	struct HookDiff {
		void* m_address;
		tulip::hook::HandlerMetadata m_metadata;
		Diff m_diff;
	};
}