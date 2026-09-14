#include <cstdio>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

#include <yaml-cpp/yaml.h>

#include "utils/ruleset_format.h"

static int failures = 0;

static void expect_eq(const char *name, const std::string &got, const std::string &want)
{
    if (got == want)
    {
        std::printf("PASS %s\n", name);
        return;
    }
    std::printf("FAIL %s: got '%s' want '%s'\n", name, got.c_str(), want.c_str());
    failures++;
}

static void expect_true(const char *name, bool ok)
{
    if (ok)
    {
        std::printf("PASS %s\n", name);
        return;
    }
    std::printf("FAIL %s\n", name);
    failures++;
}

// Same split as INIBinding::from<RulesetConfig>::from_ini
struct IniRuleset
{
    std::string group;
    std::string url;
    int interval = 86400;
};

static IniRuleset parseIniRuleset(const std::string &x)
{
    IniRuleset conf;
    auto pos = x.find(',');
    if (pos == std::string::npos)
        return conf;
    conf.group = x.substr(0, pos);
    if (x.substr(pos + 1, 2) == "[]")
    {
        conf.url = x.substr(pos + 1);
        return conf;
    }
    auto epos = x.rfind(',');
    if (pos != epos)
    {
        conf.interval = std::stoi(x.substr(epos + 1));
        conf.url = x.substr(pos + 1, epos - pos - 1);
    }
    else
        conf.url = x.substr(pos + 1);
    return conf;
}

// Same prefix table as RulesetTypes in settings.cpp
static int stripRulesetType(std::string &url)
{
    static const std::pair<const char *, int> types[] = {
        {"clash-domain:", 2},
        {"clash-ipcidr:", 3},
        {"clash-classic:", 4},
        {"quanx:", 1},
        {"surge:", 0},
    };
    for (const auto &t : types)
    {
        const std::string prefix = t.first;
        if (url.compare(0, prefix.size(), prefix) == 0)
        {
            url.erase(0, prefix.size());
            return t.second;
        }
    }
    return 0; // RULESET_SURGE
}

// Same fields templates.cpp writes for a clash-domain provider
static std::string emitDomainProvider(const std::string &name, const std::string &url,
                                      const std::string &format, int interval)
{
    YAML::Node base_rule;
    auto node = base_rule["rule-providers"][name];
    node["type"] = "http";
    node["behavior"] = "domain";
    node["url"] = url;
    node["path"] = "./providers/hash_domain.yaml";
    if (!format.empty())
        node["format"] = format;
    if (interval)
        node["interval"] = interval;
    return YAML::Dump(base_rule);
}

