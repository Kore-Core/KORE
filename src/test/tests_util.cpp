

#include "arith_uint256.h"
#include "blocksignature.h"
#include "legacy/consensus/merkle.h"
#include "main.h"
#include "miner.h"
#include "primitives/block.h"
#include "pubkey.h"
#include "tests_util.h"
#include "txdb.h"
#include "uint256.h"
#include "util.h"
#include "utiltime.h"
#include "validationinterface.h"
#include "wallet.h"


#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/tuple/tuple.hpp>


CWallet* pwalletMain;

static CCoinsViewDB* pcoinsdbview = NULL;

CCoinsViewCache* SaveDatabaseState()
{
    // This method is based on the PrepareShutdown
    
    //if (pwalletMain)
    //    bitdb.Flush(false);        

    {
        LOCK(cs_main);
        if (pcoinsTip != NULL) {
            FlushStateToDisk();

            //record that client took the proper shutdown procedure
            pblocktree->WriteFlag("shutdown", true);
        }

        delete pcoinsTip;
        pcoinsTip = NULL;
        delete pcoinsdbview;
        delete pblocktree;
    }

    if (pwalletMain)
        bitdb.Flush(false);
        
}

bool ReadDatabaseState()
{
    string strBlockIndexError = "";
    std::string strLoadError;
    UnloadBlockIndex();

    pcoinsdbview = new CCoinsViewDB(1 << 23, false);
    pblocktree = new CBlockTreeDB(1 << 20, false);
    pcoinsTip = new CCoinsViewCache(pcoinsdbview);

    if (!LoadBlockIndex()) {
        strLoadError = _("Error loading block database");
        strLoadError = strprintf("%s : %s", strLoadError, strBlockIndexError);
        LogPrintf("ReadDatabaseState %s \n", strLoadError);
        return false;
    }

    // Initialize the block index (no-op if non-empty database was already loaded)
    if (!InitBlockIndex()) {
        strLoadError = _("Error initializing block database");
        LogPrintf("ReadDatabaseState %s \n", strLoadError);
        return false;
    }
    if (!CVerifyDB().VerifyDB(pcoinsdbview, GetArg("-checklevel", DEFAULT_CHECKLEVEL),
            GetArg("-checkblocks", DEFAULT_CHECKBLOCKS))) {
        strLoadError = _("Corrupted block database detected");
        LogPrintf("ReadDatabaseState %s \n", strLoadError);
        return false;
    }

    return true;
}

void CheckDatabaseState(CWallet* pwalletMain)
{
    SaveDatabaseState();

    BOOST_CHECK(ReadDatabaseState());
}

void InitializeDBTest(const boost::filesystem::path & path)
{
    
#ifdef ENABLE_WALLET
    bitdb.MakeMock();
#endif
    boost::filesystem::create_directories(path / "unittest" / "blocks");
    pcoinsdbview = new CCoinsViewDB(1 << 23, false);
    pblocktree = new CBlockTreeDB(1 << 20, false);
    pcoinsTip = new CCoinsViewCache(pcoinsdbview);
    InitBlockIndex();
#ifdef ENABLE_WALLET
    bool fFirstRun;
    pwalletMain = new CWallet("wallet.dat");
    pwalletMain->LoadWallet(fFirstRun);
    RegisterValidationInterface(pwalletMain);
#endif
}

void FinalizeDBTest(bool shutdown) 
{
#ifdef ENABLE_WALLET
    bitdb.Flush(shutdown);
    //bitdb.Close();
#endif
    delete pcoinsTip;
    delete pcoinsdbview;
    delete pblocktree;
#ifdef ENABLE_WALLET
    UnregisterValidationInterface(pwalletMain);
    delete pwalletMain;
    pwalletMain = NULL;
#endif
}

