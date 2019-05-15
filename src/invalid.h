// Copyright (c) 2018 The KORE developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef KORE_INVALID_H
#define KORE_INVALID_H

#endif //KORE_INVALID_H

#include <primitives/transaction.h>
#include <univalue/include/univalue.h>

namespace invalid_out
{
extern std::set<COutPoint> setInvalidOutPoints;

UniValue read_json(const std::string& jsondata);

bool ContainsOutPoint(const COutPoint& out);
bool LoadOutpoints();

} // namespace invalid_out