#include "tag_registry.hpp"
#include <loglet/loglet.hpp>

LOGLET_MODULE(tag_registry);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(tag_registry)

namespace tags {

void TagRegistry::register_tag(std::string const& name, std::string const& description,
                               std::string const& category) {
    if (mTags.find(name) != mTags.end()) {
        return;
    }

    if (mNextBitMask == 0) {
        ERRORF("Tag registry full (64 tags maximum)");
        return;
    }

    TagInfo info;
    info.name        = name;
    info.description = description;
    info.category    = category;
    info.mask        = Tag(mNextBitMask);

    mTags[name]       = info;
    mNameToMask[name] = Tag(mNextBitMask);
    mNextBitMask      = mNextBitMask << 1;
}

void TagRegistry::add_alias(std::string const& tag, std::string const& alias) {
    if (mTags.find(tag) == mTags.end()) {
        WARNF("Cannot add alias '%s' for unknown tag '%s'", alias.c_str(), tag.c_str());
        return;
    }

    mAliases[alias] = tag;
    mTags[tag].aliases.push_back(alias);
}

std::string TagRegistry::resolve_alias(std::string const& name) const {
    auto it = mAliases.find(name);
    return it != mAliases.end() ? it->second : name;
}

Tag TagRegistry::get_tag(std::string const& name) const {
    auto resolved = resolve_alias(name);
    auto it       = mNameToMask.find(resolved);

    if (it == mNameToMask.end()) {
        WARNF("Unknown tag '%s'", name.c_str());
        return Tag(0);
    }

    return it->second;
}

TagMask TagRegistry::get_tag(std::vector<std::string> const& names) const {
    uint64_t mask = 0;
    for (auto const& name : names) {
        mask |= get_tag(name).value;
    }
    return TagMask(mask);
}

std::string TagRegistry::mask_to_string(TagMask mask) const {
    if (mask.value == 0) return "";

    std::string result;
    for (auto const& pair : mTags) {
        if (pair.second.mask.value & mask.value) {
            if (!result.empty()) result += "+";
            result += pair.second.name;
        }
    }
    return result;
}

bool TagRegistry::exists(std::string const& name) const {
    auto resolved = resolve_alias(name);
    return mNameToMask.find(resolved) != mNameToMask.end();
}

std::vector<TagInfo> TagRegistry::list_tags() const {
    std::vector<TagInfo> result;
    for (auto const& pair : mTags) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<TagInfo> TagRegistry::list_tags_by_category(std::string const& category) const {
    std::vector<TagInfo> result;
    for (auto const& pair : mTags) {
        if (pair.second.category == category) {
            result.push_back(pair.second);
        }
    }
    return result;
}

void TagRegistry::dump() const {
    INFOF("Registered tags:");
    for (auto const& pair : mTags) {
        auto const& info = pair.second;
        INFOF("  %-15s - %s (category: %s, mask: 0x%016llX)", info.name.c_str(),
              info.description.c_str(), info.category.c_str(),
              static_cast<unsigned long long>(info.mask.value));
        for (auto const& alias : info.aliases) {
            INFOF("    alias: %s", alias.c_str());
        }
    }
}

TagRegistry& registry() {
    return global_tag_registry();
}

void register_tag(std::string const& name, std::string const& description,
                 std::string const& category) {
    registry().register_tag(name, description, category);
}

void add_alias(std::string const& tag, std::string const& alias) {
    registry().add_alias(tag, alias);
}

Tag get(std::string const& name) {
    return registry().get_tag(name);
}

TagMask get(std::vector<std::string> const& names) {
    return registry().get_tag(names);
}

std::string to_string(TagMask mask) {
    return registry().mask_to_string(mask);
}

bool exists(std::string const& name) {
    return registry().exists(name);
}

}  // namespace tags

tags::TagRegistry& global_tag_registry() {
    static tags::TagRegistry registry;
    return registry;
}