/*
CBlock ============================>>>>
    hash=0f1d9c8d3cc8cd3dc9cac4e393dec1e7fc7caf2e7f60918caa32c93700b54a95 
    ver=1 
    hashPrevBlock=0aab10677b4fe0371a67f99e78a69e7d9fa03a1c7d48747978da405dc5abeb99, 
    hashMerkleRoot=283a74c16816416ac46e0bda045e009fbeb04b61c23eb92ea15d35966374214e, 
    nTime=1553431167, 
    nBits=201fffff, 
    nNonce=6, 
    nBirthdayA=51845350, 
    nBirthdayB=53515334, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=283a74c168, ver=1,  nTime=1553431167, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 510101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    283a74c16816416ac46e0bda045e009fbeb04b61c23eb92ea15d35966374214e
CBlock <<<<============================

{0, 1553431167, 1553431167 , 538968063 , 6 , 1 , 51845350 , 53515334 , uint256("0f1d9c8d3cc8cd3dc9cac4e393dec1e7fc7caf2e7f60918caa32c93700b54a95") , uint256("283a74c16816416ac46e0bda045e009fbeb04b61c23eb92ea15d35966374214e") , 0 }, // Block 1

CBlock ============================>>>>
    hash=107aab84ec63a96209af139c522c9e26cc1ce3c2991cbd234cdff3f148be13d4 
    ver=1 
    hashPrevBlock=0f1d9c8d3cc8cd3dc9cac4e393dec1e7fc7caf2e7f60918caa32c93700b54a95, 
    hashMerkleRoot=387dae07ab324b98e2bcd8331eded8915d89a2074d1716e703b29630e98c044f, 
    nTime=1553431187, 
    nBits=201fffff, 
    nNonce=18, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=387dae07ab, ver=1,  nTime=1553431187, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 520101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    387dae07ab324b98e2bcd8331eded8915d89a2074d1716e703b29630e98c044f
CBlock <<<<============================

{0, 1553431187, 1553431187 , 538968063 , 18 , 1 , 0 , 0 , uint256("107aab84ec63a96209af139c522c9e26cc1ce3c2991cbd234cdff3f148be13d4") , uint256("387dae07ab324b98e2bcd8331eded8915d89a2074d1716e703b29630e98c044f") , 0 }, // Block 2
CBlock ============================>>>>
    hash=02252b1bbf3c4d4395e9d91f65a7d6f39b0bbca5b5f37d815eea23812596f116 
    ver=1 
    hashPrevBlock=107aab84ec63a96209af139c522c9e26cc1ce3c2991cbd234cdff3f148be13d4, 
    hashMerkleRoot=5cb269175f939f4bb2cf64b89152723fc993cc40a012490891eaae22d6b97dea, 
    nTime=1553431207, 
    nBits=201fffff, 
    nNonce=13, 
    nBirthdayA=9910726, 
    nBirthdayB=18271698, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=5cb269175f, ver=1,  nTime=1553431207, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 530101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    5cb269175f939f4bb2cf64b89152723fc993cc40a012490891eaae22d6b97dea
CBlock <<<<============================

{0, 1553431207, 1553431207 , 538968063 , 13 , 1 , 9910726 , 18271698 , uint256("02252b1bbf3c4d4395e9d91f65a7d6f39b0bbca5b5f37d815eea23812596f116") , uint256("5cb269175f939f4bb2cf64b89152723fc993cc40a012490891eaae22d6b97dea") , 0 }, // Block 3
CBlock ============================>>>>
    hash=0cae580325c780b52d1e4367ac896cf826ff1511547d6be545c7840d87899114 
    ver=1 
    hashPrevBlock=02252b1bbf3c4d4395e9d91f65a7d6f39b0bbca5b5f37d815eea23812596f116, 
    hashMerkleRoot=7bbe3e96ef9ca80cb66ca9ef73eb9c5e3384ad05d8374f240442195f8e2d8be3, 
    nTime=1553431227, 
    nBits=201fffff, 
    nNonce=5, 
    nBirthdayA=19067498, 
    nBirthdayB=51658456, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=7bbe3e96ef, ver=1,  nTime=1553431227, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 540101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    7bbe3e96ef9ca80cb66ca9ef73eb9c5e3384ad05d8374f240442195f8e2d8be3
CBlock <<<<============================

{0, 1553431227, 1553431227 , 538968063 , 5 , 1 , 19067498 , 51658456 , uint256("0cae580325c780b52d1e4367ac896cf826ff1511547d6be545c7840d87899114") , uint256("7bbe3e96ef9ca80cb66ca9ef73eb9c5e3384ad05d8374f240442195f8e2d8be3") , 900000000000 }, // Block 4
CBlock ============================>>>>
    hash=072eb9afa2bbe57a2a0ba63b1efbe969174352b3e5c79328e511e32a1cd3387d 
    ver=1 
    hashPrevBlock=0cae580325c780b52d1e4367ac896cf826ff1511547d6be545c7840d87899114, 
    hashMerkleRoot=1d34fbae18d949fbe0252e5f6f52411d37ed45f5dc2e01607a166d654c8169ab, 
    nTime=1553431247, 
    nBits=201fffff, 
    nNonce=11, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=1d34fbae18, ver=1,  nTime=1553431247, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 550101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    1d34fbae18d949fbe0252e5f6f52411d37ed45f5dc2e01607a166d654c8169ab
CBlock <<<<============================

{0, 1553431247, 1553431247 , 538968063 , 11 , 1 , 0 , 0 , uint256("072eb9afa2bbe57a2a0ba63b1efbe969174352b3e5c79328e511e32a1cd3387d") , uint256("1d34fbae18d949fbe0252e5f6f52411d37ed45f5dc2e01607a166d654c8169ab") , 1800000000000 }, // Block 5
CBlock ============================>>>>
    hash=03423cb0018affcfb9452473df7aaf4762b7510b1767eb9d88dea59b261b7fc4 
    ver=1 
    hashPrevBlock=072eb9afa2bbe57a2a0ba63b1efbe969174352b3e5c79328e511e32a1cd3387d, 
    hashMerkleRoot=b75a1ff164b2b807871305963ec461b17f63d1f8a95c93a3f5d404d8e9da7b11, 
    nTime=1553431267, 
    nBits=201fffff, 
    nNonce=4, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=b75a1ff164, ver=1,  nTime=1553431267, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 560101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    b75a1ff164b2b807871305963ec461b17f63d1f8a95c93a3f5d404d8e9da7b11
CBlock <<<<============================

{0, 1553431267, 1553431267 , 538968063 , 4 , 1 , 0 , 0 , uint256("03423cb0018affcfb9452473df7aaf4762b7510b1767eb9d88dea59b261b7fc4") , uint256("b75a1ff164b2b807871305963ec461b17f63d1f8a95c93a3f5d404d8e9da7b11") , 2700000000000 }, // Block 6
CBlock ============================>>>>
    hash=119f013ab1d6be26ed7075eec6c96de59a50da481d513d2b08cfb1597319f431 
    ver=1 
    hashPrevBlock=03423cb0018affcfb9452473df7aaf4762b7510b1767eb9d88dea59b261b7fc4, 
    hashMerkleRoot=f6b04f1c9c529973a1919a4348187e5c6a9e76541265ee13949c86c3d7167230, 
    nTime=1553431287, 
    nBits=201fffff, 
    nNonce=2, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=f6b04f1c9c, ver=1,  nTime=1553431287, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 570101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    f6b04f1c9c529973a1919a4348187e5c6a9e76541265ee13949c86c3d7167230
CBlock <<<<============================

{0, 1553431287, 1553431287 , 538968063 , 2 , 1 , 0 , 0 , uint256("119f013ab1d6be26ed7075eec6c96de59a50da481d513d2b08cfb1597319f431") , uint256("f6b04f1c9c529973a1919a4348187e5c6a9e76541265ee13949c86c3d7167230") , 3600000000000 }, // Block 7
CBlock ============================>>>>
    hash=07601619c49dc71f6984b6914334484bded5bf1fa3c6aa335cd0a2e82eb92830 
    ver=1 
    hashPrevBlock=119f013ab1d6be26ed7075eec6c96de59a50da481d513d2b08cfb1597319f431, 
    hashMerkleRoot=e70df2d3c92c12eb4573d95ad5a01ad335f53b853f51538b01ea8339f1b9c5cd, 
    nTime=1553431307, 
    nBits=201fffff, 
    nNonce=9, 
    nBirthdayA=18700191, 
    nBirthdayB=25291895, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=e70df2d3c9, ver=1,  nTime=1553431307, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 580101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    e70df2d3c92c12eb4573d95ad5a01ad335f53b853f51538b01ea8339f1b9c5cd
CBlock <<<<============================

{0, 1553431307, 1553431307 , 538968063 , 9 , 1 , 18700191 , 25291895 , uint256("07601619c49dc71f6984b6914334484bded5bf1fa3c6aa335cd0a2e82eb92830") , uint256("e70df2d3c92c12eb4573d95ad5a01ad335f53b853f51538b01ea8339f1b9c5cd") , 4500000000000 }, // Block 8
CBlock ============================>>>>
    hash=0ea0ad704b03f3b2fe953f8c23db043f660e3f24ed4ce8483d4dc53797119baf 
    ver=1 
    hashPrevBlock=07601619c49dc71f6984b6914334484bded5bf1fa3c6aa335cd0a2e82eb92830, 
    hashMerkleRoot=52c3a6613d0de34bcd41828c6f03bcd814e287d08f46c31022eac0cf2b6aaa03, 
    nTime=1553431327, 
    nBits=201fffff, 
    nNonce=6, 
    nBirthdayA=25384411, 
    nBirthdayB=43369449, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=52c3a6613d, ver=1,  nTime=1553431327, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 590101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    52c3a6613d0de34bcd41828c6f03bcd814e287d08f46c31022eac0cf2b6aaa03
CBlock <<<<============================

{0, 1553431327, 1553431327 , 538968063 , 6 , 1 , 25384411 , 43369449 , uint256("0ea0ad704b03f3b2fe953f8c23db043f660e3f24ed4ce8483d4dc53797119baf") , uint256("52c3a6613d0de34bcd41828c6f03bcd814e287d08f46c31022eac0cf2b6aaa03") , 5400000000000 }, // Block 9
CBlock ============================>>>>
    hash=04a473ccc463700aa652b695cf4c4ba771f602e56a2e1c8fa3211cf3ffd9f8bb 
    ver=1 
    hashPrevBlock=0ea0ad704b03f3b2fe953f8c23db043f660e3f24ed4ce8483d4dc53797119baf, 
    hashMerkleRoot=033593578ad159074b3757d350034a7121fe257fb03ecbec3de86bfca60bf63c, 
    nTime=1553431347, 
    nBits=201fffff, 
    nNonce=2, 
    nBirthdayA=19096856, 
    nBirthdayB=54986782, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=033593578a, ver=1,  nTime=1553431347, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 5a0101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    033593578ad159074b3757d350034a7121fe257fb03ecbec3de86bfca60bf63c
CBlock <<<<============================

{0, 1553431347, 1553431347 , 538968063 , 2 , 1 , 19096856 , 54986782 , uint256("04a473ccc463700aa652b695cf4c4ba771f602e56a2e1c8fa3211cf3ffd9f8bb") , uint256("033593578ad159074b3757d350034a7121fe257fb03ecbec3de86bfca60bf63c") , 6300000000000 }, // Block 10
CBlock ============================>>>>
    hash=0559717d7507f322630fff794ba30c5e3bfd1238cb05c22106362d07cb8725c6 
    ver=1 
    hashPrevBlock=04a473ccc463700aa652b695cf4c4ba771f602e56a2e1c8fa3211cf3ffd9f8bb, 
    hashMerkleRoot=bc81ba0c52eb8666604ae29c6d56823a5bb72f622ee5b1a88a774961b85ff720, 
    nTime=1553431367, 
    nBits=201fffff, 
    nNonce=4, 
    nBirthdayA=33170299, 
    nBirthdayB=65785478, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=bc81ba0c52, ver=1,  nTime=1553431367, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 5b0101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    bc81ba0c52eb8666604ae29c6d56823a5bb72f622ee5b1a88a774961b85ff720
CBlock <<<<============================

{0, 1553431367, 1553431367 , 538968063 , 4 , 1 , 33170299 , 65785478 , uint256("0559717d7507f322630fff794ba30c5e3bfd1238cb05c22106362d07cb8725c6") , uint256("bc81ba0c52eb8666604ae29c6d56823a5bb72f622ee5b1a88a774961b85ff720") , 7200000000000 }, // Block 11
CBlock ============================>>>>
    hash=1ce80a9da65026c2c17e05651e85fb687eb118352f4f40bdb0bc4a0cf97ab7ba 
    ver=1 
    hashPrevBlock=0559717d7507f322630fff794ba30c5e3bfd1238cb05c22106362d07cb8725c6, 
    hashMerkleRoot=af524765d4faa9682b47d8abe5900a46377b40b3377216b04253de04213b2865, 
    nTime=1553431387, 
    nBits=201fffff, 
    nNonce=15, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=af524765d4, ver=1,  nTime=1553431387, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 5c0101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    af524765d4faa9682b47d8abe5900a46377b40b3377216b04253de04213b2865
CBlock <<<<============================

{0, 1553431387, 1553431387 , 538968063 , 15 , 1 , 0 , 0 , uint256("1ce80a9da65026c2c17e05651e85fb687eb118352f4f40bdb0bc4a0cf97ab7ba") , uint256("af524765d4faa9682b47d8abe5900a46377b40b3377216b04253de04213b2865") , 8100000000000 }, // Block 12

CBlock ============================>>>>
    hash=05c4a54c127e07c96d00f417b2dd59f8471ed99dc80318875d29924412a0da52 
    ver=1 
    hashPrevBlock=1ce80a9da65026c2c17e05651e85fb687eb118352f4f40bdb0bc4a0cf97ab7ba, 
    hashMerkleRoot=6af5a5b978237bf0df6f352a99307c1aeda47a0c0be7dc865d5174003e0fe153, 
    nTime=1553431407, 
    nBits=201fffff, 
    nNonce=1, 
    nBirthdayA=17325978, 
    nBirthdayB=18828871, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=6af5a5b978, ver=1,  nTime=1553431407, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 5d0101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    6af5a5b978237bf0df6f352a99307c1aeda47a0c0be7dc865d5174003e0fe153
CBlock <<<<============================

{0, 1553431407, 1553431407 , 538968063 , 1 , 1 , 17325978 , 18828871 , uint256("05c4a54c127e07c96d00f417b2dd59f8471ed99dc80318875d29924412a0da52") , uint256("6af5a5b978237bf0df6f352a99307c1aeda47a0c0be7dc865d5174003e0fe153") , 9000000000000 }, // Block 13
CBlock ============================>>>>
    hash=05b8b9e1b1b37571341c58df5fd4facf9a0f50639de078fa63279c3a2cc9a00e 
    ver=1 
    hashPrevBlock=05c4a54c127e07c96d00f417b2dd59f8471ed99dc80318875d29924412a0da52, 
    hashMerkleRoot=3741e99159b6d87203d8df4e777a913d5f7a4357f584b1c2ba11c8b276bc44f2, 
    nTime=1553431427, 
    nBits=201fffff, 
    nNonce=1, 
    nBirthdayA=42369479, 
    nBirthdayB=43637928, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=3741e99159, ver=1,  nTime=1553431427, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 5e0101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    3741e99159b6d87203d8df4e777a913d5f7a4357f584b1c2ba11c8b276bc44f2
CBlock <<<<============================

{0, 1553431427, 1553431427 , 538968063 , 1 , 1 , 42369479 , 43637928 , uint256("05b8b9e1b1b37571341c58df5fd4facf9a0f50639de078fa63279c3a2cc9a00e") , uint256("3741e99159b6d87203d8df4e777a913d5f7a4357f584b1c2ba11c8b276bc44f2") , 9900000000000 }, // Block 14

CBlock ============================>>>>
    hash=0dc0073d7bbd97245b43c22b741d5adcb4ad021734cd2a2044289078dbcdbae1 
    ver=1 
    hashPrevBlock=05b8b9e1b1b37571341c58df5fd4facf9a0f50639de078fa63279c3a2cc9a00e, 
    hashMerkleRoot=bc948a60efe6d2a856ee2adb28046fbfd4f1cf8486260a7a3bb5176b1b41626d, 
    nTime=1553431447, 
    nBits=201fffff, 
    nNonce=7, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=bc948a60ef, ver=1,  nTime=1553431447, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 5f0101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    bc948a60efe6d2a856ee2adb28046fbfd4f1cf8486260a7a3bb5176b1b41626d
CBlock <<<<============================

{0, 1553431447, 1553431447 , 538968063 , 7 , 1 , 0 , 0 , uint256("0dc0073d7bbd97245b43c22b741d5adcb4ad021734cd2a2044289078dbcdbae1") , uint256("bc948a60efe6d2a856ee2adb28046fbfd4f1cf8486260a7a3bb5176b1b41626d") , 10800000000000 }, // Block 15
CBlock ============================>>>>
    hash=11b1820f994bc3cf939b348e877ab77d35f65e869efd087c24a5c2f5802e5a3f 
    ver=1 
    hashPrevBlock=0dc0073d7bbd97245b43c22b741d5adcb4ad021734cd2a2044289078dbcdbae1, 
    hashMerkleRoot=8232dfa9dc01eb79c47785dd9e9a2e8486e8ca9fff1feb2a4c51657062a8e755, 
    nTime=1553431467, 
    nBits=201fffff, 
    nNonce=11, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=8232dfa9dc, ver=1,  nTime=1553431467, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 600101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    8232dfa9dc01eb79c47785dd9e9a2e8486e8ca9fff1feb2a4c51657062a8e755
CBlock <<<<============================

{0, 1553431467, 1553431467 , 538968063 , 11 , 1 , 0 , 0 , uint256("11b1820f994bc3cf939b348e877ab77d35f65e869efd087c24a5c2f5802e5a3f") , uint256("8232dfa9dc01eb79c47785dd9e9a2e8486e8ca9fff1feb2a4c51657062a8e755") , 11700000000000 }, // Block 16
CBlock ============================>>>>
    hash=01482d703276be0b1b02bd50e4d09e7d54c721cb0c59a36109ac5a2d0be8841d 
    ver=1 
    hashPrevBlock=11b1820f994bc3cf939b348e877ab77d35f65e869efd087c24a5c2f5802e5a3f, 
    hashMerkleRoot=1f28000536ede468cbdf20b5d9f6e641fb114af0c34b52e359b7fbd2d04ae957, 
    nTime=1553431487, 
    nBits=201fffff, 
    nNonce=1, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=1f28000536, ver=1,  nTime=1553431487, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 01110101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    1f28000536ede468cbdf20b5d9f6e641fb114af0c34b52e359b7fbd2d04ae957
CBlock <<<<============================

{0, 1553431487, 1553431487 , 538968063 , 1 , 1 , 0 , 0 , uint256("01482d703276be0b1b02bd50e4d09e7d54c721cb0c59a36109ac5a2d0be8841d") , uint256("1f28000536ede468cbdf20b5d9f6e641fb114af0c34b52e359b7fbd2d04ae957") , 12600000000000 }, // Block 17
CBlock ============================>>>>
    hash=0a505d61eb1f16a672efbeb77a6fc151a32281f0bce43a744491573d2bcf0344 
    ver=1 
    hashPrevBlock=01482d703276be0b1b02bd50e4d09e7d54c721cb0c59a36109ac5a2d0be8841d, 
    hashMerkleRoot=914be36a1934252cc6e89d7c75f4c69d2aec40c16e670570bb2a906b9ab13388, 
    nTime=1553431507, 
    nBits=201fffff, 
    nNonce=2, 
    nBirthdayA=22830698, 
    nBirthdayB=65149328, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=914be36a19, ver=1,  nTime=1553431507, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 01120101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    914be36a1934252cc6e89d7c75f4c69d2aec40c16e670570bb2a906b9ab13388
CBlock <<<<============================

{0, 1553431507, 1553431507 , 538968063 , 2 , 1 , 22830698 , 65149328 , uint256("0a505d61eb1f16a672efbeb77a6fc151a32281f0bce43a744491573d2bcf0344") , uint256("914be36a1934252cc6e89d7c75f4c69d2aec40c16e670570bb2a906b9ab13388") , 13500000000000 }, // Block 18
CBlock ============================>>>>
    hash=0cb8014a7f447cd25e8bc757fe291e16ccc994728ed00c499cb91511d63ea23b 
    ver=1 
    hashPrevBlock=0a505d61eb1f16a672efbeb77a6fc151a32281f0bce43a744491573d2bcf0344, 
    hashMerkleRoot=9db7a8bce36bbc45c9530c5545809869f2baf600d035eea810d09193cfe7d368, 
    nTime=1553431527, 
    nBits=201fffff, 
    nNonce=2, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=9db7a8bce3, ver=1,  nTime=1553431527, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 01130101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    9db7a8bce36bbc45c9530c5545809869f2baf600d035eea810d09193cfe7d368
CBlock <<<<============================

{0, 1553431527, 1553431527 , 538968063 , 2 , 1 , 0 , 0 , uint256("0cb8014a7f447cd25e8bc757fe291e16ccc994728ed00c499cb91511d63ea23b") , uint256("9db7a8bce36bbc45c9530c5545809869f2baf600d035eea810d09193cfe7d368") , 14400000000000 }, // Block 19
CBlock ============================>>>>
    hash=01ccdf5b4c7787c7ee75a0eccc8fad58717487a8418b77d68bc6311b78fcbe10 
    ver=1 
    hashPrevBlock=0cb8014a7f447cd25e8bc757fe291e16ccc994728ed00c499cb91511d63ea23b, 
    hashMerkleRoot=217259941931d3aad64f03410cadfbb1f4d7acf019d2baa7dd918c0ecfb18107, 
    nTime=1553431547, 
    nBits=201fffff, 
    nNonce=6, 
    nBirthdayA=17277539, 
    nBirthdayB=32090829, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=2172599419, ver=1,  nTime=1553431547, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 01140101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    217259941931d3aad64f03410cadfbb1f4d7acf019d2baa7dd918c0ecfb18107
CBlock <<<<============================

{0, 1553431547, 1553431547 , 538968063 , 6 , 1 , 17277539 , 32090829 , uint256("01ccdf5b4c7787c7ee75a0eccc8fad58717487a8418b77d68bc6311b78fcbe10") , uint256("217259941931d3aad64f03410cadfbb1f4d7acf019d2baa7dd918c0ecfb18107") , 15300000000000 }, // Block 20
CBlock ============================>>>>
    hash=107d8e1967a3aaa3df7e89297fddbe47d0302e28c36ce47ec3fb62bc662e3798 
    ver=1 
    hashPrevBlock=01ccdf5b4c7787c7ee75a0eccc8fad58717487a8418b77d68bc6311b78fcbe10, 
    hashMerkleRoot=fe398043614657aef184d9efb40f74db8aa661bd698e8c1739680519b0a1b8cb, 
    nTime=1553431567, 
    nBits=201fffff, 
    nNonce=2, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=fe39804361, ver=1,  nTime=1553431567, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 01150101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    fe398043614657aef184d9efb40f74db8aa661bd698e8c1739680519b0a1b8cb
CBlock <<<<============================

{0, 1553431567, 1553431567 , 538968063 , 2 , 1 , 0 , 0 , uint256("107d8e1967a3aaa3df7e89297fddbe47d0302e28c36ce47ec3fb62bc662e3798") , uint256("fe398043614657aef184d9efb40f74db8aa661bd698e8c1739680519b0a1b8cb") , 16200000000000 }, // Block 21
CBlock ============================>>>>
    hash=0651f80e581655f63d3417b2a5dcac3ce338e9d70ae155a7547e46c20da79629 
    ver=1 
    hashPrevBlock=107d8e1967a3aaa3df7e89297fddbe47d0302e28c36ce47ec3fb62bc662e3798, 
    hashMerkleRoot=e1f66a80f6051657855d1340a180be7d194681beda9210bdd07707b3b1a6590b, 
    nTime=1553431587, 
    nBits=201fffff, 
    nNonce=1, 
    nBirthdayA=11150915, 
    nBirthdayB=12984773, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=e1f66a80f6, ver=1,  nTime=1553431587, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 01160101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    e1f66a80f6051657855d1340a180be7d194681beda9210bdd07707b3b1a6590b
CBlock <<<<============================

{0, 1553431587, 1553431587 , 538968063 , 1 , 1 , 11150915 , 12984773 , uint256("0651f80e581655f63d3417b2a5dcac3ce338e9d70ae155a7547e46c20da79629") , uint256("e1f66a80f6051657855d1340a180be7d194681beda9210bdd07707b3b1a6590b") , 17100000000000 }, // Block 22
CBlock ============================>>>>
    hash=060cd8bdffe5c309aac63291b98b954de36b606d243d15e28d82f86e4a196407 
    ver=1 
    hashPrevBlock=0651f80e581655f63d3417b2a5dcac3ce338e9d70ae155a7547e46c20da79629, 
    hashMerkleRoot=03d58645edeae85b6135d117daeab55db27bb84acebaf81dc51683cc3b9b5c52, 
    nTime=1553431607, 
    nBits=201fffff, 
    nNonce=9, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=03d58645ed, ver=1,  nTime=1553431607, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 01170101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    03d58645edeae85b6135d117daeab55db27bb84acebaf81dc51683cc3b9b5c52
CBlock <<<<============================

{0, 1553431607, 1553431607 , 538968063 , 9 , 1 , 0 , 0 , uint256("060cd8bdffe5c309aac63291b98b954de36b606d243d15e28d82f86e4a196407") , uint256("03d58645edeae85b6135d117daeab55db27bb84acebaf81dc51683cc3b9b5c52") , 18000000000000 }, // Block 23
CBlock ============================>>>>
    hash=145d11423a04763ad670d41fe65ce36479532ef2027329f3efc7752954d00a3f 
    ver=1 
    hashPrevBlock=060cd8bdffe5c309aac63291b98b954de36b606d243d15e28d82f86e4a196407, 
    hashMerkleRoot=691372ab4a67fac384d4418194ed1d32cdd7daff94fd035e1ec98c8ef8b4c799, 
    nTime=1553431627, 
    nBits=201fffff, 
    nNonce=3, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=691372ab4a, ver=1,  nTime=1553431627, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 01180101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    691372ab4a67fac384d4418194ed1d32cdd7daff94fd035e1ec98c8ef8b4c799
CBlock <<<<============================

{0, 1553431627, 1553431627 , 538968063 , 3 , 1 , 0 , 0 , uint256("145d11423a04763ad670d41fe65ce36479532ef2027329f3efc7752954d00a3f") , uint256("691372ab4a67fac384d4418194ed1d32cdd7daff94fd035e1ec98c8ef8b4c799") , 18900000000000 }, // Block 24
CBlock ============================>>>>
    hash=0e37eba9ad001a42113014ee70b834b0162417a87bd9a562daedb680fd4dd116 
    ver=1 
    hashPrevBlock=145d11423a04763ad670d41fe65ce36479532ef2027329f3efc7752954d00a3f, 
    hashMerkleRoot=2dd8088941f37a5993dd2a2f90d9691f53f9e7df341a7b44ed09554ee16222b2, 
    nTime=1553431647, 
    nBits=201fffff, 
    nNonce=4, 
    nBirthdayA=1165095, 
    nBirthdayB=42231805, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=2dd8088941, ver=1,  nTime=1553431647, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 01190101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    2dd8088941f37a5993dd2a2f90d9691f53f9e7df341a7b44ed09554ee16222b2
CBlock <<<<============================

{0, 1553431647, 1553431647 , 538968063 , 4 , 1 , 1165095 , 42231805 , uint256("0e37eba9ad001a42113014ee70b834b0162417a87bd9a562daedb680fd4dd116") , uint256("2dd8088941f37a5993dd2a2f90d9691f53f9e7df341a7b44ed09554ee16222b2") , 19800000000000 }, // Block 25
CBlock ============================>>>>
    hash=0b67fbf670c6b89166d8a56989998185028f980faf168b7522f20ab5bde7685d 
    ver=1 
    hashPrevBlock=0e37eba9ad001a42113014ee70b834b0162417a87bd9a562daedb680fd4dd116, 
    hashMerkleRoot=b4179e6d2d38d6bad58620875d57f7c7240fc4554b4c6c5cc273d9ec612a90b3, 
    nTime=1553431667, 
    nBits=201fffff, 
    nNonce=1, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=b4179e6d2d, ver=1,  nTime=1553431667, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 011a0101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    b4179e6d2d38d6bad58620875d57f7c7240fc4554b4c6c5cc273d9ec612a90b3
CBlock <<<<============================

{0, 1553431667, 1553431667 , 538968063 , 1 , 1 , 0 , 0 , uint256("0b67fbf670c6b89166d8a56989998185028f980faf168b7522f20ab5bde7685d") , uint256("b4179e6d2d38d6bad58620875d57f7c7240fc4554b4c6c5cc273d9ec612a90b3") , 20700000000000 }, // Block 26
CBlock ============================>>>>
    hash=06648ca38ee38467453bfab5b512641c353be60bf381b11a60fca01bc6289f79 
    ver=1 
    hashPrevBlock=0b67fbf670c6b89166d8a56989998185028f980faf168b7522f20ab5bde7685d, 
    hashMerkleRoot=6e104a139a739ef27340222bdc5eba1abed8b703e4b6dab7bcff385e216a9292, 
    nTime=1553431687, 
    nBits=201fffff, 
    nNonce=9, 
    nBirthdayA=5986303, 
    nBirthdayB=45133333, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=6e104a139a, ver=1,  nTime=1553431687, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 011b0101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    6e104a139a739ef27340222bdc5eba1abed8b703e4b6dab7bcff385e216a9292
CBlock <<<<============================

{0, 1553431687, 1553431687 , 538968063 , 9 , 1 , 5986303 , 45133333 , uint256("06648ca38ee38467453bfab5b512641c353be60bf381b11a60fca01bc6289f79") , uint256("6e104a139a739ef27340222bdc5eba1abed8b703e4b6dab7bcff385e216a9292") , 21600000000000 }, // Block 27
CBlock ============================>>>>
    hash=18f0a438d36370b91f148783366a0c631eb7ec5b16e2e9a9ece463c993d22d70 
    ver=1 
    hashPrevBlock=06648ca38ee38467453bfab5b512641c353be60bf381b11a60fca01bc6289f79, 
    hashMerkleRoot=8665703f83ded2e1a564bda40df6bb34d9bdb6986caa1f0a8c9222f4734c034d, 
    nTime=1553431707, 
    nBits=201fffff, 
    nNonce=7, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=8665703f83, ver=1,  nTime=1553431707, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 011c0101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    8665703f83ded2e1a564bda40df6bb34d9bdb6986caa1f0a8c9222f4734c034d
CBlock <<<<============================

{0, 1553431707, 1553431707 , 538968063 , 7 , 1 , 0 , 0 , uint256("18f0a438d36370b91f148783366a0c631eb7ec5b16e2e9a9ece463c993d22d70") , uint256("8665703f83ded2e1a564bda40df6bb34d9bdb6986caa1f0a8c9222f4734c034d") , 22500000000000 }, // Block 28
CBlock ============================>>>>
    hash=1c1269fcce54502b1243063a77dc8b9065c5dce448641740cb544eefe2a3f686 
    ver=1 
    hashPrevBlock=18f0a438d36370b91f148783366a0c631eb7ec5b16e2e9a9ece463c993d22d70, 
    hashMerkleRoot=c9b20561dca28d8f86cde5c8b3e2d8d3dde5673ad5eaee08e9cef4a2b32618d6, 
    nTime=1553431727, 
    nBits=201fffff, 
    nNonce=8, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=c9b20561dc, ver=1,  nTime=1553431727, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 011d0101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    c9b20561dca28d8f86cde5c8b3e2d8d3dde5673ad5eaee08e9cef4a2b32618d6
CBlock <<<<============================

{0, 1553431727, 1553431727 , 538968063 , 8 , 1 , 0 , 0 , uint256("1c1269fcce54502b1243063a77dc8b9065c5dce448641740cb544eefe2a3f686") , uint256("c9b20561dca28d8f86cde5c8b3e2d8d3dde5673ad5eaee08e9cef4a2b32618d6") , 23400000000000 }, // Block 29
CBlock ============================>>>>
    hash=07f8b76bfec15f22e0a9addc7467a701c7784832ebb711b3e72ff1127e5e6877 
    ver=1 
    hashPrevBlock=1c1269fcce54502b1243063a77dc8b9065c5dce448641740cb544eefe2a3f686, 
    hashMerkleRoot=785cba513559589287ef2a7b8bc3b130071929ee87a9b5d8853f76cfc6eac70f, 
    nTime=1553431747, 
    nBits=201fffff, 
    nNonce=6, 
    nBirthdayA=27440498, 
    nBirthdayB=42505344, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=785cba5135, ver=1,  nTime=1553431747, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 011e0101)
    CTxOut(nValue=900000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=100000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    785cba513559589287ef2a7b8bc3b130071929ee87a9b5d8853f76cfc6eac70f
CBlock <<<<============================

{0, 1553431747, 1553431747 , 538968063 , 6 , 1 , 27440498 , 42505344 , uint256("07f8b76bfec15f22e0a9addc7467a701c7784832ebb711b3e72ff1127e5e6877") , uint256("785cba513559589287ef2a7b8bc3b130071929ee87a9b5d8853f76cfc6eac70f") , 24300000000000 }, // Block 30


*/

