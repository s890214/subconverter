#include "utils/ruleset_format.h"

#include <cctype>
#include <string>

namespace
{
std::string urlPathForExtension(const std::string &url)
{
    std::string path = url;
    auto cut = path.find_first_of("?#");
    if (cut != std::string::npos)
        path.resize(cut);
    for (char &c : path)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return path;
}
} // namespace

std::string inferRulesetFormatFromUrl(const std::string &url)
{
    const std::string path = urlPathForExtension(url);
    static const std::string mrs = ".mrs";
    if (path.size() >= mrs.size() && path.compare(path.size() - mrs.size(), mrs.size(), mrs) == 0)
        return "mrs";
    return "";
}

std::string resolveRulesetFormat(int rule_type, const std::string &url,
                                 const std::string &explicit_format)
{
    if (!explicit_format.empty())
        return explicit_format;
    // ruleset_type: 2 = clash-domain, 3 = clash-ipcidr
    if (rule_type != 2 && rule_type != 3)
        return "";
    return inferRulesetFormatFromUrl(url);
}
