
#include "chainparams.h"
#include "checkpoints.h"
#include "init.h"
#include "tests_util.h"
#include "utiltime.h"

//#define GENERATE_BLOCK_INFO

#include <boost/test/unit_test.hpp>

static const string strSecret("5HxWvvfubhXpYYpS3tJkw6fq9jE9j18THftkZjHHfmFiWtmAbrj");

BOOST_AUTO_TEST_SUITE(generate_blockinfo)

#ifdef GENERATE_BLOCK_INFO
/*
  This TEST CASE SHOULD BE USED ONLY WHEN YOU WANT TO CREATE DATA TO BLOCK INFO
  */
  
BOOST_AUTO_TEST_CASE(generate_old_pow)
{
    SetMockTime(GetTime());
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
    int nMaturity                    = minConfirmations;
    int nTargetTimespan              = 1 * 60; // KORE: 1 minute
    int nStakeTargetSpacing          = 10;
    int nTargetSpacing               = nStakeTargetSpacing;
    int nModifierInterval            = nStakeTargetSpacing; // Modifier interval: time to elapse before new modifier is computed
    int nStakeMinAge                 = 30 * 60; // It will stake after 30 minutes

    ModifiableParams()->setHeightToFork(999);
    ModifiableParams()->setCoinMaturity(nMaturity);
    ModifiableParams()->setTargetSpacing(nTargetSpacing);
    ModifiableParams()->setStakeModifierInterval(nModifierInterval);
    ModifiableParams()->setStakeMinAge(nStakeMinAge);
    ModifiableParams()->setTargetTimespan(nTargetTimespan);
    ModifiableParams()->setEnableBigRewards(true); 
    
    ScanForWalletTransactions(pwalletMain);
    CScript scriptPubKey = GenerateSamePubKeyScript4Wallet(strSecret, pwalletMain);

    int totalOldPow = 30;
    // generate old pow blocks
    GeneratePOWLegacyBlocks(1,totalOldPow+1, pwalletMain, scriptPubKey, true);
 
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