blockinfo_t blockinfo[] = {
{0, 1553431167, 1553431167 , 538968063 , 6 , 1 , 51845350 , 53515334 , uint256("0f1d9c8d3cc8cd3dc9cac4e393dec1e7fc7caf2e7f60918caa32c93700b54a95") , uint256("283a74c16816416ac46e0bda045e009fbeb04b61c23eb92ea15d35966374214e") , 0 }, // Block 1
{0, 1553431187, 1553431187 , 538968063 , 18 , 1 , 0 , 0 , uint256("107aab84ec63a96209af139c522c9e26cc1ce3c2991cbd234cdff3f148be13d4") , uint256("387dae07ab324b98e2bcd8331eded8915d89a2074d1716e703b29630e98c044f") , 0 }, // Block 2
{0, 1553431207, 1553431207 , 538968063 , 13 , 1 , 9910726 , 18271698 , uint256("02252b1bbf3c4d4395e9d91f65a7d6f39b0bbca5b5f37d815eea23812596f116") , uint256("5cb269175f939f4bb2cf64b89152723fc993cc40a012490891eaae22d6b97dea") , 0 }, // Block 3
{0, 1553431227, 1553431227 , 538968063 , 5 , 1 , 19067498 , 51658456 , uint256("0cae580325c780b52d1e4367ac896cf826ff1511547d6be545c7840d87899114") , uint256("7bbe3e96ef9ca80cb66ca9ef73eb9c5e3384ad05d8374f240442195f8e2d8be3") , 900000000000 }, // Block 4
{0, 1553431247, 1553431247 , 538968063 , 11 , 1 , 0 , 0 , uint256("072eb9afa2bbe57a2a0ba63b1efbe969174352b3e5c79328e511e32a1cd3387d") , uint256("1d34fbae18d949fbe0252e5f6f52411d37ed45f5dc2e01607a166d654c8169ab") , 1800000000000 }, // Block 5
{0, 1553431267, 1553431267 , 538968063 , 4 , 1 , 0 , 0 , uint256("03423cb0018affcfb9452473df7aaf4762b7510b1767eb9d88dea59b261b7fc4") , uint256("b75a1ff164b2b807871305963ec461b17f63d1f8a95c93a3f5d404d8e9da7b11") , 2700000000000 }, // Block 6
{0, 1553431287, 1553431287 , 538968063 , 2 , 1 , 0 , 0 , uint256("119f013ab1d6be26ed7075eec6c96de59a50da481d513d2b08cfb1597319f431") , uint256("f6b04f1c9c529973a1919a4348187e5c6a9e76541265ee13949c86c3d7167230") , 3600000000000 }, // Block 7
{0, 1553431307, 1553431307 , 538968063 , 9 , 1 , 18700191 , 25291895 , uint256("07601619c49dc71f6984b6914334484bded5bf1fa3c6aa335cd0a2e82eb92830") , uint256("e70df2d3c92c12eb4573d95ad5a01ad335f53b853f51538b01ea8339f1b9c5cd") , 4500000000000 }, // Block 8
{0, 1553431327, 1553431327 , 538968063 , 6 , 1 , 25384411 , 43369449 , uint256("0ea0ad704b03f3b2fe953f8c23db043f660e3f24ed4ce8483d4dc53797119baf") , uint256("52c3a6613d0de34bcd41828c6f03bcd814e287d08f46c31022eac0cf2b6aaa03") , 5400000000000 }, // Block 9
{0, 1553431347, 1553431347 , 538968063 , 2 , 1 , 19096856 , 54986782 , uint256("04a473ccc463700aa652b695cf4c4ba771f602e56a2e1c8fa3211cf3ffd9f8bb") , uint256("033593578ad159074b3757d350034a7121fe257fb03ecbec3de86bfca60bf63c") , 6300000000000 }, // Block 10
{0, 1553431367, 1553431367 , 538968063 , 4 , 1 , 33170299 , 65785478 , uint256("0559717d7507f322630fff794ba30c5e3bfd1238cb05c22106362d07cb8725c6") , uint256("bc81ba0c52eb8666604ae29c6d56823a5bb72f622ee5b1a88a774961b85ff720") , 7200000000000 }, // Block 11
{0, 1553431387, 1553431387 , 538968063 , 15 , 1 , 0 , 0 , uint256("1ce80a9da65026c2c17e05651e85fb687eb118352f4f40bdb0bc4a0cf97ab7ba") , uint256("af524765d4faa9682b47d8abe5900a46377b40b3377216b04253de04213b2865") , 8100000000000 }, // Block 12
{0, 1553431407, 1553431407 , 538968063 , 1 , 1 , 17325978 , 18828871 , uint256("05c4a54c127e07c96d00f417b2dd59f8471ed99dc80318875d29924412a0da52") , uint256("6af5a5b978237bf0df6f352a99307c1aeda47a0c0be7dc865d5174003e0fe153") , 9000000000000 }, // Block 13
{0, 1553431427, 1553431427 , 538968063 , 1 , 1 , 42369479 , 43637928 , uint256("05b8b9e1b1b37571341c58df5fd4facf9a0f50639de078fa63279c3a2cc9a00e") , uint256("3741e99159b6d87203d8df4e777a913d5f7a4357f584b1c2ba11c8b276bc44f2") , 9900000000000 }, // Block 14
{0, 1553431447, 1553431447 , 538968063 , 7 , 1 , 0 , 0 , uint256("0dc0073d7bbd97245b43c22b741d5adcb4ad021734cd2a2044289078dbcdbae1") , uint256("bc948a60efe6d2a856ee2adb28046fbfd4f1cf8486260a7a3bb5176b1b41626d") , 10800000000000 }, // Block 15
{0, 1553431467, 1553431467 , 538968063 , 11 , 1 , 0 , 0 , uint256("11b1820f994bc3cf939b348e877ab77d35f65e869efd087c24a5c2f5802e5a3f") , uint256("8232dfa9dc01eb79c47785dd9e9a2e8486e8ca9fff1feb2a4c51657062a8e755") , 11700000000000 }, // Block 16
{0, 1553431487, 1553431487 , 538968063 , 1 , 1 , 0 , 0 , uint256("01482d703276be0b1b02bd50e4d09e7d54c721cb0c59a36109ac5a2d0be8841d") , uint256("1f28000536ede468cbdf20b5d9f6e641fb114af0c34b52e359b7fbd2d04ae957") , 12600000000000 }, // Block 17
{0, 1553431507, 1553431507 , 538968063 , 2 , 1 , 22830698 , 65149328 , uint256("0a505d61eb1f16a672efbeb77a6fc151a32281f0bce43a744491573d2bcf0344") , uint256("914be36a1934252cc6e89d7c75f4c69d2aec40c16e670570bb2a906b9ab13388") , 13500000000000 }, // Block 18
{0, 1553431527, 1553431527 , 538968063 , 2 , 1 , 0 , 0 , uint256("0cb8014a7f447cd25e8bc757fe291e16ccc994728ed00c499cb91511d63ea23b") , uint256("9db7a8bce36bbc45c9530c5545809869f2baf600d035eea810d09193cfe7d368") , 14400000000000 }, // Block 19
{0, 1553431547, 1553431547 , 538968063 , 6 , 1 , 17277539 , 32090829 , uint256("01ccdf5b4c7787c7ee75a0eccc8fad58717487a8418b77d68bc6311b78fcbe10") , uint256("217259941931d3aad64f03410cadfbb1f4d7acf019d2baa7dd918c0ecfb18107") , 15300000000000 }, // Block 20
{0, 1553431567, 1553431567 , 538968063 , 2 , 1 , 0 , 0 , uint256("107d8e1967a3aaa3df7e89297fddbe47d0302e28c36ce47ec3fb62bc662e3798") , uint256("fe398043614657aef184d9efb40f74db8aa661bd698e8c1739680519b0a1b8cb") , 16200000000000 }, // Block 21
{0, 1553431587, 1553431587 , 538968063 , 1 , 1 , 11150915 , 12984773 , uint256("0651f80e581655f63d3417b2a5dcac3ce338e9d70ae155a7547e46c20da79629") , uint256("e1f66a80f6051657855d1340a180be7d194681beda9210bdd07707b3b1a6590b") , 17100000000000 }, // Block 22
{0, 1553431607, 1553431607 , 538968063 , 9 , 1 , 0 , 0 , uint256("060cd8bdffe5c309aac63291b98b954de36b606d243d15e28d82f86e4a196407") , uint256("03d58645edeae85b6135d117daeab55db27bb84acebaf81dc51683cc3b9b5c52") , 18000000000000 }, // Block 23
{0, 1553431627, 1553431627 , 538968063 , 3 , 1 , 0 , 0 , uint256("145d11423a04763ad670d41fe65ce36479532ef2027329f3efc7752954d00a3f") , uint256("691372ab4a67fac384d4418194ed1d32cdd7daff94fd035e1ec98c8ef8b4c799") , 18900000000000 }, // Block 24
{0, 1553431647, 1553431647 , 538968063 , 4 , 1 , 1165095 , 42231805 , uint256("0e37eba9ad001a42113014ee70b834b0162417a87bd9a562daedb680fd4dd116") , uint256("2dd8088941f37a5993dd2a2f90d9691f53f9e7df341a7b44ed09554ee16222b2") , 19800000000000 }, // Block 25
{0, 1553431667, 1553431667 , 538968063 , 1 , 1 , 0 , 0 , uint256("0b67fbf670c6b89166d8a56989998185028f980faf168b7522f20ab5bde7685d") , uint256("b4179e6d2d38d6bad58620875d57f7c7240fc4554b4c6c5cc273d9ec612a90b3") , 20700000000000 }, // Block 26
{0, 1553431687, 1553431687 , 538968063 , 9 , 1 , 5986303 , 45133333 , uint256("06648ca38ee38467453bfab5b512641c353be60bf381b11a60fca01bc6289f79") , uint256("6e104a139a739ef27340222bdc5eba1abed8b703e4b6dab7bcff385e216a9292") , 21600000000000 }, // Block 27
{0, 1553431707, 1553431707 , 538968063 , 7 , 1 , 0 , 0 , uint256("18f0a438d36370b91f148783366a0c631eb7ec5b16e2e9a9ece463c993d22d70") , uint256("8665703f83ded2e1a564bda40df6bb34d9bdb6986caa1f0a8c9222f4734c034d") , 22500000000000 }, // Block 28
{0, 1553431727, 1553431727 , 538968063 , 8 , 1 , 0 , 0 , uint256("1c1269fcce54502b1243063a77dc8b9065c5dce448641740cb544eefe2a3f686") , uint256("c9b20561dca28d8f86cde5c8b3e2d8d3dde5673ad5eaee08e9cef4a2b32618d6") , 23400000000000 }, // Block 29
{0, 1553431747, 1553431747 , 538968063 , 6 , 1 , 27440498 , 42505344 , uint256("07f8b76bfec15f22e0a9addc7467a701c7784832ebb711b3e72ff1127e5e6877") , uint256("785cba513559589287ef2a7b8bc3b130071929ee87a9b5d8853f76cfc6eac70f") , 24300000000000 }, // Block 30
};

