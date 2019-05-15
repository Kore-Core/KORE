// Copyright (c) 2011-2014 The Bitcoin Core developers
// Copyright (c) 2016 The KORE developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "checkpoints.h"
#include "init.h"
#include "tests_util.h"
#include "txdb.h"
#include "util.h"
#include "utiltime.h"

#include <boost/test/unit_test.hpp>

static const string strSecret("5HxWvvfubhXpYYpS3tJkw6fq9jE9j18THftkZjHHfmFiWtmAbrj");

//#define RUN_FORK_TESTS

// struct RestartDataBaseTest
// {

//     RestartDataBaseTest()
//     {
//         cout << "Finalizing Current Database !!!" << endl;
//         FinalizeDBTest(false);
//     }
//     ~RestartDataBaseTest() 
//     { 
//         cout << "Starting a New Database !!!" << endl;
//         InitializeDBTest();
//     }    
// };


//BOOST_FIXTURE_TEST_SUITE(fork_kore, RestartDataBaseTest)
BOOST_AUTO_TEST_SUITE(fork_kore)

#ifdef RUN_FORK_TESTS

BOOST_AUTO_TEST_CASE(minimum_fork)
{
    cout << "**  Starting fork_kore/minimum_fork **" << endl;
    if (fDebug) {
        LogPrintf("************************************** \n");
        LogPrintf("**  Starting fork_kore/minimum_fork ** \n");
        LogPrintf("************************************** \n");
    }
    Checkpoints::fEnabled = false;
    int64_t oldTargetTimespan = Params().GetTargetTimespan();
    int64_t oldTargetSpacing = Params().GetTargetSpacing();
    int oldHeightToFork = Params().HeightToFork();
    int oldCoinBaseMaturity = Params().GetCoinMaturity();
    int oldStakeMinAge = Params().GetStakeMinAge();
    int oldModifier = Params().GetModifierInterval();
    // confirmations    : 3
    // remember that the miminum spacing is 10 !!!
    // spacing          : [confirmations-1, max(confirmations-1, value)]
    // modifierInterval : [spacing, spacing)]
    // pow blocks       : [confirmations + 1, max(confirmations+1, value)], this way we will have 2 modifiers
    int minConfirmations = 3;
    ModifiableParams()->setHeightToFork(9);
    ModifiableParams()->setTargetSpacing(minConfirmations - 1);
    ModifiableParams()->setStakeModifierInterval(minConfirmations - 1);
    ModifiableParams()->setCoinMaturity(minConfirmations); 
    ModifiableParams()->setStakeMinAge(0);
    ModifiableParams()->setTargetTimespan(1);
    ModifiableParams()->setEnableBigRewards(true);
    SetMockTime(0);

    ScanForWalletTransactions(pwalletMain);
    CScript scriptPubKey = GenerateSamePubKeyScript4Wallet(strSecret, pwalletMain);

    // generate 4 pow blocks
    GeneratePOWLegacyBlocks(1, minConfirmations + 2, pwalletMain, scriptPubKey);
    // generate 4 pos blocks
    GeneratePOSLegacyBlocks(minConfirmations + 2, 9, pwalletMain, scriptPubKey);

    GenerateBlocks(9, 100, pwalletMain, scriptPubKey, true);

    // Leaving old values
    Checkpoints::fEnabled = true;
    ModifiableParams()->setHeightToFork(oldHeightToFork);
    ModifiableParams()->setEnableBigRewards(false);
    ModifiableParams()->setCoinMaturity(oldCoinBaseMaturity);
    ModifiableParams()->setStakeMinAge(oldStakeMinAge);
    ModifiableParams()->setStakeModifierInterval(oldModifier);
    ModifiableParams()->setTargetTimespan(oldTargetTimespan);
    ModifiableParams()->setTargetSpacing(oldTargetSpacing);
}

BOOST_AUTO_TEST_CASE(minimum_fork2)
{
    cout << "**  Starting fork_kore/minimum_fork2 **" << endl;
}

#endif

BOOST_AUTO_TEST_SUITE_END()
