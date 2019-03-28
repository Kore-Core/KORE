#include "chainparams.h"
#include "checkpoints.h"
#include "init.h"
#include "tests_util.h"
#include "util.h"
#include "utiltime.h"


#include <boost/test/unit_test.hpp>

static const string strSecret("5HxWvvfubhXpYYpS3tJkw6fq9jE9j18THftkZjHHfmFiWtmAbrj");

/* This testcase will take long time to run */
BOOST_AUTO_TEST_SUITE(fork_testnet)

// #define RUN_FORK_TESTS

#ifdef RUN_FORK_TESTS

BOOST_AUTO_TEST_CASE(testnet_parameters)
{
    SetMockTime(GetTime());
    if (fDebug) {
        LogPrintf("*********************************************** \n");
        LogPrintf("**  Starting fork_testnet/testnet_parameters ** \n");
        LogPrintf("*********************************************** \n");
    }

    Checkpoints::fEnabled = false;
    int64_t oldTargetTimespan = Params().GetTargetTimespan();
    int64_t oldTargetSpacing = Params().GetTargetSpacing();
    int oldHeightToFork = Params().HeightToFork();
    int oldStakeMinConfirmations = Params().GetStakeMinConfirmations();
    int oldCoinBaseMaturity = Params().GetCoinbaseMaturity();
    int oldStakeMinAge = Params().GetStakeMinAge();
    int oldModifier = Params().GetModifierInterval();
    // confirmations    : 3
    // remember that the miminum spacing is 10 !!!
    // spacing          : [confirmations-1, max(confirmations-1, value)]
    // modifierInterval : [spacing, spacing)]
    // pow blocks       : [confirmations + 1, max(confirmations+1, value)], this way we will have 2 modifiers
    int minConfirmations = 25;
    int nStakeMinConfirmations       = minConfirmations;        
    int nCoinbaseMaturity                    = minConfirmations;
    int nTargetTimespan              = 1 * 60; // KORE: 1 minute
    int nTargetSpacingForStake          = 60;
    int nTargetSpacing               = nTargetSpacingForStake;
    int nModifierInterval            = nTargetSpacingForStake; // Modifier interval: time to elapse before new modifier is computed
    int nStakeMinAge                 = 30 * 60; // It will stake after 30 minutes

    ModifiableParams()->setHeightToFork(minConfirmations + 2);
    ModifiableParams()->setStakeMinConfirmations(nStakeMinConfirmations);
    ModifiableParams()->setCoinbaseMaturity(nCoinbaseMaturity);
    ModifiableParams()->setTargetSpacing(nTargetSpacing);
    ModifiableParams()->setStakeModifierInterval(nModifierInterval);
    ModifiableParams()->setStakeMinAge(nStakeMinAge);
    ModifiableParams()->setTargetTimespan(nTargetTimespan);
    ModifiableParams()->setEnableBigRewards(true);
    //SetMockTime(0);    

    ScanForWalletTransactions(pwalletMain);
    CScript scriptPubKey = GenerateSamePubKeyScript4Wallet(strSecret, pwalletMain);

    // recreate old pow blocks
    CreateOldBlocksFromBlockInfo(1, minConfirmations + 2, blockinfo[0], pwalletMain, scriptPubKey, false);

    // lets generate new pow blocks
    int pow = minConfirmations + 2 + Params().GetStakeMinConfirmations() + 10;
    ModifiableParams()->setLastPowBlock(pow);
    GenerateBlocks(minConfirmations + 2, pow, pwalletMain, scriptPubKey, false);

    // lets generate enought block and have 10 POS blocks confirmed
    GenerateBlocks(pow, pow + Params().GetStakeMinConfirmations() + 10, pwalletMain, scriptPubKey, true);

    // Leaving old values
    Checkpoints::fEnabled = true;
    ModifiableParams()->setHeightToFork(oldHeightToFork);
    ModifiableParams()->setEnableBigRewards(false);
    ModifiableParams()->setCoinbaseMaturity(oldCoinBaseMaturity);
    ModifiableParams()->setStakeMinAge(oldStakeMinAge);
    ModifiableParams()->setStakeModifierInterval(oldModifier);
    ModifiableParams()->setStakeMinConfirmations(oldStakeMinConfirmations);
    ModifiableParams()->setTargetTimespan(oldTargetTimespan);
    ModifiableParams()->setTargetSpacing(oldTargetSpacing);
}

#endif

BOOST_AUTO_TEST_SUITE_END()