void LogBlockFound(CWallet* pwallet, int blockNumber, CBlock* pblock, unsigned int nExtraNonce, bool fProofOfStake, bool logToStdout)
{
    std::stringstream str;
    str << "{" << fProofOfStake << ", ";
    str << pblock->nTime << ", ";
    str << pblock->vtx[0].nTime << " , ";
    str << pblock->nBits << " , ";
    str << pblock->nNonce << " , ";
    str << nExtraNonce << " , ";
    str << pblock->nBirthdayA << " , ";
    str << pblock->nBirthdayB << " , ";
    str << "uint256(\"" << pblock->GetHash().ToString().c_str() << "\") , ";
    str << "uint256(\"" << pblock->hashMerkleRoot.ToString().c_str() << "\") , ";
    str << pwallet->GetBalance() << " },";
    str << " // Block " << blockNumber << endl;

    if (logToStdout) {
        cout << pblock->ToString().c_str() << endl;
        cout << str.str();
    }

    if (fDebug) {
        LogPrintf("%s \n", str.str());
        LogPrintf("Block %d %s \n", blockNumber, (pblock->IsProofOfStake() ? " (PoS) " : " (PoW) "));
        LogPrintf(" nTime               : %u \n", pblock->nTime);
        LogPrintf(" hash                : %s \n", pblock->GetHash().ToString().c_str());
        LogPrintf(" StakeModifier       : %u \n", chainActive.Tip()->nStakeModifier);
        LogPrintf(" OldStakeModifier    : %s \n", chainActive.Tip()->nStakeModifierOld.ToString());
        LogPrintf(" Modifier Generated? : %s \n", (chainActive.Tip()->GeneratedStakeModifier() ? "True" : "False"));
        LogPrintf(" Balance             : %d \n", pwallet->GetBalance());
        LogPrintf(" Unconfirmed Balance : %d \n", pwallet->GetUnconfirmedBalance());
        LogPrintf(" Immature  Balance   : %d \n", pwallet->GetImmatureBalance());
        LogPrintf(" ---- \n");
    }
}

