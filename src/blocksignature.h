// Copyright (c) 2017 The KORE developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef KORE_BLOCKSIGNATURE_H
#define KORE_BLOCKSIGNATURE_H

#include "key.h"
#include "primitives/block.h"
#include "keystore.h"

bool SignBlockWithKey(CBlock& block, const CKey& key);
bool SignBlock(CBlock& block, const CKeyStore& keystore);
bool CheckBlockSignature(const CBlock& block);

bool CheckBlockSignature_Legacy(const CBlock& block, const uint256& hash);

#endif //KORE_BLOCKSIGNATURE_H
