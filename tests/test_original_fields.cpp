#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include <yaml-cpp/yaml.h>

#include "utils/extra_options.h"
#include "utils/original_dns.h"

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

int main()
{
    const std::string airport_dns =
        "enable: true\nnameserver-policy:\n  +.v51124-1.qpon: tcp://example.fyi:8080\n";

    expect_eq("empty", pickOriginalDnsYaml({}), "");
    expect_eq("skip-leading-empty", pickOriginalDnsYaml({"", "", airport_dns}), airport_dns);
    expect_eq("first-non-empty-wins", pickOriginalDnsYaml({airport_dns, "other: 1\n"}), airport_dns);
    expect_eq("insert-then-airport", pickOriginalDnsYaml({"", airport_dns}), airport_dns);

    YAML::Node proxy = YAML::Load(
        "name: 香港W01\n"
        "server: hk01.entry.v51124-1.qpon\n"
        "port: 443\n"
        "type: ss\n"
        "udp: true\n"
        "alpn:\n  - h2\n  - http/1.1\n"
        "ws-opts:\n  path: /\n  headers:\n    Host: example.com\n");
    std::map<std::string, std::string> extra;
    collectScalarExtraOptions(proxy, extra);
    expect_eq("scalar-name", extra["name"], "香港W01");
    expect_eq("scalar-server", extra["server"], "hk01.entry.v51124-1.qpon");
    expect_eq("scalar-port", extra["port"], "443");
    expect_eq("scalar-udp", extra["udp"], "true");
    expect_true("no-ws-opts", extra.find("ws-opts") == extra.end());
    expect_true("no-alpn", extra.find("alpn") == extra.end());

    YAML::Node hy2 = YAML::Load(
        "name: hy2-null\n"
        "type: hysteria2\n"
        "server: 1.2.3.4\n"
        "port: 443\n"
        "up: null\n"
        "down: ~\n"
        "sni: example.com\n"
        "udp: true\n");
    removeNullFields(hy2);
    expect_true("null-up-removed", !hy2["up"].IsDefined());
    expect_true("null-down-removed", !hy2["down"].IsDefined());
    expect_eq("keep-sni", hy2["sni"].as<std::string>(), "example.com");
    expect_eq("keep-udp", hy2["udp"].as<std::string>(), "true");
    expect_eq("keep-name-hy2", hy2["name"].as<std::string>(), "hy2-null");

    YAML::Node list = YAML::Load("- a\n- b\n");
    removeNullFields(list);
    expect_true("non-map-untouched", list.size() == 2);

    if (failures)
    {
        std::printf("%d failed\n", failures);
        return 1;
    }
    std::printf("all passed\n");
    return 0;
}