void InitializeLastCoinStakeSearchTime(CWallet* pwallet, CScript& scriptPubKey)
{
    const CChainParams& chainparams = Params();

    // this is just to initialize nLastCoinStakeSearchTime
    unique_ptr<CBlockTemplate> pblocktemplate(CreateNewBlock_Legacy(chainparams, scriptPubKey, pwallet, true));
    if (!pblocktemplate.get())
        return;
    CBlock* pblock = &pblocktemplate->block;
    SignBlock_Legacy(pwallet, pblock);
    SetMockTime(GetTime() + 30);
}


CScript GenerateSamePubKeyScript4Wallet( const string & secret, CWallet* pwallet )
{
    CBitcoinSecret bsecret;
    bsecret.SetString(secret);
    CKey key = bsecret.GetKey();
    CPubKey pubKey = key.GetPubKey();
    CKeyID keyID = pubKey.GetID();
    CScript scriptPubKey = GetScriptForDestination(keyID);

    //pwallet->NewKeyPool();
    LOCK(pwallet->cs_wallet);
    pwallet->AddKeyPubKey(key, pubKey);
    pwallet->SetDefaultKey(pubKey);

    if(fDebug) { 
        LogPrintf("pub key used      : %s \n", scriptPubKey.ToString()); 
        LogPrintf("pub key used (hex): %x \n", HexStr(scriptPubKey)); 
    }

    return scriptPubKey;
}


