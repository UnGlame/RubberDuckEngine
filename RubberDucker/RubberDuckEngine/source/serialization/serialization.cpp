#include "precompiled/pch.hpp"

#include "serialization.hpp"

namespace RDE {
namespace Serialization {

nlohmann::ordered_json serialize(const rttr::instance& instance, uint32_t level)
{
    const auto type = instance.get_type();
    const auto properties = type.get_properties();
    nlohmann::ordered_json json{};

    for (const auto& property : properties) {
        const auto propName = property.get_name();
        const auto propValue = property.get_value(instance);
        const auto propType = propValue.get_type();

        if (!propValue) {
            RDELOG_ERROR("Could not obtain property value {} {} of instance!", propType.get_name(), propName)
            continue;
        }
        if (isAtomicType(propType)) {
            serializeAtomic(propName, propValue, json);
        } else {
            json[propName] = serialize(propValue, level + 1);
        }
    }

    return json;
}

nlohmann::ordered_json serializeAtomic(rttr::string_view name,
                                       const rttr::variant& variant,
                                       nlohmann::ordered_json& json)
{
    const auto& type = variant.get_type();

    if (type.is_arithmetic()) {
        if (type == rttr::type::get<bool>()) {
            json[name] = variant.to_bool();
        } else if (type == rttr::type::get<int8_t>()) {
            json[name] = variant.to_int8();
        } else if (type == rttr::type::get<int16_t>()) {
            json[name] = variant.to_int16();
        } else if (type == rttr::type::get<int32_t>()) {
            json[name] = variant.to_int32();
        } else if (type == rttr::type::get<int64_t>()) {
            json[name] = variant.to_int64();
        } else if (type == rttr::type::get<uint8_t>()) {
            json[name] = variant.to_uint8();
        } else if (type == rttr::type::get<uint16_t>()) {
            json[name] = variant.to_uint16();
        } else if (type == rttr::type::get<uint32_t>()) {
            json[name] = variant.to_uint32();
        } else if (type == rttr::type::get<uint64_t>()) {
            json[name] = variant.to_uint64();
        } else if (type == rttr::type::get<float>()) {
            json[name] = variant.to_float();
        } else if (type == rttr::type::get<double>()) {
            json[name] = variant.to_double();
        }
        return json;
    }

    if (type.is_enumeration()) {
        bool success = false;
        const auto string = variant.to_string(&success);

        if (success) {
            json[name] = string;
            return json;
        }

        const auto value = variant.to_int32(&success);
        if (success) {
            json[name] = value;
            return json;
        }
    }

    json[name] = variant.to_string();
    return json;
}
} // namespace Serialization
} // namespace RDE
