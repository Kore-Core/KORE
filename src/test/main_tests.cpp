// Copyright (c) 2014 The Bitcoin Core developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The KORE developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "main.h"
#include "miner.h"
#include "primitives/transaction.h"

#include "utilmoneystr.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(main_tests)

// We are unable to test pre-fork supply beacause it doesn't use a fixed amount.
// For test purpose we consider the fork to happen on block 429033.
// The money supply on block 429032 is 2,141,450.338583.
BOOST_AUTO_TEST_CASE(subsidy_limit_test_post_fork)
{
    SelectParams(CBaseChainParams::MAIN);

    int nHeight = 429033;
    CAmount nMoneySupply = 214145033858300;
    CAmount nlastSubsidy = 103008606;
    CAmount nSubsidy = 0;

    // Represents block 429032
    CBlockIndex pindexPrev;
    pindexPrev.nHeight = 429032;
    pindexPrev.nMoneySupply = nMoneySupply;

    for (nHeight; nHeight <= 12435722; nHeight++) {
        /* PoS */
        nSubsidy = GetBlockReward(&pindexPrev);
        BOOST_CHECK(nSubsidy <= nlastSubsidy);
        nlastSubsidy = nSubsidy;
        if(!MoneyRange(nSubsidy))
            printf("%d, %s", nHeight, FormatMoney(nSubsidy).c_str());
        BOOST_CHECK(MoneyRange(nSubsidy));
        nMoneySupply += nSubsidy;
        BOOST_CHECK(nMoneySupply <= MAX_MONEY);
        pindexPrev.nMoneySupply = nMoneySupply;
        pindexPrev.nHeight++;
    }
    BOOST_ASSERT(pindexPrev.nMoneySupply == MAX_MONEY);

    // Try to call it again after the limit was reached
    nSubsidy = GetBlockReward(&pindexPrev);
    BOOST_ASSERT(nSubsidy == 0);
}

BOOST_AUTO_TEST_SUITE_END()
