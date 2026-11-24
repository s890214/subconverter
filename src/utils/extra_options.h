#ifndef EXTRA_OPTIONS_H_INCLUDED
#define EXTRA_OPTIONS_H_INCLUDED

#include <map>
#include <string>

#include <yaml-cpp/yaml.h>

/// Copy only scalar keys from a Clash proxy mapping. Nested maps/lists
/// (ws-opts, reality-opts, alpn, ...) must not be stored as strings.
void collectScalarExtraOptions(const YAML::Node &node,
                               std::map<std::string, std::string> &out);

/// Drop mapping keys whose value is YAML null (e.g. `up: null`, `down: ~`
/// found in airport hysteria2 entries). Null values are not valid proxy
/// settings and break Clash parsing when written back verbatim.
void removeNullFields(YAML::Node &node);

#endif // EXTRA_OPTIONS_H_INCLUDED
