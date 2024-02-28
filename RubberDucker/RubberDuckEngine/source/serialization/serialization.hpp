#pragma once
#include <nlohmann/json.hpp>
#include <rttr/type.h>

#include <optional>

namespace RDE {
namespace Serialization {

inline bool isAtomicType(const rttr::type& type)
{
    return type.is_arithmetic() || type.is_enumeration() || type == rttr::type::get<std::string>();
}

nlohmann::ordered_json serialize(const rttr::instance& instance, uint32_t level = 0);
nlohmann::ordered_json serializeAtomic(rttr::string_view name,
                                       const rttr::variant& variant,
                                       nlohmann::ordered_json& json);

} // namespace Serialization
} // namespace RDE
