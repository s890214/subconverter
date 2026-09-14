#ifndef EXTRA_OPTIONS_H_INCLUDED
#define EXTRA_OPTIONS_H_INCLUDED

#include <map>
#include <string>

#include <yaml-cpp/yaml.h>

/// Copy only scalar keys from a Clash proxy mapping. Nested maps/lists
/// (ws-opts, reality-opts, alpn, ...) must not be stored as strings.
void collectScalarExtraOptions(const YAML::Node &node,
                               std::map<std::string, std::string> &out);

#endif // EXTRA_OPTIONS_H_INCLUDED