void ScanForWalletTransactions(CWallet* pwallet)
{
    pwallet->nTimeFirstKey = chainActive[0]->nTime;
    // pwallet->fFileBacked = true;
    // CBlockIndex* genesisBlock = chainActive[0];
    // pwallet->ScanForWalletTransactions(genesisBlock, true);
}

void GenerateBlocks(int startBlock, int endBlock, CWallet* pwallet, CScript& scriptPubKey, bool fProofOfStake, bool logToStdout)
{
    bool fGenerateBitcoins = false;
    bool fMintableCoins = false;
    int nMintableLastCheck = 0;
    CReserveKey reservekey(pwallet); // Lico, once we want to use the same pubkey, we dont need to remove it from key pool

    // Each thread has its own key and counter
    unsigned int nExtraNonce = 0;

    int oldnHeight = chainActive.Tip()->nHeight;

    for (int j = startBlock; j < endBlock;) {
        if (j == endBlock - 1)
            LogPrintf("a %s", "b");
        SetMockTime(GetTime() + Params().GetTargetSpacing());
        if (fProofOfStake) {
            //control the amount of times the client will check for mintable coins
            if ((GetTime() - nMintableLastCheck > Params().GetClientMintableCoinsInterval())) {
                nMintableLastCheck = GetTime();
                fMintableCoins = pwallet->MintableCoins();
            }

            while (pwallet->IsLocked() || !fMintableCoins ||
                   (pwallet->GetBalance() > 0 && nReserveBalance >= pwallet->GetBalance())) {
                nLastCoinStakeSearchInterval = 0;
                // Do a separate 1 minute check here to ensure fMintableCoins is updated
                if (!fMintableCoins) {
                    if (GetTime() - nMintableLastCheck > Params().GetEnsureMintableCoinsInterval()) // 1 minute check time
                    {
                        nMintableLastCheck = GetTime();
                        fMintableCoins = pwallet->MintableCoins();
                    }
                }

                SetMockTime(GetTime() + 5000);
                boost::this_thread::interruption_point();

                if (!fGenerateBitcoins && !fProofOfStake) {
                    //cout << "BitcoinMiner Going out of Loop !!!" << endl;
                    continue;
                }
            }

            if (mapHashedBlocks.count(chainActive.Tip()->nHeight)) //search our map of hashed blocks, see if bestblock has been hashed yet
            {
                if (GetTime() - mapHashedBlocks[chainActive.Tip()->nHeight] < max(pwallet->nHashInterval, (unsigned int)1)) // wait half of the nHashDrift with max wait of 3 minutes
                {
                    SetMockTime(GetTime() + 5);
                    //cout << "BitcoinMiner Going out of Loop !!!" << endl;
                    continue;
                }
            }
        }

        //
        // Create new block
        //
        //cout << "KOREMiner: Creating new Block " << endl;
        if (fDebug) {
            LogPrintf("vNodes Empty  ? %s \n", vNodes.empty() ? "true" : "false");
            LogPrintf("Wallet Locked ? %s \n", pwallet->IsLocked() ? "true" : "false");
            LogPrintf("Is there Mintable Coins ? %s \n", fMintableCoins ? "true" : "false");
            LogPrintf("Do we have Balance ? %s \n", pwallet->GetBalance() > 0 ? "true" : "false");
            LogPrintf("Balance is Greater than reserved one ? %s \n", nReserveBalance >= pwallet->GetBalance() ? "true" : "false");
        }
        unsigned int nTransactionsUpdatedLast = mempool.GetTransactionsUpdated();
        CBlockIndex* pindexPrev = chainActive.Tip();
        if (!pindexPrev) {
            continue;
        }


        unique_ptr<CBlockTemplate> pblocktemplate(CreateNewBlock(scriptPubKey, pwallet, fProofOfStake));
        // need to create a new block
        //BOOST_CHECK(pblocktemplate.get());
        if (!pblocktemplate.get())
        {
            SetMockTime(GetTime() + (Params().GetTargetSpacing() * 2));
            continue;
        }
        CBlock* pblock = &pblocktemplate->block;
        IncrementExtraNonce(pblock, pindexPrev, nExtraNonce);
        //Stake miner main
        if (fProofOfStake) {
            //cout << "CPUMiner : proof-of-stake block found " << pblock->GetHash().ToString() << endl;
            if (!SignBlock(*pblock, *pwallet)) {
                //cout << "BitcoinMiner(): Signing new block with UTXO key failed" << endl;
                continue;
            }
            //cout << "CPUMiner : proof-of-stake block was signed " << pblock->GetHash().ToString() << endl;
            BOOST_CHECK(ProcessBlockFound(pblock, *pwallet, reservekey));
            LogBlockFound(pwallet, j, pblock, nExtraNonce, fProofOfStake, logToStdout);
            cout << "Current nHeight: " << chainActive.Tip()->nHeight << endl;
            j++;
            continue;
        }

        //
        // Search
        //
        int64_t nStart = GetTime();
        uint256 hashTarget = uint256().SetCompact(pblock->nBits);
        //cout << "target: " << hashTarget.GetHex() << endl;
        while (true) {
            unsigned int nHashesDone = 0;

            uint256 hash;

            //cout << "nbits : " << pblock->nBits << endl;
            while (true) {
                hash = pblock->GetHash();
                //cout << "pblock.nBirthdayA: " << pblock->nBirthdayA << endl;
                //cout << "pblock.nBirthdayB: " << pblock->nBirthdayB << endl;
                //cout << "hash             : " << hash.ToString() << endl;
                //cout << "hashTarget       : " << hashTarget.ToString() << endl;

                if (hash <= hashTarget) {
                    // Found a solution
                    //cout << "BitcoinMiner:" << endl;
                    //cout << "proof-of-work found  "<< endl;
                    //cout << "hash  : " << hash.GetHex() << endl;
                    //cout << "target: " << hashTarget.GetHex() << endl;
                    BOOST_CHECK(ProcessBlockFound(pblock, *pwallet, reservekey));
                    LogBlockFound(pwallet, j, pblock, nExtraNonce, fProofOfStake, logToStdout);

                    // In regression test mode, stop mining after a block is found. This
                    // allows developers to controllably generate a block on demand.
                    // if (Params().ShouldMineBlocksOnDemand())
                    //    throw boost::thread_interrupted();
                    break;
                }
                pblock->nNonce += 1;
                nHashesDone += 1;
                //cout << "Looking for a solution with nounce " << pblock->nNonce << " hashesDone : " << nHashesDone << endl;
                if ((pblock->nNonce & 0xFF) == 0)
                    break;
            }

            // Check for stop or if block needs to be rebuilt
            boost::this_thread::interruption_point();
            // Regtest mode doesn't require peers
            if (vNodes.empty() && Params().DoesMiningRequiresPeers())
                break;
            if (pblock->nNonce >= 0xffff0000)
                break;
            if (mempool.GetTransactionsUpdated() != nTransactionsUpdatedLast && GetTime() - nStart > 60)
                break;
            if (pindexPrev != chainActive.Tip())
                break;

            // Update nTime every few seconds
            UpdateTime(pblock, pindexPrev, fProofOfStake);
            // Changing pblock->nTime can change work required on testnet:
            hashTarget.SetCompact(pblock->nBits);
        }
        j++;
    }

    // lets check if we have generated the munber of blocks requested
    cout << "old Height :" << oldnHeight << endl;
    cout << "should generate: " << endBlock - startBlock << endl;
    cout << "actual generated: " << chainActive.Tip()->nHeight - oldnHeight << endl;
    BOOST_CHECK(oldnHeight + endBlock - startBlock == chainActive.Tip()->nHeight);
}

