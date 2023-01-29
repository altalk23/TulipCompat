#include <BinaryLoader.hpp>
#include <Geode/loader/Log.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/platform/platform.hpp>
#include <MappingTracker.hpp>
#include <chrono>
#include <map>
#include <thread>

#ifdef GEODE_IS_MACOS
#include <dlfcn.h>
#endif

using namespace tulip::compat;
USE_GEODE_NAMESPACE();

class BinaryLoader::Impl {
public:
	Impl(ghc::filesystem::path const& path);
	~Impl();

	ghc::filesystem::path m_path;
};

namespace {
	void load(std::string const& path) {
		// making it lazy since this is a compat thing
		if (path.find(".dylib") != std::string::npos) {
			using namespace std::chrono_literals;

#ifdef GEODE_IS_MACOS
			auto binary = dlopen(path.c_str(), RTLD_NOW);
			std::this_thread::sleep_for(100ms); // im not shameful
#elif GEODE_IS_WINDOWS
			auto test = LoadLibraryA(path.c_str());
			std::this_thread::sleep_for(5000ms); // im not shameful
#endif
		}
	}
}

BinaryLoader::Impl::Impl(ghc::filesystem::path const& path) :
    m_path(path) {
	for (auto const& entry : ghc::filesystem::directory_iterator(path)) {
		log::info("Loading {}", entry.path().string());
#ifdef GEODE_IS_MACOS
		auto gdMap = MappingTracker(reinterpret_cast<void*>(base::get() + 0x1980), 0x4883b2);
#elif GEODE_IS_WINDOWS
		auto gdMap = MappingTracker(reinterpret_cast<void*>(base::get() + 0x1000), 0x280e00);
#endif

		auto loading = std::thread(&load, entry.path().string());
		loading.join();

		gdMap.finalize();

		auto diffs = gdMap.getDifferences();
		log::info("Found {} differences", diffs.size());

		for (auto& [address, size, original, modified] : diffs) {
			log::info("Has difference at address {} size {}", address, size);
		}

		auto hookDiffs = gdMap.getHookDifferences();
		log::info("Found {} hook differences", hookDiffs.size());

		for (auto& [hook, metadata, diff] : hookDiffs) {
			log::info("Has hook at address {}", hook);

			auto res = Mod::get()->patch(diff.m_address, diff.m_original);
			if (!res) {
				log::warn("Reverting the hook {} failed", diff.m_address);
			}

			auto res2 = Mod::get()->addHook(Hook::create(
			    Mod::get(), diff.m_address, hook, "", metadata, tulip::hook::HookMetadata()
			));
		}
	}
}

BinaryLoader::Impl::~Impl() {}

BinaryLoader::BinaryLoader(ghc::filesystem::path const& path) :
    m_impl(new Impl(path)) {}

BinaryLoader::~BinaryLoader() {}

BinaryLoader* BinaryLoader::get() {
	static auto s_ret = BinaryLoader(
	    Mod::get()->getSaveDir() /
	    ghc::filesystem::path(
	        Mod::get()->getSettingValue<std::string>("binary-folder"),
	        ghc::filesystem::path::format::generic_format
	    )
	);
	return &s_ret;
}