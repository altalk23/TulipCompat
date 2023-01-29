#include <BinaryLoader.hpp>
#include <Geode/loader/Mod.hpp>

using namespace tulip::compat;
USE_GEODE_NAMESPACE();

$on_mod(Enabled) {
	ghc::filesystem::create_directories(
	    Mod::get()->getSaveDir() /
	    ghc::filesystem::path(
	        Mod::get()->getSettingValue<std::string>("binary-folder"),
	        ghc::filesystem::path::format::generic_format
	    )
	);
	(void)BinaryLoader::get();
}