void GeneratePOWLegacyBlocks(int startBlock, int endBlock, CWallet* pwallet, CScript& scriptPubKey, bool logToStdout)
{
    const CChainParams& chainparams = Params();
    unsigned int nExtraNonce = 0;

    for (int j = startBlock; j < endBlock; j++) {
        SetMockTime(GetTime() + (Params().GetTargetSpacing() * 2));
        int lastBlock = chainActive.Tip()->nHeight;
        CAmount oldBalance = pwallet->GetBalance() + pwallet->GetImmatureBalance() + pwallet->GetUnconfirmedBalance();
        // Let-s make sure we have the correct spacing
        //MilliSleep(Params().GetTargetSpacing()*1000);
        bool foundBlock = false;
        //
        // Create new block
        //
        CBlockIndex* pindexPrev = chainActive.Tip();

        unique_ptr<CBlockTemplate> pblocktemplate(CreateNewBlock_Legacy(chainparams, scriptPubKey, NULL, false));

        if (!pblocktemplate.get()) {
            //cout << "Error in KoreMiner: Keypool ran out, please call keypoolrefill before restarting the mining thread" << endl;
            return;
        }
        CBlock* pblock = &pblocktemplate->block;
        IncrementExtraNonce_Legacy(pblock, pindexPrev, nExtraNonce);

        LogPrintf("Running KoreMiner with %u transactions in block (%u bytes)\n", pblock->vtx.size(),
            ::GetSerializeSize(*pblock, SER_NETWORK, PROTOCOL_VERSION));

        //
        // Search
        //
        int64_t nStart = GetTime();
        arith_uint256 hashTarget = arith_uint256().SetCompact(pblock->nBits);
        uint256 testHash;

        for (; !foundBlock;) {
            unsigned int nHashesDone = 0;
            unsigned int nNonceFound = (unsigned int)-1;

            for (int i = 0; i < 1; i++) {
                pblock->nNonce = pblock->nNonce + 1;
                testHash = pblock->CalculateBestBirthdayHash();
                nHashesDone++;
                //cout << "proof-of-work found  "<< endl;
                //cout << "testHash  : " << UintToArith256(testHash).ToString() << endl;
                //cout << "target    : " << hashTarget.GetHex() << endl;
                if (fDebug) {
                    LogPrintf("testHash : %s \n", UintToArith256(testHash).ToString());
                    LogPrintf("target   : %s \n", hashTarget.ToString());
                }

                if (UintToArith256(testHash) < hashTarget) {
                    // Found a solution
                    nNonceFound = pblock->nNonce;
                    // Found a solution
                    assert(testHash == pblock->GetHash());
                    foundBlock = true;
                    ProcessBlockFound_Legacy(pblock, chainparams);
                    // We have our data, lets print them
                    LogBlockFound(pwallet, j, pblock, nExtraNonce, false, logToStdout);

                    break;
                }
            }

            // Update nTime every few seconds
            UpdateTime(pblock, pindexPrev);
        }

        // a new block was created
        BOOST_CHECK(chainActive.Tip()->nHeight == lastBlock + 1);
        // lets check if the block was created and if the balance is correct
        CAmount bValue = GetBlockValue(chainActive.Tip()->nHeight);
        // 10% to dev fund, we don't have masternode
        if(fDebug) {
            LogPrintf("Checking balance is %s \n", (pwallet->GetBalance() + pwallet->GetImmatureBalance() + pwallet->GetUnconfirmedBalance() == oldBalance + bValue * 0.9 ? "OK" : "NOK"));
        }
        BOOST_CHECK(pwallet->GetBalance() + pwallet->GetImmatureBalance() + pwallet->GetUnconfirmedBalance() == oldBalance + bValue * 0.9);
    }
}

void GeneratePOSLegacyBlocks(int startBlock, int endBlock, CWallet* pwallet, CScript& scriptPubKey, bool logToStdout)
{
    const CChainParams& chainparams = Params();

    InitializeLastCoinStakeSearchTime(pwallet, scriptPubKey);

    for (int j = startBlock; j < endBlock; ) {
        unique_ptr<CBlockTemplate> pblocktemplate(CreateNewBlock_Legacy(chainparams, scriptPubKey, pwallet, true));
        if (!pblocktemplate.get()) 
            return;
        CBlock* pblock = &pblocktemplate->block;
        if (SignBlock_Legacy(pwallet, pblock)) {
            if (ProcessBlockFound_Legacy(pblock, chainparams)) {
                // we dont have extranounce for pos
                LogBlockFound(pwallet, j, pblock, 0, true, logToStdout);
                // Let's wait to generate the nextBlock
                SetMockTime(GetTime() + Params().GetTargetSpacing());
                j++;
            } else {
                cout << "Error PROCESS BLOCK :" << j << endl;
            }
        } else {
            cout << "Error Signing the block: " << j << endl;
        }
    }
}

