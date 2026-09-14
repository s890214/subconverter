#ifndef ORIGINAL_DNS_H_INCLUDED
#define ORIGINAL_DNS_H_INCLUDED

#include <string>
#include <vector>

/// First non-empty original dns: block. Inserted nodes without DNS are skipped
/// so the airport subscription's top-level dns is not lost.
std::string pickOriginalDnsYaml(const std::vector<std::string> &dns_blocks);

#endif // ORIGINAL_DNS_H_INCLUDED
