#include "utils/extra_options.h"

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
