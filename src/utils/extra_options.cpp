#include "utils/extra_options.h"

#include <vector>

void collectScalarExtraOptions(const YAML::Node &node,
                               std::map<std::string, std::string> &out)
{
    if (!node || !node.IsMap())
        return;
    for (auto it = node.begin(); it != node.end(); ++it)
    {
        if (!it->second.IsScalar())
            continue;
        out[it->first.as<std::string>()] = it->second.as<std::string>();
    }
}

void removeNullFields(YAML::Node &node)
{
    if (!node || !node.IsMap())
        return;

    std::vector<std::string> null_keys;
    for (auto it = node.begin(); it != node.end(); ++it)
    {
        if (it->second.IsNull())
            null_keys.emplace_back(it->first.as<std::string>());
    }
    for (const auto &key : null_keys)
        node.remove(key);
}
