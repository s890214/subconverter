#ifndef RULESET_FORMAT_H_INCLUDED
#define RULESET_FORMAT_H_INCLUDED

#include <string>

/// Infer Clash/Mihomo rule-provider format from a ruleset URL.
/// Returns "mrs" when the URL path ends with .mrs (case-insensitive,
/// query/fragment ignored). Otherwise empty — caller keeps the YAML default.
std::string inferRulesetFormatFromUrl(const std::string &url);

/// Resolve the format field written into a Clash rule-provider.
/// explicit_format wins when non-empty. MRS is only valid for clash-domain
/// and clash-ipcidr (rule_type 2 and 3, matching ruleset_type).
std::string resolveRulesetFormat(int rule_type, const std::string &url,
                                 const std::string &explicit_format);

#endif // RULESET_FORMAT_H_INCLUDED
