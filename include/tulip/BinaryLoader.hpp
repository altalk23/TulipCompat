#pragma once

#include <ghc/filesystem.hpp>
#include <memory>

namespace tulip::compat {
	class BinaryLoader {
		class Impl;
		std::unique_ptr<Impl> m_impl;

	public:
		BinaryLoader(ghc::filesystem::path const& path);
		~BinaryLoader();

		static BinaryLoader* get();
	};
}