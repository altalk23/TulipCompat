#include <Geode/modify/Addresses.hpp>
#include <MappingTracker.hpp>
#include <algorithm>
#include <optional>

using namespace tulip::compat;
USE_GEODE_NAMESPACE();

class MappingTracker::Impl {
public:
	std::vector<uint8_t> m_original;
	std::vector<uint8_t> m_modified;

	void* m_address;
	size_t m_size;

	Impl(void* address, size_t size);
	~Impl();

	void finalize();

	std::vector<Diff> getDifferences();

	std::vector<HookDiff> getHookDifferences();
};

MappingTracker::Impl::Impl(void* address, size_t size) :
    m_address(address),
    m_size(size) {
	std::copy_n(static_cast<uint8_t*>(m_address), m_size, std::back_inserter(m_original));
}

MappingTracker::Impl::~Impl() {}

void MappingTracker::Impl::finalize() {
	std::copy_n(static_cast<uint8_t*>(m_address), m_size, std::back_inserter(m_modified));
}

std::vector<Diff> MappingTracker::Impl::getDifferences() {
	if (m_modified.size() != m_original.size()) {
		return {};
	}
	std::vector<Diff> ret;
	std::optional<Diff> currentDiff;
	for (intptr_t i = 0; i < m_size; ++i) {
		if (!currentDiff && m_original[i] != m_modified[i]) {
			currentDiff = Diff {
				.m_address = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_address) + i),
				.m_size = 0,
			};
		}

		if (m_original[i] != m_modified[i]) {
			currentDiff->m_original.push_back(m_original[i]);
			currentDiff->m_modified.push_back(m_modified[i]);
			++currentDiff->m_size;
		}
		else if (currentDiff && m_original[i] == m_modified[i]) {
			ret.push_back(std::move(*currentDiff));
			currentDiff.reset();
		}
	}
	if (currentDiff) {
		ret.push_back(std::move(*currentDiff));
		currentDiff.reset();
	}
	return ret;
}

std::vector<HookDiff> MappingTracker::Impl::getHookDifferences() {
	if (m_modified.size() != m_original.size()) {
		return {};
	}
	std::vector<HookDiff> ret;
	auto diffs = this->getDifferences();
	for (auto& diff : diffs) {
		auto address = diff.m_address;
		auto res = modifier::handlerMetadataForAddress(reinterpret_cast<uintptr_t>(address));
		if (res) { // the difference exists in the function address position
#if defined(GEODE_IS_MACOS) || defined(GEODE_IS_WINDOWS)
			uintptr_t hook;
			if (*reinterpret_cast<uint8_t*>(address) == 0xe9) {
				auto offset = *reinterpret_cast<int32_t*>(reinterpret_cast<uintptr_t>(address) + 1);
				hook = reinterpret_cast<uintptr_t>(address) + 5 + offset;
			}
			else if (*reinterpret_cast<uint16_t*>(address) == 0x25ff) {
				hook = *reinterpret_cast<int64_t*>(reinterpret_cast<uintptr_t>(address) + 6);
			}
#endif
			ret.push_back(HookDiff {
			    .m_address = reinterpret_cast<void*>(hook),
			    .m_metadata = std::move(res.unwrap()),
			    .m_diff = std::move(diff),
			});
		}
	}
	return ret;
}

MappingTracker::MappingTracker(void* address, size_t size) :
    m_impl(new Impl(address, size)) {}

MappingTracker::~MappingTracker() {}

void MappingTracker::finalize() {
	return m_impl->finalize();
}

std::vector<Diff> MappingTracker::getDifferences() {
	return m_impl->getDifferences();
}

std::vector<HookDiff> MappingTracker::getHookDifferences() {
	return m_impl->getHookDifferences();
}