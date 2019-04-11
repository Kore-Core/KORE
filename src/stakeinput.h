// Copyright (c) 2017-2018 The KORE developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef KORE_STAKEINPUT_H
#define KORE_STAKEINPUT_H

class CKeyStore;
class CWallet;
class CWalletTx;

class CStakeInput
{
protected:
    CBlockIndex* pindexFrom;

public:
    virtual ~CStakeInput(){};
    virtual CScript GetScriptPubKey(CWallet* pwallet, CScript& scriptPubKey, bool fisStake = true) = 0; // TODOnGoline: default true
    virtual CBlockIndex* GetIndexFrom() = 0;
    virtual bool CreateTxIn(CWallet* pwallet, CTxIn& txIn, uint256 hashTxOut = 0) = 0;
    virtual bool GetTxFrom(CTransaction& tx) = 0;
    virtual CAmount GetValue() = 0;
    virtual bool CreateLockingTxOuts(CWallet* pwallet, vector<CTxOut>& vout, CAmount value) = 0;
    virtual bool CreateTxOut(CWallet* pwallet, CTxOut& txOut) = 0;
    virtual bool GetModifier(uint64_t& nStakeModifier) = 0;
    virtual uint256 GetOldModifier(bool isProofOfStake) = 0;
    virtual CDataStream GetUniqueness() = 0;
    virtual int GetPosition() = 0;
};


class CKoreStake : public CStakeInput
{
private:
    CTransaction txFrom;
    unsigned int nPosition;
    txnouttype ptxType;

public:
    CKoreStake()
    {
        this->pindexFrom = nullptr;
    }

    CScript GetScriptPubKey(CWallet* pwallet, CScript& scriptPubKey, bool fisStake = true) override;
    bool SetInput(CTransaction txPrev, unsigned int n);

    CBlockIndex* GetIndexFrom() override;
    bool GetTxFrom(CTransaction& tx) override;
    CAmount GetValue() override;
    bool GetModifier(uint64_t& nStakeModifier) override;
    uint256 GetOldModifier(bool isProofOfStake) override;
    CDataStream GetUniqueness() override;
    bool CreateTxIn(CWallet* pwallet, CTxIn& txIn, uint256 hashTxOut = 0) override;
    virtual bool CreateLockingTxOuts(CWallet* pwallet, vector<CTxOut>& vout, CAmount value) override;
    virtual bool CreateTxOut(CWallet* pwallet, CTxOut& txOut) override;
    int GetPosition() override;
};


#endif //KORE_STAKEINPUT_H
