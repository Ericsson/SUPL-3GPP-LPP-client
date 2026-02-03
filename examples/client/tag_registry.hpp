#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#ifndef TAG_MAX_COUNT
#define TAG_MAX_COUNT 64
#endif

namespace tags {

struct Tag {
#if TAG_MAX_COUNT <= 64
    uint64_t value;
    Tag() : value(0) {}
    explicit Tag(uint64_t v) : value(v) {}
    operator uint64_t() const { return value; }
    bool operator==(Tag other) const { return value == other.value; }
    bool operator!=(Tag other) const { return value != other.value; }
#else
#error "TAG_MAX_COUNT > 64 not yet implemented"
#endif
};

struct TagMask {
#if TAG_MAX_COUNT <= 64
    uint64_t value;
    TagMask() : value(0) {}
    explicit TagMask(uint64_t v) : value(v) {}
    explicit TagMask(Tag tag) : value(tag.value) {}
    operator uint64_t() const { return value; }

    TagMask operator|(TagMask other) const { return TagMask(value | other.value); }
    TagMask operator&(TagMask other) const { return TagMask(value & other.value); }
    TagMask& operator|=(TagMask other) { value |= other.value; return *this; }
    TagMask& operator&=(TagMask other) { value &= other.value; return *this; }
    bool operator==(TagMask other) const { return value == other.value; }
    bool operator!=(TagMask other) const { return value != other.value; }

    TagMask operator|(Tag tag) const { return TagMask(value | tag.value); }
    TagMask& operator|=(Tag tag) { value |= tag.value; return *this; }

    static bool filter(TagMask include, TagMask exclude, Tag tag) {
        return tag.value == 0 ||
               (((include.value & tag.value) || include.value == 0) && !(exclude.value & tag.value));
    }
#else
#error "TAG_MAX_COUNT > 64 not yet implemented"
#endif
};

struct TagInfo {
    std::string              name;
    std::string              description;
    std::string              category;
    std::vector<std::string> aliases;
    Tag                      mask;
};

class TagRegistry {
public:
    TagRegistry() : mNextBitMask(1) {}

    void register_tag(std::string const& name, std::string const& description,
                     std::string const& category = "general");

    void add_alias(std::string const& tag, std::string const& alias);

    Tag get_tag(std::string const& name) const;
    TagMask get_tag(std::vector<std::string> const& names) const;

    std::string mask_to_string(TagMask mask) const;

    bool exists(std::string const& name) const;

    std::vector<TagInfo> list_tags() const;
    std::vector<TagInfo> list_tags_by_category(std::string const& category) const;

    void dump() const;

private:
    std::string resolve_alias(std::string const& name) const;

    uint64_t                                     mNextBitMask;
    std::unordered_map<std::string, TagInfo>     mTags;
    std::unordered_map<std::string, Tag>         mNameToMask;
    std::unordered_map<std::string, std::string> mAliases;
};

TagRegistry& registry();

void register_tag(std::string const& name, std::string const& description,
                 std::string const& category = "general");

void add_alias(std::string const& tag, std::string const& alias);

Tag get(std::string const& name);

TagMask get(std::vector<std::string> const& names);

std::string to_string(TagMask mask);

bool exists(std::string const& name);

}  // namespace tags

tags::TagRegistry& global_tag_registry();