int main()
{
    const std::string aether_direct =
        "https://cdn.jsdelivr.net/gh/Aethersailor/Custom_OpenClash_Rules@main/rule/Custom_Direct_Domain.mrs";
    const std::string aether_proxy =
        "https://cdn.jsdelivr.net/gh/Aethersailor/Custom_OpenClash_Rules@main/rule/Custom_Proxy_Domain.mrs";
    const std::string aether_steam =
        "https://cdn.jsdelivr.net/gh/Aethersailor/Custom_OpenClash_Rules@main/rule/Steam_CDN_Domain.mrs";
    const std::string aether_classic =
        "https://cdn.jsdelivr.net/gh/Aethersailor/Custom_OpenClash_Rules@main/rule/Custom_Direct_Classical_IP.yaml";

    expect_eq("direct.mrs", inferRulesetFormatFromUrl(aether_direct), "mrs");
    expect_eq("proxy.mrs", inferRulesetFormatFromUrl(aether_proxy), "mrs");
    expect_eq("steam.mrs", inferRulesetFormatFromUrl(aether_steam), "mrs");
    expect_eq("classic.yaml", inferRulesetFormatFromUrl(aether_classic), "");
    expect_eq("uppercase.MRS", inferRulesetFormatFromUrl("https://example.com/x.MRS"), "mrs");
    expect_eq("query.mrs", inferRulesetFormatFromUrl("https://example.com/x.mrs?token=1"), "mrs");
    expect_eq("fragment.mrs", inferRulesetFormatFromUrl("https://example.com/x.mrs#frag"), "mrs");
    expect_eq("not-mrs-substring", inferRulesetFormatFromUrl("https://example.com/mrs.yaml"), "");

    expect_eq("domain+mrs", resolveRulesetFormat(2, aether_direct, ""), "mrs");
    expect_eq("ipcidr+mrs", resolveRulesetFormat(3, "https://example.com/cn.mrs", ""), "mrs");
    expect_eq("classical+mrs-rejected", resolveRulesetFormat(4, aether_direct, ""), "");
    expect_eq("surge+mrs-ignored", resolveRulesetFormat(0, aether_direct, ""), "");
    expect_eq("explicit-wins", resolveRulesetFormat(2, aether_direct, "text"), "text");
    expect_eq("domain+yaml-empty", resolveRulesetFormat(2, aether_classic, ""), "");

    // Full INI → type-strip → resolve → YAML emit, using the live Aethersailor line
    const std::string ini_direct =
        "🎯 全球直连,clash-domain:https://cdn.jsdelivr.net/gh/Aethersailor/Custom_OpenClash_Rules@main/rule/Custom_Direct_Domain.mrs,1800";
    const std::string ini_classic =
        "🎯 全球直连,clash-classic:https://cdn.jsdelivr.net/gh/Aethersailor/Custom_OpenClash_Rules@main/rule/Custom_Direct_Classical_IP.yaml,28800";
    const std::string ini_proxy =
        "🚀 手动选择,clash-domain:https://cdn.jsdelivr.net/gh/Aethersailor/Custom_OpenClash_Rules@main/rule/Custom_Proxy_Domain.mrs,28800";
    const std::string ini_geo = "🎯 全球直连,[]GEOSITE,private";

    {
        auto conf = parseIniRuleset(ini_direct);
        expect_eq("ini.direct.url-has-prefix", conf.url.substr(0, 13), "clash-domain:");
        expect_eq("ini.direct.interval", std::to_string(conf.interval), "1800");
        std::string url = conf.url;
        int type = stripRulesetType(url);
        expect_eq("ini.direct.type", std::to_string(type), "2");
        expect_eq("ini.direct.stripped", url, aether_direct);
        std::string format = resolveRulesetFormat(type, url, "");
        expect_eq("ini.direct.format", format, "mrs");
        std::string yaml = emitDomainProvider("Custom_Direct_Domain", url, format, conf.interval);
        expect_true("ini.direct.yaml-has-format-mrs", yaml.find("format: mrs") != std::string::npos);
        expect_true("ini.direct.yaml-url-still-mrs", yaml.find(aether_direct) != std::string::npos);
        expect_true("ini.direct.yaml-path-still-yaml", yaml.find("_domain.yaml") != std::string::npos);
        std::printf("--- emitted ---\n%s---\n", yaml.c_str());
    }

    {
        auto conf = parseIniRuleset(ini_proxy);
        std::string url = conf.url;
        int type = stripRulesetType(url);
        std::string format = resolveRulesetFormat(type, url, "");
        expect_eq("ini.proxy.format", format, "mrs");
        std::string yaml = emitDomainProvider("Custom_Proxy_Domain", url, format, conf.interval);
        expect_true("ini.proxy.yaml-has-format-mrs", yaml.find("format: mrs") != std::string::npos);
    }

    {
        auto conf = parseIniRuleset(ini_classic);
        std::string url = conf.url;
        int type = stripRulesetType(url);
        expect_eq("ini.classic.type", std::to_string(type), "4");
        std::string format = resolveRulesetFormat(type, url, "");
        expect_eq("ini.classic.format", format, "");
        YAML::Node node;
        node["rule-providers"]["Custom_Direct_Classical_IP"]["type"] = "http";
        node["rule-providers"]["Custom_Direct_Classical_IP"]["behavior"] = "classical";
        node["rule-providers"]["Custom_Direct_Classical_IP"]["url"] = url;
        node["rule-providers"]["Custom_Direct_Classical_IP"]["path"] = "./providers/hash.yaml";
        if (!format.empty())
            node["rule-providers"]["Custom_Direct_Classical_IP"]["format"] = format;
        std::string yaml = YAML::Dump(node);
        expect_true("ini.classic.yaml-no-format", yaml.find("format:") == std::string::npos);
    }

    {
        auto conf = parseIniRuleset(ini_geo);
        expect_eq("ini.geo.url", conf.url, "[]GEOSITE,private");
        expect_true("ini.geo.inline-skip", conf.url.compare(0, 2, "[]") == 0);
    }

    if (failures)
    {
        std::printf("%d failed\n", failures);
        return 1;
    }
    std::printf("all passed\n");
    return 0;
}