void Create_Transaction(CBlock* pblock, const CBlockIndex* pindexPrev, const blockinfo_t blockinfo[], int i)
{
    // This method simulates the transaction creation, similar to IncrementExtraNonce_Legacy
    unsigned int nHeight = pindexPrev->nHeight + 1; // Height first in coinbase required for block.version=2
    CMutableTransaction txCoinbase(pblock->vtx[0]);
    txCoinbase.vin[0].scriptSig = (CScript() << nHeight << CScriptNum(blockinfo[i].extranonce)) + COINBASE_FLAGS;
    assert(txCoinbase.vin[0].scriptSig.size() <= 100);
    // lets update the time in order to simulate the creation
    txCoinbase.nTime = blockinfo[i].transactionTime;
    pblock->vtx[0] = txCoinbase;
    pblock->hashMerkleRoot = BlockMerkleRoot(*pblock);
}

void Create_NewTransaction(CBlock* pblock, const CBlockIndex* pindexPrev, const blockinfo_t blockinfo[], int i)
{
    // This method simulates the transaction creation, similar to IncrementExtraNonce_Legacy
    unsigned int nHeight = pindexPrev->nHeight + 1; // Height first in coinbase required for block.version=2
    CMutableTransaction txCoinbase(pblock->vtx[0]);
    txCoinbase.vin[0].scriptSig = (CScript() << nHeight << CScriptNum(blockinfo[i].extranonce)) + COINBASE_FLAGS;
    assert(txCoinbase.vin[0].scriptSig.size() <= 100);
    // lets update the time in order to simulate the creation
    //txCoinbase.nTime = blockinfo[i].transactionTime;
    pblock->vtx[0] = txCoinbase;
    pblock->hashMerkleRoot = pblock->BuildMerkleTree();
}

void CreateOldBlocksFromBlockInfo(int startBlock, int endBlock, blockinfo_t& blockInfo, CWallet* pwallet, CScript& scriptPubKey, bool fProofOfStake, bool logToStdout)
{
    CBlockTemplate* pblocktemplate;
    const CChainParams& chainparams = Params();

    CBlockIndex* pindexPrev = chainActive.Tip();


    std::vector<CTransaction*> txFirst;
    for (int i = startBlock - 1; i < endBlock - 1; i++) {
        // Simple block creation, nothing special yet:
        BOOST_CHECK(pblocktemplate = CreateNewBlock_Legacy(chainparams, scriptPubKey, pwallet, fProofOfStake));
        CBlock* pblock = &pblocktemplate->block; // pointer for convenience
        pblock->nVersion = CBlockHeader::CURRENT_VERSION;
        pblock->nBits = blockinfo[i].nBits;
        // Lets create the transaction
        if (!fProofOfStake) {
            pblock->nTime = blockinfo[i].nTime;
            Create_Transaction(pblock, pindexPrev, blockinfo, i);
        }
        pblock->nNonce = blockinfo[i].nonce;
        pblock->nBirthdayA = blockinfo[i].nBirthdayA;
        pblock->nBirthdayB = blockinfo[i].nBirthdayB;
        CValidationState state;
        /*
        cout << "Found Block === " << i+1 << " === " << endl;
        cout << "nTime         : " << pblock->nTime << endl;
        cout << "nNonce        : " << pblock->nNonce << endl;
        cout << "extranonce    : " << blockinfo[i].extranonce << endl;
        cout << "nBirthdayA    : " << pblock->nBirthdayA << endl;
        cout << "nBirthdayB    : " << pblock->nBirthdayB << endl;
        cout << "nBits         : " << pblock->nBits << endl;
        cout << "Hash          : " << pblock->GetHash().ToString().c_str() << endl;
        cout << "hashMerkleRoot: " << pblock->hashMerkleRoot.ToString().c_str()  << endl;
        cout << "New Block values" << endl;
        */

        if (fProofOfStake) {
            BOOST_CHECK(SignBlock_Legacy(pwallet, pblock));
            //cout << pblock->ToString() << endl;
            //cout << "scriptPubKey: " << HexStr(scriptPubKey) << endl;
            // the coin selected can be a different one, so the hash will be different
            //BOOST_CHECK(pblock->GetHash() == blockinfo[i].hash);
            //BOOST_CHECK(pblock->hashMerkleRoot == blockinfo[i].hashMerkleRoot);
            BOOST_CHECK(ProcessNewBlock_Legacy(state, chainparams, NULL, pblock, true, NULL));
        } else {
            //cout << pblock->ToString() << endl;
            BOOST_CHECK(scriptPubKey == pblock->vtx[0].vout[0].scriptPubKey);
            //BOOST_CHECK(pblock->GetHash() == blockinfo[i].hash);
            //BOOST_CHECK(pblock->hashMerkleRoot == blockinfo[i].hashMerkleRoot);
            BOOST_CHECK(ProcessNewBlock_Legacy(state, chainparams, NULL, pblock, true, NULL));
        }
        LogBlockFound(pwallet, i + 1, pblock, blockinfo[i].extranonce, fProofOfStake, logToStdout);
        BOOST_CHECK(state.IsValid());
        // we should get the same balance, depends the maturity
        // cout << "Block: " << i+1 << " time ("<< pblock->GetBlockTime() << ") Should have balance: " << blockinfo[i].balance << " Actual Balance: " << pwallet->GetBalance() << endl;
        // BOOST_CHECK(blockinfo[i].balance == pwallet->GetBalance() + pwallet->GetImmatureBalance() + pwallet->GetUnconfirmedBalance());
        // if we have added a new block the chainActive should be correct
        BOOST_CHECK(pindexPrev != chainActive.Tip());
        pblock->hashPrevBlock = pblock->GetHash();
        pindexPrev = chainActive.Tip();
        if (pblocktemplate)
            delete pblocktemplate;
    }
}

void createNewBlocksFromBlockInfo(int startBlock, int endBlock, blockinfo_t& blockInfo, CWallet* pwallet, CScript& scriptPubKey, bool fProofOfStake, bool logToStdout)
{
    CBlockTemplate* pblocktemplate;
    const CChainParams& chainparams = Params();
    CReserveKey reservekey(pwallet); // only for consistency !!!


    CBlockIndex* pindexPrev = chainActive.Tip();

    std::vector<CTransaction*> txFirst;
    for (int i = startBlock - 1; i < endBlock - 1; i++) {
        unique_ptr<CBlockTemplate> pblocktemplate(CreateNewBlock(scriptPubKey, pwallet, fProofOfStake));
        assert(pblocktemplate.get() != NULL);
        CBlock* pblock = &pblocktemplate->block; // pointer for convenience
        pblock->nVersion = CBlockHeader::CURRENT_VERSION;
        //pblock->nTime = blockinfo[i].nTime;
        pblock->nBits = blockinfo[i].nBits;
        // Lets create the transaction
        if (!fProofOfStake)
            Create_NewTransaction(pblock, pindexPrev, blockinfo, i);
        pblock->nNonce = blockinfo[i].nonce;
        pblock->nBirthdayA = blockinfo[i].nBirthdayA;
        pblock->nBirthdayB = blockinfo[i].nBirthdayB;
        CValidationState state;
        //cout << "Found Block === " << i+1 << " === " << endl;
        //cout << "nTime         : " << pblock->nTime << endl;
        //cout << "nNonce        : " << pblock->nNonce << endl;
        //cout << "extranonce    : " << blockinfo[i].extranonce << endl;
        //cout << "nBirthdayA    : " << pblock->nBirthdayA << endl;
        //cout << "nBirthdayB    : " << pblock->nBirthdayB << endl;
        //cout << "nBits         : " << pblock->nBits << endl;
        //cout << "Hash          : " << pblock->GetHash().ToString().c_str() << endl;
        //cout << "hashMerkleRoot: " << pblock->hashMerkleRoot.ToString().c_str()  << endl;
        //cout << "New Block values" << endl;

        if (fProofOfStake) {
            BOOST_CHECK(SignBlock(*pblock, *pwallet));
            //cout << pblock->ToString() << endl;
            BOOST_CHECK(ProcessBlockFound(pblock, *pwallet, reservekey));
        } else {
            //cout << pblock->ToString() << endl;
            //BOOST_CHECK(scriptPubKey == pblock->vtx[0].vout[0].scriptPubKey);
            // previous block hash is not the same
            //BOOST_CHECK(pblock->GetHash() == blockinfo[i].hash);
            //BOOST_CHECK(pblock->hashMerkleRoot == blockinfo[i].hashMerkleRoot);
            BOOST_CHECK(ProcessBlockFound(pblock, *pwallet, reservekey));
        }

        BOOST_CHECK(state.IsValid());
        // we should get the same balance
        cout << "Block: " << i + 1 << " time (" << pblock->GetBlockTime() << ") Should have balance: " << blockinfo[i].balance << " Actual Balance: " << pwallet->GetBalance() << endl;
        BOOST_CHECK(blockinfo[i].balance == pwallet->GetBalance());
        // if we have added a new block the chainActive should be correct
        BOOST_CHECK(pindexPrev != chainActive.Tip());
        pblock->hashPrevBlock = pblock->GetHash();
        pindexPrev = chainActive.Tip();
    }
}
