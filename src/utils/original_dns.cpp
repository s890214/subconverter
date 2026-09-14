#include "utils/original_dns.h"

std::string pickOriginalDnsYaml(const std::vector<std::string> &dns_blocks)
{
    for (const std::string &block : dns_blocks)
    {
        if (!block.empty())
            return block;
    }
    return "";
}
