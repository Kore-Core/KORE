#include "kernelrecord.h"
#include "timedata.h"
#include "wallet/wallet.h"
#include "base58.h"

bool KernelRecord::showTransaction(const CWalletTx &wtx)
{
    if (wtx.IsCoinBase())
    {
        if (wtx.GetDepthInMainChain() < 2)
            return false;
    } else
        if (wtx.GetDepthInMainChain() == 0)
            return false;

    return true;
}

/*
 * Decompose CWallet transaction to model kernel records.
 */
std::vector<KernelRecord> KernelRecord::decomposeOutput(const CWallet *wallet, const CWalletTx &wtx)
{
    std::vector<KernelRecord> parts;
    int64_t nTime = wtx.GetTxTime();
    uint256 hash = wtx.GetHash();
    std::map<std::string, std::string> mapValue = wtx.mapValue;
    int nWeight = (std::min((GetAdjustedTime() - nTime), (int64_t)Params().GetConsensus().nStakeMinAge * 6) - Params().GetConsensus().nStakeMinAge) / 3600;

    if (showTransaction(wtx))
    {
        for (unsigned int nOut = 0; nOut < wtx.vout.size(); nOut++)
        {
            CTxOut txOut = wtx.vout[nOut];
            if( wallet->IsMine(txOut) ) {
                CTxDestination address;
                std::string addrStr;

                uint64_t coinAge = std::max(txOut.nValue * nWeight / COIN, (int64_t)0);

                if (ExtractDestination(txOut.scriptPubKey, address))
                {
                    // Sent to Address
                    addrStr = CKoreAddress(address).ToString();
                }
                else
                {
                    // Sent to IP, or other non-address transaction like OP_EVAL
                    addrStr = mapValue["to"];
                }

                parts.push_back(KernelRecord(hash, nTime, addrStr, txOut.nValue, nOut,  wallet->IsSpent(wtx.GetHash(), nOut), coinAge));
            }
        }
    }

    return parts;
}

std::string KernelRecord::getTxID()
{
    return hash.ToString() + strprintf("-%03d", idx);
}

int64_t KernelRecord::getAge() const
{
    return GetAdjustedTime() - nTime;
}

double KernelRecord::getProbToMintStake(double difficulty, int timeOffset) const
{
    double maxTarget = pow(static_cast<double>(2), 224);
    double target = maxTarget / difficulty;
    int dayWeight = (std::min((GetAdjustedTime() - nTime) + timeOffset, (int64_t)Params().GetConsensus().nStakeMinAge* 6) - Params().GetConsensus().nStakeMinAge);
    uint64_t coinAge = std::max(nValue * dayWeight / COIN, (int64_t)0);
    return target * coinAge / pow(static_cast<double>(2), 256);
}

double KernelRecord::getProbToMintWithinNMinutes(double difficulty, int minutes)
{
    if(difficulty != prevDifficulty || minutes != prevMinutes)
    {
        double prob = 1;
        double p;
        int d = minutes / 60 ; // Number of full hours
        int m = minutes % 60 ; // Number of minutes in the last hour
        int timeOffset;

        // Probabilities for the first d hours
        for(int i = 0; i < d; i++)
        {
            timeOffset = i;
            p = pow(1 - getProbToMintStake(difficulty, timeOffset), 60);
            prob *= p;
        }

        // Probability for the m minutes of the last day
        timeOffset = d;
        p = pow(1 - getProbToMintStake(difficulty, timeOffset), m);
        prob *= p;

        prob = 1 - prob;
        prevProbability = prob;
        prevDifficulty = difficulty;
        prevMinutes = minutes;
    }
    return prevProbability *1000 * 1000;
}
