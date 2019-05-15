#include "chainparams.h"
#include "checkpoints.h"
#include "init.h"
#include "tests_util.h"
#include "util.h"
#include "utiltime.h"


#include <boost/test/unit_test.hpp>

static const string strSecret("5HxWvvfubhXpYYpS3tJkw6fq9jE9j18THftkZjHHfmFiWtmAbrj");

//#define RUN_FORK_TESTS

BOOST_AUTO_TEST_SUITE(fork_afterwords)

#ifdef RUN_FORK_TESTS

BOOST_AUTO_TEST_CASE(after_fork)
{
    // todo how to get this parameter from argument list ??
    //bool logToStdout = GetBoolArg("-logtostdout", false);
    bool logToStdout = true;

    if (fDebug) {
        LogPrintf("****************************************** \n");
        LogPrintf("**  Starting fork_afterwords/after_fork ** \n");
        LogPrintf("****************************************** \n");
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
    ModifiableParams()->setHeightToFork(0);
    ModifiableParams()->setCoinMaturity(minConfirmations - 1);
    ModifiableParams()->setTargetSpacing(minConfirmations - 1);
    ModifiableParams()->setStakeModifierInterval(minConfirmations - 1);
    //ModifiableParams()->setTargetSpacing(10);
    //ModifiableParams()->setStakeModifierInterval(10);
    ModifiableParams()->setStakeMinAge(0);
    ModifiableParams()->setTargetTimespan(1);
    ModifiableParams()->setEnableBigRewards(true);
    //ModifiableParams()->setMineBlocksOnDemand(false);
    SetMockTime(0);

    ScanForWalletTransactions(pwalletMain);
    CScript scriptPubKey = GenerateSamePubKeyScript4Wallet(strSecret, pwalletMain);

    // generate pow blocks, so we can stake
    GenerateBlocks(1, minConfirmations + 2, pwalletMain, scriptPubKey, false, logToStdout);

    // we are just checking if we are able to generate PoS blocks after fork
    // lets exercise more than 64 blocks, this way we will see if the max
    // modifierinterval is working, it gets max(64 blocks)
    GenerateBlocks(minConfirmations + 2, minConfirmations + 2 + 100, pwalletMain, scriptPubKey, true, logToStdout);

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