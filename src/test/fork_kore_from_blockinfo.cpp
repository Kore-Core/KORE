// Copyright (c) 2011-2014 The Bitcoin Core developers
// Copyright (c) 2016 The KORE developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "checkpoints.h"
#include "init.h"
#include "main.h" // pcoinsTip;
#include "tests_util.h"
#include "util.h"
#include "utiltime.h"

#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_SUITE(fork_kore_from_blockinfo)

static const string strSecret("5HxWvvfubhXpYYpS3tJkw6fq9jE9j18THftkZjHHfmFiWtmAbrj");

//#define RUN_FORK_TESTS

#ifdef RUN_FORK_TESTS


BOOST_AUTO_TEST_CASE(quick_fork)
{
    
    // todo how to get this parameter from argument list ??
    //bool logToStdout = GetBoolArg("-logtostdout", false);
    bool logToStdout = true;
    SetMockTime(GetTime());
    
    if (fDebug) {
        LogPrintf("*************************************************** \n");
        LogPrintf("**  Starting fork_kore_from_blockinfo/quick_fork ** \n");
        LogPrintf("*************************************************** \n");
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
    ModifiableParams()->setLastPowBlock(minConfirmations + 1);
    
    ScanForWalletTransactions(pwalletMain);
    CScript scriptPubKey = GenerateSamePubKeyScript4Wallet(strSecret, pwalletMain);

    // generate 4 pow blocks
    CreateOldBlocksFromBlockInfo(1, minConfirmations + 2, blockinfo[0], pwalletMain, scriptPubKey, false, logToStdout);

    // generate 4 pos blocks
    GeneratePOSLegacyBlocks(minConfirmations + 2, 9, pwalletMain, scriptPubKey, logToStdout);

    GenerateBlocks(9, 100, pwalletMain, scriptPubKey, true, logToStdout);

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

BOOST_AUTO_TEST_CASE(check_database_pow)
{
    
    // todo how to get this parameter from argument list ??
    //bool logToStdout = GetBoolArg("-logtostdout", false);
    bool logToStdout = true;
    SetMockTime(GetTime());
    
    if (fDebug) {
        LogPrintf("*************************************************** \n");
        LogPrintf("**  Starting fork_kore_from_blockinfo/quick_fork ** \n");
        LogPrintf("*************************************************** \n");
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
    ModifiableParams()->setLastPowBlock(minConfirmations + 1);
    
    ScanForWalletTransactions(pwalletMain);
    CScript scriptPubKey = GenerateSamePubKeyScript4Wallet(strSecret, pwalletMain);

    // generate 4 pow blocks
    CreateOldBlocksFromBlockInfo(1, minConfirmations + 2, blockinfo[0], pwalletMain, scriptPubKey, false, logToStdout);

    // generate 4 pos blocks
    //GeneratePOSLegacyBlocks(minConfirmations + 2, 6, pwalletMain, scriptPubKey, logToStdout);

    //GenerateBlocks(9, 100, pwalletMain, scriptPubKey, true, logToStdout);

    CheckDatabaseState(pwalletMain);

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

BOOST_AUTO_TEST_CASE(check_database_pow_pow)
{
    
    // todo how to get this parameter from argument list ??
    //bool logToStdout = GetBoolArg("-logtostdout", false);
    bool logToStdout = true;
    SetMockTime(GetTime());
    
    if (fDebug) {
        LogPrintf("*************************************************** \n");
        LogPrintf("**  Starting fork_kore_from_blockinfo/quick_fork ** \n");
        LogPrintf("*************************************************** \n");
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
    ModifiableParams()->setLastPowBlock(20);
    
    ScanForWalletTransactions(pwalletMain);
    CScript scriptPubKey = GenerateSamePubKeyScript4Wallet(strSecret, pwalletMain);

    // generate 8 pow blocks
    CreateOldBlocksFromBlockInfo(1, 9, blockinfo[0], pwalletMain, scriptPubKey, false, logToStdout);

    // generate new pow
    GenerateBlocks(9, 20, pwalletMain, scriptPubKey, false, logToStdout);

    CheckDatabaseState(pwalletMain);

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

BOOST_AUTO_TEST_CASE(check_database_pow_pos)
{
    
    // todo how to get this parameter from argument list ??
    //bool logToStdout = GetBoolArg("-logtostdout", false);
    bool logToStdout = true;
    SetMockTime(GetTime());
    
    if (fDebug) {
        LogPrintf("*************************************************** \n");
        LogPrintf("**  Starting fork_kore_from_blockinfo/quick_fork ** \n");
        LogPrintf("*************************************************** \n");
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
    ModifiableParams()->setHeightToFork(50);
    ModifiableParams()->setTargetSpacing(minConfirmations - 1);
    ModifiableParams()->setStakeModifierInterval(minConfirmations - 1);
    ModifiableParams()->setCoinMaturity(minConfirmations); 
    ModifiableParams()->setStakeMinAge(0);
    ModifiableParams()->setTargetTimespan(1);
    ModifiableParams()->setEnableBigRewards(true);
    ModifiableParams()->setLastPowBlock(minConfirmations + 1);
    
    ScanForWalletTransactions(pwalletMain);
    CScript scriptPubKey = GenerateSamePubKeyScript4Wallet(strSecret, pwalletMain);

    // generate 4 pow blocks
    CreateOldBlocksFromBlockInfo(1, minConfirmations + 2, blockinfo[0], pwalletMain, scriptPubKey, false, logToStdout);

    // Lets check how it the cachedCoins
    // generate 1 pos blocks
    GeneratePOSLegacyBlocks(minConfirmations + 2, 6, pwalletMain, scriptPubKey, logToStdout);

    CheckDatabaseState(pwalletMain);

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

BOOST_AUTO_TEST_CASE(check_database_pow_pos_newpos)
{
    
    // todo how to get this parameter from argument list ??
    //bool logToStdout = GetBoolArg("-logtostdout", false);
    bool logToStdout = true;
    SetMockTime(GetTime());
    
    if (fDebug) {
        LogPrintf("*************************************************** \n");
        LogPrintf("**  Starting fork_kore_from_blockinfo/quick_fork ** \n");
        LogPrintf("*************************************************** \n");
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
    ModifiableParams()->setHeightToFork(6);
    ModifiableParams()->setTargetSpacing(minConfirmations - 1);
    ModifiableParams()->setStakeModifierInterval(minConfirmations - 1);
    ModifiableParams()->setCoinMaturity(minConfirmations); 
    ModifiableParams()->setStakeMinAge(0);
    ModifiableParams()->setTargetTimespan(1);
    ModifiableParams()->setEnableBigRewards(true);
    ModifiableParams()->setLastPowBlock(minConfirmations + 1);
    
    ScanForWalletTransactions(pwalletMain);
    CScript scriptPubKey = GenerateSamePubKeyScript4Wallet(strSecret, pwalletMain);

    // generate 4 pow blocks
    CreateOldBlocksFromBlockInfo(1, minConfirmations + 2, blockinfo[0], pwalletMain, scriptPubKey, false, logToStdout);

    // generate 1 pos blocks
    GeneratePOSLegacyBlocks(minConfirmations + 2, 6, pwalletMain, scriptPubKey, logToStdout);

    // generate 1 new pos blocks
    GenerateBlocks(6, 7, pwalletMain, scriptPubKey, true, logToStdout);

    uint256 oldTipHash = chainActive.Tip()->GetBlockHash();

    CheckDatabaseState(pwalletMain);

    uint256 newTipHash = chainActive.Tip()->GetBlockHash();
    BOOST_ASSERT(oldTipHash == newTipHash);

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

#endif

BOOST_AUTO_TEST_SUITE_END()
