

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
    hash=090d45d6ab57de89def94eb6f70cc11d7eb3c09b1240da9a4248ab56ef08cc43 
    ver=1 
    hashPrevBlock=0aab10677b4fe0371a67f99e78a69e7d9fa03a1c7d48747978da405dc5abeb99, 
    hashMerkleRoot=b8791578d87ad4a5751ff4be7c5328c357e45ce85ddeb0eaeb6061a8f4033d87, 
    nTime=1554930863, 
    nBits=201fffff, 
    nNonce=27, 
    nBirthdayA=40667891, 
    nBirthdayB=49644332, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=b8791578d8, ver=1,  nTime=1554930863, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 510101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    b8791578d87ad4a5751ff4be7c5328c357e45ce85ddeb0eaeb6061a8f4033d87
CBlock <<<<============================

{0, 1554930863, 1554930863 , 538968063 , 27 , 1 , 40667891 , 49644332 , uint256("090d45d6ab57de89def94eb6f70cc11d7eb3c09b1240da9a4248ab56ef08cc43") , uint256("b8791578d87ad4a5751ff4be7c5328c357e45ce85ddeb0eaeb6061a8f4033d87") , 0 }, // Block 1
CBlock ============================>>>>
    hash=07670f398c28e87d2288441ce0fa89f6ed3c6487897c362faa62eccdcd5aff4b 
    ver=1 
    hashPrevBlock=090d45d6ab57de89def94eb6f70cc11d7eb3c09b1240da9a4248ab56ef08cc43, 
    hashMerkleRoot=3a514bbafe1ae9a0b9bf20011e2d3badc382b7937b1ad7d6c7f12826da6ecc81, 
    nTime=1554930883, 
    nBits=201fffff, 
    nNonce=4, 
    nBirthdayA=11556780, 
    nBirthdayB=35585619, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=3a514bbafe, ver=1,  nTime=1554930883, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 520101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    3a514bbafe1ae9a0b9bf20011e2d3badc382b7937b1ad7d6c7f12826da6ecc81
CBlock <<<<============================

{0, 1554930883, 1554930883 , 538968063 , 4 , 1 , 11556780 , 35585619 , uint256("07670f398c28e87d2288441ce0fa89f6ed3c6487897c362faa62eccdcd5aff4b") , uint256("3a514bbafe1ae9a0b9bf20011e2d3badc382b7937b1ad7d6c7f12826da6ecc81") , 0 }, // Block 2
CBlock ============================>>>>
    hash=0875439740e31181356fec6531b95e69b8bb5a262f112b031cc8e532b0c55dfa 
    ver=1 
    hashPrevBlock=07670f398c28e87d2288441ce0fa89f6ed3c6487897c362faa62eccdcd5aff4b, 
    hashMerkleRoot=ad020cc2a6f6328d00e466485f8c3ff9092b8450beb255c9e838fbf37bfe2eee, 
    nTime=1554930903, 
    nBits=201fffff, 
    nNonce=15, 
    nBirthdayA=62672505, 
    nBirthdayB=65157209, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=ad020cc2a6, ver=1,  nTime=1554930903, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 530101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    ad020cc2a6f6328d00e466485f8c3ff9092b8450beb255c9e838fbf37bfe2eee
CBlock <<<<============================

{0, 1554930903, 1554930903 , 538968063 , 15 , 1 , 62672505 , 65157209 , uint256("0875439740e31181356fec6531b95e69b8bb5a262f112b031cc8e532b0c55dfa") , uint256("ad020cc2a6f6328d00e466485f8c3ff9092b8450beb255c9e838fbf37bfe2eee") , 0 }, // Block 3
CBlock ============================>>>>
    hash=17fe1737153d983ee0dbf99ddebf0b7d2bb34dd86e7975366a013044f2542e29 
    ver=1 
    hashPrevBlock=0875439740e31181356fec6531b95e69b8bb5a262f112b031cc8e532b0c55dfa, 
    hashMerkleRoot=c058b7d2e37daf00ba0d254eb8aa6ddc4040f12fe8c45c04f66ff4f6ee7b7df5, 
    nTime=1554930923, 
    nBits=201fffff, 
    nNonce=1, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=c058b7d2e3, ver=1,  nTime=1554930923, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 540101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    c058b7d2e37daf00ba0d254eb8aa6ddc4040f12fe8c45c04f66ff4f6ee7b7df5
CBlock <<<<============================

{0, 1554930923, 1554930923 , 538968063 , 1 , 1 , 0 , 0 , uint256("17fe1737153d983ee0dbf99ddebf0b7d2bb34dd86e7975366a013044f2542e29") , uint256("c058b7d2e37daf00ba0d254eb8aa6ddc4040f12fe8c45c04f66ff4f6ee7b7df5") , 3600000000000 }, // Block 4
CBlock ============================>>>>
    hash=0e9bfe641fa71fbfed60f0162b5e8fa1a4e93fdca64a72d1d11ce133907a480b 
    ver=1 
    hashPrevBlock=17fe1737153d983ee0dbf99ddebf0b7d2bb34dd86e7975366a013044f2542e29, 
    hashMerkleRoot=e94d4208c8506c5f8e873171e7e66db5b99c7e3efbb055d9ea64a6d2ca4a9335, 
    nTime=1554930943, 
    nBits=201fffff, 
    nNonce=11, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=e94d4208c8, ver=1,  nTime=1554930943, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 550101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    e94d4208c8506c5f8e873171e7e66db5b99c7e3efbb055d9ea64a6d2ca4a9335
CBlock <<<<============================

{0, 1554930943, 1554930943 , 538968063 , 11 , 1 , 0 , 0 , uint256("0e9bfe641fa71fbfed60f0162b5e8fa1a4e93fdca64a72d1d11ce133907a480b") , uint256("e94d4208c8506c5f8e873171e7e66db5b99c7e3efbb055d9ea64a6d2ca4a9335") , 7200000000000 }, // Block 5
CBlock ============================>>>>
    hash=09a31e3b4f42f0099c7bc3c205ca7006295c5e15aa5e6a63b3e8268ec7f58d6f 
    ver=1 
    hashPrevBlock=0e9bfe641fa71fbfed60f0162b5e8fa1a4e93fdca64a72d1d11ce133907a480b, 
    hashMerkleRoot=1cb50f76afa7a852bf31dea5e57d9b9dc1117abf348201bd254d782e76801334, 
    nTime=1554930963, 
    nBits=201fffff, 
    nNonce=1, 
    nBirthdayA=18912201, 
    nBirthdayB=28192639, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=1cb50f76af, ver=1,  nTime=1554930963, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 560101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    1cb50f76afa7a852bf31dea5e57d9b9dc1117abf348201bd254d782e76801334
CBlock <<<<============================

{0, 1554930963, 1554930963 , 538968063 , 1 , 1 , 18912201 , 28192639 , uint256("09a31e3b4f42f0099c7bc3c205ca7006295c5e15aa5e6a63b3e8268ec7f58d6f") , uint256("1cb50f76afa7a852bf31dea5e57d9b9dc1117abf348201bd254d782e76801334") , 10800000000000 }, // Block 6
CBlock ============================>>>>
    hash=1f8b2176c585619d57b57db8b27ee0932b4718813c40544627ba8ad7a09979ca 
    ver=1 
    hashPrevBlock=09a31e3b4f42f0099c7bc3c205ca7006295c5e15aa5e6a63b3e8268ec7f58d6f, 
    hashMerkleRoot=acbf8dc76e185a1fb9cc9d281268ad0db00df7a41d2abc13f6437a9df8f1f4ce, 
    nTime=1554930983, 
    nBits=201fffff, 
    nNonce=1, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=acbf8dc76e, ver=1,  nTime=1554930983, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 570101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    acbf8dc76e185a1fb9cc9d281268ad0db00df7a41d2abc13f6437a9df8f1f4ce
CBlock <<<<============================

{0, 1554930983, 1554930983 , 538968063 , 1 , 1 , 0 , 0 , uint256("1f8b2176c585619d57b57db8b27ee0932b4718813c40544627ba8ad7a09979ca") , uint256("acbf8dc76e185a1fb9cc9d281268ad0db00df7a41d2abc13f6437a9df8f1f4ce") , 14400000000000 }, // Block 7
CBlock ============================>>>>
    hash=16c089dfbdc8a8e307168334b8b0e2a99755cd9a53d7a06844f64166e0000268 
    ver=1 
    hashPrevBlock=1f8b2176c585619d57b57db8b27ee0932b4718813c40544627ba8ad7a09979ca, 
    hashMerkleRoot=af9035035ec116665fc2bb31af7936a6d0631bc75dd702d63a9c3f020ff1c6d3, 
    nTime=1554931003, 
    nBits=201fffff, 
    nNonce=8, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=af9035035e, ver=1,  nTime=1554931003, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 580101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    af9035035ec116665fc2bb31af7936a6d0631bc75dd702d63a9c3f020ff1c6d3
CBlock <<<<============================

{0, 1554931003, 1554931003 , 538968063 , 8 , 1 , 0 , 0 , uint256("16c089dfbdc8a8e307168334b8b0e2a99755cd9a53d7a06844f64166e0000268") , uint256("af9035035ec116665fc2bb31af7936a6d0631bc75dd702d63a9c3f020ff1c6d3") , 18000000000000 }, // Block 8
CBlock ============================>>>>
    hash=068c81958508a14bb5efd023d9d3600003fcdade0efcbf48107ed3000f0abf6c 
    ver=1 
    hashPrevBlock=16c089dfbdc8a8e307168334b8b0e2a99755cd9a53d7a06844f64166e0000268, 
    hashMerkleRoot=6a0194e17876b968caba4c12520dfdd00305ae820f72d413a465c9377f52ac4a, 
    nTime=1554931023, 
    nBits=201fffff, 
    nNonce=12, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=6a0194e178, ver=1,  nTime=1554931023, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 590101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    6a0194e17876b968caba4c12520dfdd00305ae820f72d413a465c9377f52ac4a
CBlock <<<<============================

{0, 1554931023, 1554931023 , 538968063 , 12 , 1 , 0 , 0 , uint256("068c81958508a14bb5efd023d9d3600003fcdade0efcbf48107ed3000f0abf6c") , uint256("6a0194e17876b968caba4c12520dfdd00305ae820f72d413a465c9377f52ac4a") , 21600000000000 }, // Block 9
CBlock ============================>>>>
    hash=06df31ed99f6aac5e75810d9d95468568867ee3e9d0e00dad9e5080c3f2e433e 
    ver=1 
    hashPrevBlock=068c81958508a14bb5efd023d9d3600003fcdade0efcbf48107ed3000f0abf6c, 
    hashMerkleRoot=4141f214f959e2068fbff02e8c66ff6446792a7f228cbe4950131bcf26454f98, 
    nTime=1554931043, 
    nBits=201fffff, 
    nNonce=7, 
    nBirthdayA=1806273, 
    nBirthdayB=37938425, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=4141f214f9, ver=1,  nTime=1554931043, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 5a0101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    4141f214f959e2068fbff02e8c66ff6446792a7f228cbe4950131bcf26454f98
CBlock <<<<============================

{0, 1554931043, 1554931043 , 538968063 , 7 , 1 , 1806273 , 37938425 , uint256("06df31ed99f6aac5e75810d9d95468568867ee3e9d0e00dad9e5080c3f2e433e") , uint256("4141f214f959e2068fbff02e8c66ff6446792a7f228cbe4950131bcf26454f98") , 25200000000000 }, // Block 10
CBlock ============================>>>>
    hash=0cf67647fadea04625ecb1c40508503f567c6274a6215c7727263fb796a11ece 
    ver=1 
    hashPrevBlock=06df31ed99f6aac5e75810d9d95468568867ee3e9d0e00dad9e5080c3f2e433e, 
    hashMerkleRoot=47b3714365db8997b0d754ce936c02c28517fdd49f6e17d1022c58ceccbf0c8e, 
    nTime=1554931063, 
    nBits=201fffff, 
    nNonce=4, 
    nBirthdayA=9465122, 
    nBirthdayB=19688108, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=47b3714365, ver=1,  nTime=1554931063, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 5b0101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    47b3714365db8997b0d754ce936c02c28517fdd49f6e17d1022c58ceccbf0c8e
CBlock <<<<============================

{0, 1554931063, 1554931063 , 538968063 , 4 , 1 , 9465122 , 19688108 , uint256("0cf67647fadea04625ecb1c40508503f567c6274a6215c7727263fb796a11ece") , uint256("47b3714365db8997b0d754ce936c02c28517fdd49f6e17d1022c58ceccbf0c8e") , 28800000000000 }, // Block 11
CBlock ============================>>>>
    hash=138f488120c52c51007fc4eefadd8aaeaaf310c87b3a39c5984bfadb38cb65e0 
    ver=1 
    hashPrevBlock=0cf67647fadea04625ecb1c40508503f567c6274a6215c7727263fb796a11ece, 
    hashMerkleRoot=1e346ae58583da7ac6b58185b2c3a61bbf120486b4815ad35a386718d84253ee, 
    nTime=1554931083, 
    nBits=201fffff, 
    nNonce=1, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=1e346ae585, ver=1,  nTime=1554931083, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 5c0101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    1e346ae58583da7ac6b58185b2c3a61bbf120486b4815ad35a386718d84253ee
CBlock <<<<============================

{0, 1554931083, 1554931083 , 538968063 , 1 , 1 , 0 , 0 , uint256("138f488120c52c51007fc4eefadd8aaeaaf310c87b3a39c5984bfadb38cb65e0") , uint256("1e346ae58583da7ac6b58185b2c3a61bbf120486b4815ad35a386718d84253ee") , 32400000000000 }, // Block 12
CBlock ============================>>>>
    hash=0386365ee9d78a45fff02f8d7c49a4950f266e770da439a600967ad36fdcb37d 
    ver=1 
    hashPrevBlock=138f488120c52c51007fc4eefadd8aaeaaf310c87b3a39c5984bfadb38cb65e0, 
    hashMerkleRoot=6e2b8d50e4fb32085a22e5f0f467b4768979936d57ed478f4f28daa263f58651, 
    nTime=1554931103, 
    nBits=201fffff, 
    nNonce=1, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=6e2b8d50e4, ver=1,  nTime=1554931103, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 5d0101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    6e2b8d50e4fb32085a22e5f0f467b4768979936d57ed478f4f28daa263f58651
CBlock <<<<============================

{0, 1554931103, 1554931103 , 538968063 , 1 , 1 , 0 , 0 , uint256("0386365ee9d78a45fff02f8d7c49a4950f266e770da439a600967ad36fdcb37d") , uint256("6e2b8d50e4fb32085a22e5f0f467b4768979936d57ed478f4f28daa263f58651") , 36000000000000 }, // Block 13
CBlock ============================>>>>
    hash=09d84999f5353bd2e464cb83a57ad748fb7291fc8818d16548ac94786fe7af68 
    ver=1 
    hashPrevBlock=0386365ee9d78a45fff02f8d7c49a4950f266e770da439a600967ad36fdcb37d, 
    hashMerkleRoot=9fd52da101686ec13f3b91ff43161c8a3d2690b88893b42b2026f9f21728de46, 
    nTime=1554931123, 
    nBits=201fffff, 
    nNonce=1, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=9fd52da101, ver=1,  nTime=1554931123, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 5e0101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    9fd52da101686ec13f3b91ff43161c8a3d2690b88893b42b2026f9f21728de46
CBlock <<<<============================

{0, 1554931123, 1554931123 , 538968063 , 1 , 1 , 0 , 0 , uint256("09d84999f5353bd2e464cb83a57ad748fb7291fc8818d16548ac94786fe7af68") , uint256("9fd52da101686ec13f3b91ff43161c8a3d2690b88893b42b2026f9f21728de46") , 39600000000000 }, // Block 14
CBlock ============================>>>>
    hash=03fe846719cfa5e0d26ac435efeb7776d1070a7fdba8776f569c7d7c1c5b8235 
    ver=1 
    hashPrevBlock=09d84999f5353bd2e464cb83a57ad748fb7291fc8818d16548ac94786fe7af68, 
    hashMerkleRoot=a9efd61e15a80d847cd798a1ef7d31354b1b7c583a9c717df33e8de2618ccbab, 
    nTime=1554931143, 
    nBits=201fffff, 
    nNonce=6, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=a9efd61e15, ver=1,  nTime=1554931143, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 5f0101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    a9efd61e15a80d847cd798a1ef7d31354b1b7c583a9c717df33e8de2618ccbab
CBlock <<<<============================

{0, 1554931143, 1554931143 , 538968063 , 6 , 1 , 0 , 0 , uint256("03fe846719cfa5e0d26ac435efeb7776d1070a7fdba8776f569c7d7c1c5b8235") , uint256("a9efd61e15a80d847cd798a1ef7d31354b1b7c583a9c717df33e8de2618ccbab") , 43200000000000 }, // Block 15
CBlock ============================>>>>
    hash=02c64c51029d9c62b783e7be9642f6d74259de87d6c2128a33f506153a63db56 
    ver=1 
    hashPrevBlock=03fe846719cfa5e0d26ac435efeb7776d1070a7fdba8776f569c7d7c1c5b8235, 
    hashMerkleRoot=1d8ed63408c0f4adf6a554d4534354abe150ea3af73fab91b1cb277eeb1b3dc4, 
    nTime=1554931163, 
    nBits=201fffff, 
    nNonce=3, 
    nBirthdayA=3172386, 
    nBirthdayB=38682123, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=1d8ed63408, ver=1,  nTime=1554931163, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 600101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    1d8ed63408c0f4adf6a554d4534354abe150ea3af73fab91b1cb277eeb1b3dc4
CBlock <<<<============================

{0, 1554931163, 1554931163 , 538968063 , 3 , 1 , 3172386 , 38682123 , uint256("02c64c51029d9c62b783e7be9642f6d74259de87d6c2128a33f506153a63db56") , uint256("1d8ed63408c0f4adf6a554d4534354abe150ea3af73fab91b1cb277eeb1b3dc4") , 46800000000000 }, // Block 16
CBlock ============================>>>>
    hash=1caa5b60e929c60e88317bbda1e7eedbcf1823cd71aeadec3570ae59dcb82b35 
    ver=1 
    hashPrevBlock=02c64c51029d9c62b783e7be9642f6d74259de87d6c2128a33f506153a63db56, 
    hashMerkleRoot=1dee3c0be2b5b545751cc1f92f181939b26100be28480086fcd6cb6a4fdb4964, 
    nTime=1554931183, 
    nBits=201fffff, 
    nNonce=2, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=1dee3c0be2, ver=1,  nTime=1554931183, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 01110101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    1dee3c0be2b5b545751cc1f92f181939b26100be28480086fcd6cb6a4fdb4964
CBlock <<<<============================

{0, 1554931183, 1554931183 , 538968063 , 2 , 1 , 0 , 0 , uint256("1caa5b60e929c60e88317bbda1e7eedbcf1823cd71aeadec3570ae59dcb82b35") , uint256("1dee3c0be2b5b545751cc1f92f181939b26100be28480086fcd6cb6a4fdb4964") , 50400000000000 }, // Block 17
CBlock ============================>>>>
    hash=1048926b872d3375ad94a5b4e9b5f2474f578735e81194afd827c63dd30926dd 
    ver=1 
    hashPrevBlock=1caa5b60e929c60e88317bbda1e7eedbcf1823cd71aeadec3570ae59dcb82b35, 
    hashMerkleRoot=919111d9b47be44c99047fcbca99ab7b50abfbd8bfc671c0d7eb6c6679852658, 
    nTime=1554931203, 
    nBits=201fffff, 
    nNonce=4, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=919111d9b4, ver=1,  nTime=1554931203, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 01120101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    919111d9b47be44c99047fcbca99ab7b50abfbd8bfc671c0d7eb6c6679852658
CBlock <<<<============================

{0, 1554931203, 1554931203 , 538968063 , 4 , 1 , 0 , 0 , uint256("1048926b872d3375ad94a5b4e9b5f2474f578735e81194afd827c63dd30926dd") , uint256("919111d9b47be44c99047fcbca99ab7b50abfbd8bfc671c0d7eb6c6679852658") , 54000000000000 }, // Block 18
CBlock ============================>>>>
    hash=1eeec9a43e6047a688cc6049fa4b9af5f408e614331a2499bce359daecb0828c 
    ver=1 
    hashPrevBlock=1048926b872d3375ad94a5b4e9b5f2474f578735e81194afd827c63dd30926dd, 
    hashMerkleRoot=788f4a374cdc08b6e0efdc612796ce4dba7e632d3899d9e0cfc0a153540c0185, 
    nTime=1554931223, 
    nBits=201fffff, 
    nNonce=1, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=788f4a374c, ver=1,  nTime=1554931223, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 01130101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    788f4a374cdc08b6e0efdc612796ce4dba7e632d3899d9e0cfc0a153540c0185
CBlock <<<<============================

{0, 1554931223, 1554931223 , 538968063 , 1 , 1 , 0 , 0 , uint256("1eeec9a43e6047a688cc6049fa4b9af5f408e614331a2499bce359daecb0828c") , uint256("788f4a374cdc08b6e0efdc612796ce4dba7e632d3899d9e0cfc0a153540c0185") , 57600000000000 }, // Block 19
CBlock ============================>>>>
    hash=087fd470d6597236d9d599e09b525f9d0f2ea3b59b84798c67f0dc38d47ff7a4 
    ver=1 
    hashPrevBlock=1eeec9a43e6047a688cc6049fa4b9af5f408e614331a2499bce359daecb0828c, 
    hashMerkleRoot=4f013e148be3bb077eb79624c57cc766f88170b789f93b705e221f4ceb8582ca, 
    nTime=1554931243, 
    nBits=201fffff, 
    nNonce=1, 
    nBirthdayA=46093529, 
    nBirthdayB=57109312, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=4f013e148b, ver=1,  nTime=1554931243, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 01140101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    4f013e148be3bb077eb79624c57cc766f88170b789f93b705e221f4ceb8582ca
CBlock <<<<============================

{0, 1554931243, 1554931243 , 538968063 , 1 , 1 , 46093529 , 57109312 , uint256("087fd470d6597236d9d599e09b525f9d0f2ea3b59b84798c67f0dc38d47ff7a4") , uint256("4f013e148be3bb077eb79624c57cc766f88170b789f93b705e221f4ceb8582ca") , 61200000000000 }, // Block 20
CBlock ============================>>>>
    hash=102dfbd725242726f0024f57d8ca989865b7d921793abd7ceb581d38f621e03a 
    ver=1 
    hashPrevBlock=087fd470d6597236d9d599e09b525f9d0f2ea3b59b84798c67f0dc38d47ff7a4, 
    hashMerkleRoot=db4b40bbbb2b5e4b73300619c5b000f7fcaae6e7b98550703383a7e9480f49ae, 
    nTime=1554931263, 
    nBits=201fffff, 
    nNonce=6, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=db4b40bbbb, ver=1,  nTime=1554931263, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 01150101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    db4b40bbbb2b5e4b73300619c5b000f7fcaae6e7b98550703383a7e9480f49ae
CBlock <<<<============================

{0, 1554931263, 1554931263 , 538968063 , 6 , 1 , 0 , 0 , uint256("102dfbd725242726f0024f57d8ca989865b7d921793abd7ceb581d38f621e03a") , uint256("db4b40bbbb2b5e4b73300619c5b000f7fcaae6e7b98550703383a7e9480f49ae") , 64800000000000 }, // Block 21
CBlock ============================>>>>
    hash=1f5d3e832bdcc85ea30ea265991dd70199201e87dbf7719708bdb738d1b38038 
    ver=1 
    hashPrevBlock=102dfbd725242726f0024f57d8ca989865b7d921793abd7ceb581d38f621e03a, 
    hashMerkleRoot=fb39804fb0da94d0dba64259b1266cbb41d60f5be402cd7cb3a27833a0004299, 
    nTime=1554931283, 
    nBits=201fffff, 
    nNonce=8, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=fb39804fb0, ver=1,  nTime=1554931283, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 01160101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    fb39804fb0da94d0dba64259b1266cbb41d60f5be402cd7cb3a27833a0004299
CBlock <<<<============================

{0, 1554931283, 1554931283 , 538968063 , 8 , 1 , 0 , 0 , uint256("1f5d3e832bdcc85ea30ea265991dd70199201e87dbf7719708bdb738d1b38038") , uint256("fb39804fb0da94d0dba64259b1266cbb41d60f5be402cd7cb3a27833a0004299") , 68400000000000 }, // Block 22
CBlock ============================>>>>
    hash=03d7592f2947bd5f644047bed0b6d411077c76a6afecbdafae07958796eb0ed8 
    ver=1 
    hashPrevBlock=1f5d3e832bdcc85ea30ea265991dd70199201e87dbf7719708bdb738d1b38038, 
    hashMerkleRoot=63344cf464647becb45f20e231094097f2ba6c5a303220d20cddaa28907f48cf, 
    nTime=1554931303, 
    nBits=201fffff, 
    nNonce=7, 
    nBirthdayA=12925467, 
    nBirthdayB=64877471, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=63344cf464, ver=1,  nTime=1554931303, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 01170101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    63344cf464647becb45f20e231094097f2ba6c5a303220d20cddaa28907f48cf
CBlock <<<<============================

{0, 1554931303, 1554931303 , 538968063 , 7 , 1 , 12925467 , 64877471 , uint256("03d7592f2947bd5f644047bed0b6d411077c76a6afecbdafae07958796eb0ed8") , uint256("63344cf464647becb45f20e231094097f2ba6c5a303220d20cddaa28907f48cf") , 72000000000000 }, // Block 23
CBlock ============================>>>>
    hash=048a18fa80afa01437a80c2a2d7168ed6f4579fca2f5a9fb1eeeb8ea37c1b1a3 
    ver=1 
    hashPrevBlock=03d7592f2947bd5f644047bed0b6d411077c76a6afecbdafae07958796eb0ed8, 
    hashMerkleRoot=d9afd0ad9166beba3aeca7f1e2327b3de89009953f56d1465423fcaccdee10f6, 
    nTime=1554931323, 
    nBits=201fffff, 
    nNonce=1, 
    nBirthdayA=26305447, 
    nBirthdayB=65906031, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=d9afd0ad91, ver=1,  nTime=1554931323, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 01180101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    d9afd0ad9166beba3aeca7f1e2327b3de89009953f56d1465423fcaccdee10f6
CBlock <<<<============================

{0, 1554931323, 1554931323 , 538968063 , 1 , 1 , 26305447 , 65906031 , uint256("048a18fa80afa01437a80c2a2d7168ed6f4579fca2f5a9fb1eeeb8ea37c1b1a3") , uint256("d9afd0ad9166beba3aeca7f1e2327b3de89009953f56d1465423fcaccdee10f6") , 75600000000000 }, // Block 24
CBlock ============================>>>>
    hash=1fe6ea0a6508faf22ee242163db0da91475a0451bf6529dbd32fa4ab65e95fe4 
    ver=1 
    hashPrevBlock=048a18fa80afa01437a80c2a2d7168ed6f4579fca2f5a9fb1eeeb8ea37c1b1a3, 
    hashMerkleRoot=236ca3963e957ff5e603de54bfa239231d15a4e0e4e7d9e098eaf06943e00517, 
    nTime=1554931343, 
    nBits=201fffff, 
    nNonce=1, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=236ca3963e, ver=1,  nTime=1554931343, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 01190101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    236ca3963e957ff5e603de54bfa239231d15a4e0e4e7d9e098eaf06943e00517
CBlock <<<<============================

{0, 1554931343, 1554931343 , 538968063 , 1 , 1 , 0 , 0 , uint256("1fe6ea0a6508faf22ee242163db0da91475a0451bf6529dbd32fa4ab65e95fe4") , uint256("236ca3963e957ff5e603de54bfa239231d15a4e0e4e7d9e098eaf06943e00517") , 79200000000000 }, // Block 25
CBlock ============================>>>>
    hash=14f3b31ac363988d61a72b93a27f7cdacb7eda5661969368fb2cf4114593866e 
    ver=1 
    hashPrevBlock=1fe6ea0a6508faf22ee242163db0da91475a0451bf6529dbd32fa4ab65e95fe4, 
    hashMerkleRoot=59addd74c10d18748fbdd9518dd855c4f4385c66879b090c3fe858c189639f3b, 
    nTime=1554931363, 
    nBits=201fffff, 
    nNonce=2, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=59addd74c1, ver=1,  nTime=1554931363, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 011a0101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    59addd74c10d18748fbdd9518dd855c4f4385c66879b090c3fe858c189639f3b
CBlock <<<<============================

{0, 1554931363, 1554931363 , 538968063 , 2 , 1 , 0 , 0 , uint256("14f3b31ac363988d61a72b93a27f7cdacb7eda5661969368fb2cf4114593866e") , uint256("59addd74c10d18748fbdd9518dd855c4f4385c66879b090c3fe858c189639f3b") , 82800000000000 }, // Block 26
CBlock ============================>>>>
    hash=1602ba0f7314f2c0594fe2cd5b55f69ea43211399875460bd49a3cfe72a9f49e 
    ver=1 
    hashPrevBlock=14f3b31ac363988d61a72b93a27f7cdacb7eda5661969368fb2cf4114593866e, 
    hashMerkleRoot=c320e08bac91351bbbe510b71570a286006caa231d994796e182828963383a77, 
    nTime=1554931383, 
    nBits=201fffff, 
    nNonce=1, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=c320e08bac, ver=1,  nTime=1554931383, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 011b0101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    c320e08bac91351bbbe510b71570a286006caa231d994796e182828963383a77
CBlock <<<<============================

{0, 1554931383, 1554931383 , 538968063 , 1 , 1 , 0 , 0 , uint256("1602ba0f7314f2c0594fe2cd5b55f69ea43211399875460bd49a3cfe72a9f49e") , uint256("c320e08bac91351bbbe510b71570a286006caa231d994796e182828963383a77") , 86400000000000 }, // Block 27
CBlock ============================>>>>
    hash=091c146c540b616f8c2067557bf30b1234118ccd49680c700807b256adcd7f52 
    ver=1 
    hashPrevBlock=1602ba0f7314f2c0594fe2cd5b55f69ea43211399875460bd49a3cfe72a9f49e, 
    hashMerkleRoot=b68b576e4b24c79338acc961c8a7094ab0b7eec5f78e1a09cbbd44dc8cf39788, 
    nTime=1554931403, 
    nBits=201fffff, 
    nNonce=5, 
    nBirthdayA=37714313, 
    nBirthdayB=52064346, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=b68b576e4b, ver=1,  nTime=1554931403, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 011c0101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    b68b576e4b24c79338acc961c8a7094ab0b7eec5f78e1a09cbbd44dc8cf39788
CBlock <<<<============================

{0, 1554931403, 1554931403 , 538968063 , 5 , 1 , 37714313 , 52064346 , uint256("091c146c540b616f8c2067557bf30b1234118ccd49680c700807b256adcd7f52") , uint256("b68b576e4b24c79338acc961c8a7094ab0b7eec5f78e1a09cbbd44dc8cf39788") , 90000000000000 }, // Block 28
CBlock ============================>>>>
    hash=0a12403635b80142391e1c96cdfc025c53df0d1b2baad22f615e7aa5ca653223 
    ver=1 
    hashPrevBlock=091c146c540b616f8c2067557bf30b1234118ccd49680c700807b256adcd7f52, 
    hashMerkleRoot=aad9c974092c7efdfab046f2c1d83db1a7a8fd886e04f8932ca67e56ad68bb5b, 
    nTime=1554931423, 
    nBits=201fffff, 
    nNonce=2, 
    nBirthdayA=0, 
    nBirthdayB=0, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=aad9c97409, ver=1,  nTime=1554931423, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 011d0101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    aad9c974092c7efdfab046f2c1d83db1a7a8fd886e04f8932ca67e56ad68bb5b
CBlock <<<<============================

{0, 1554931423, 1554931423 , 538968063 , 2 , 1 , 0 , 0 , uint256("0a12403635b80142391e1c96cdfc025c53df0d1b2baad22f615e7aa5ca653223") , uint256("aad9c974092c7efdfab046f2c1d83db1a7a8fd886e04f8932ca67e56ad68bb5b") , 93600000000000 }, // Block 29
CBlock ============================>>>>
    hash=09475c3056fd7832c221097db0783b03c3483eaaf4e4c91076e18ec99bab9e6d 
    ver=1 
    hashPrevBlock=0a12403635b80142391e1c96cdfc025c53df0d1b2baad22f615e7aa5ca653223, 
    hashMerkleRoot=329b94a34c1a489daa9c1796871ef576f6c4772020cd1a52b3788bb14ce5b45c, 
    nTime=1554931443, 
    nBits=201fffff, 
    nNonce=2, 
    nBirthdayA=64717794, 
    nBirthdayB=65606977, 
    vchBlockSig=, 
    Vtx : size 1 
    CTransaction(hash=329b94a34c, ver=1,  nTime=1554931443, vin.size=1, vout.size=3, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 011e0101)
    CTxOut(nValue=3600000000000, scriptPubKey=76a914ff197b14e502ab41f3bc8ccb48c4abac9eab35bc88ac)
    CTxOut(nValue=400000000000, scriptPubKey=4104d410c4a7fec6dbf6fedc9721104ada1571d5e3e4791085efc083a9f3f4c007d240a6a647dda0ca1466641b0739a86a67b97ac48484fc7ca88257804b7ce52ed2ac)
    CTxOut(nValue=0, scriptPubKey=1e43726561746564206f6e2076657273696f6e203133207072652d666f726b6a)

  vMerkleTree:     vMerkleTree : size 1 
    329b94a34c1a489daa9c1796871ef576f6c4772020cd1a52b3788bb14ce5b45c
CBlock <<<<============================

{0, 1554931443, 1554931443 , 538968063 , 2 , 1 , 64717794 , 65606977 , uint256("09475c3056fd7832c221097db0783b03c3483eaaf4e4c91076e18ec99bab9e6d") , uint256("329b94a34c1a489daa9c1796871ef576f6c4772020cd1a52b3788bb14ce5b45c") , 97200000000000 }, // Block 30

*/

blockinfo_t blockinfo[] = {
{0, 1554930863, 1554930863 , 538968063 , 27 , 1 , 40667891 , 49644332 , uint256("090d45d6ab57de89def94eb6f70cc11d7eb3c09b1240da9a4248ab56ef08cc43") , uint256("b8791578d87ad4a5751ff4be7c5328c357e45ce85ddeb0eaeb6061a8f4033d87") , 0 }, // Block 1
{0, 1554930883, 1554930883 , 538968063 , 4 , 1 , 11556780 , 35585619 , uint256("07670f398c28e87d2288441ce0fa89f6ed3c6487897c362faa62eccdcd5aff4b") , uint256("3a514bbafe1ae9a0b9bf20011e2d3badc382b7937b1ad7d6c7f12826da6ecc81") , 0 }, // Block 2
{0, 1554930903, 1554930903 , 538968063 , 15 , 1 , 62672505 , 65157209 , uint256("0875439740e31181356fec6531b95e69b8bb5a262f112b031cc8e532b0c55dfa") , uint256("ad020cc2a6f6328d00e466485f8c3ff9092b8450beb255c9e838fbf37bfe2eee") , 0 }, // Block 3
{0, 1554930923, 1554930923 , 538968063 , 1 , 1 , 0 , 0 , uint256("17fe1737153d983ee0dbf99ddebf0b7d2bb34dd86e7975366a013044f2542e29") , uint256("c058b7d2e37daf00ba0d254eb8aa6ddc4040f12fe8c45c04f66ff4f6ee7b7df5") , 3600000000000 }, // Block 4
{0, 1554930943, 1554930943 , 538968063 , 11 , 1 , 0 , 0 , uint256("0e9bfe641fa71fbfed60f0162b5e8fa1a4e93fdca64a72d1d11ce133907a480b") , uint256("e94d4208c8506c5f8e873171e7e66db5b99c7e3efbb055d9ea64a6d2ca4a9335") , 7200000000000 }, // Block 5
{0, 1554930963, 1554930963 , 538968063 , 1 , 1 , 18912201 , 28192639 , uint256("09a31e3b4f42f0099c7bc3c205ca7006295c5e15aa5e6a63b3e8268ec7f58d6f") , uint256("1cb50f76afa7a852bf31dea5e57d9b9dc1117abf348201bd254d782e76801334") , 10800000000000 }, // Block 6
{0, 1554930983, 1554930983 , 538968063 , 1 , 1 , 0 , 0 , uint256("1f8b2176c585619d57b57db8b27ee0932b4718813c40544627ba8ad7a09979ca") , uint256("acbf8dc76e185a1fb9cc9d281268ad0db00df7a41d2abc13f6437a9df8f1f4ce") , 14400000000000 }, // Block 7
{0, 1554931003, 1554931003 , 538968063 , 8 , 1 , 0 , 0 , uint256("16c089dfbdc8a8e307168334b8b0e2a99755cd9a53d7a06844f64166e0000268") , uint256("af9035035ec116665fc2bb31af7936a6d0631bc75dd702d63a9c3f020ff1c6d3") , 18000000000000 }, // Block 8
{0, 1554931023, 1554931023 , 538968063 , 12 , 1 , 0 , 0 , uint256("068c81958508a14bb5efd023d9d3600003fcdade0efcbf48107ed3000f0abf6c") , uint256("6a0194e17876b968caba4c12520dfdd00305ae820f72d413a465c9377f52ac4a") , 21600000000000 }, // Block 9
{0, 1554931043, 1554931043 , 538968063 , 7 , 1 , 1806273 , 37938425 , uint256("06df31ed99f6aac5e75810d9d95468568867ee3e9d0e00dad9e5080c3f2e433e") , uint256("4141f214f959e2068fbff02e8c66ff6446792a7f228cbe4950131bcf26454f98") , 25200000000000 }, // Block 10
{0, 1554931063, 1554931063 , 538968063 , 4 , 1 , 9465122 , 19688108 , uint256("0cf67647fadea04625ecb1c40508503f567c6274a6215c7727263fb796a11ece") , uint256("47b3714365db8997b0d754ce936c02c28517fdd49f6e17d1022c58ceccbf0c8e") , 28800000000000 }, // Block 11
{0, 1554931083, 1554931083 , 538968063 , 1 , 1 , 0 , 0 , uint256("138f488120c52c51007fc4eefadd8aaeaaf310c87b3a39c5984bfadb38cb65e0") , uint256("1e346ae58583da7ac6b58185b2c3a61bbf120486b4815ad35a386718d84253ee") , 32400000000000 }, // Block 12
{0, 1554931103, 1554931103 , 538968063 , 1 , 1 , 0 , 0 , uint256("0386365ee9d78a45fff02f8d7c49a4950f266e770da439a600967ad36fdcb37d") , uint256("6e2b8d50e4fb32085a22e5f0f467b4768979936d57ed478f4f28daa263f58651") , 36000000000000 }, // Block 13
{0, 1554931123, 1554931123 , 538968063 , 1 , 1 , 0 , 0 , uint256("09d84999f5353bd2e464cb83a57ad748fb7291fc8818d16548ac94786fe7af68") , uint256("9fd52da101686ec13f3b91ff43161c8a3d2690b88893b42b2026f9f21728de46") , 39600000000000 }, // Block 14
{0, 1554931143, 1554931143 , 538968063 , 6 , 1 , 0 , 0 , uint256("03fe846719cfa5e0d26ac435efeb7776d1070a7fdba8776f569c7d7c1c5b8235") , uint256("a9efd61e15a80d847cd798a1ef7d31354b1b7c583a9c717df33e8de2618ccbab") , 43200000000000 }, // Block 15
{0, 1554931163, 1554931163 , 538968063 , 3 , 1 , 3172386 , 38682123 , uint256("02c64c51029d9c62b783e7be9642f6d74259de87d6c2128a33f506153a63db56") , uint256("1d8ed63408c0f4adf6a554d4534354abe150ea3af73fab91b1cb277eeb1b3dc4") , 46800000000000 }, // Block 16
{0, 1554931183, 1554931183 , 538968063 , 2 , 1 , 0 , 0 , uint256("1caa5b60e929c60e88317bbda1e7eedbcf1823cd71aeadec3570ae59dcb82b35") , uint256("1dee3c0be2b5b545751cc1f92f181939b26100be28480086fcd6cb6a4fdb4964") , 50400000000000 }, // Block 17
{0, 1554931203, 1554931203 , 538968063 , 4 , 1 , 0 , 0 , uint256("1048926b872d3375ad94a5b4e9b5f2474f578735e81194afd827c63dd30926dd") , uint256("919111d9b47be44c99047fcbca99ab7b50abfbd8bfc671c0d7eb6c6679852658") , 54000000000000 }, // Block 18
{0, 1554931223, 1554931223 , 538968063 , 1 , 1 , 0 , 0 , uint256("1eeec9a43e6047a688cc6049fa4b9af5f408e614331a2499bce359daecb0828c") , uint256("788f4a374cdc08b6e0efdc612796ce4dba7e632d3899d9e0cfc0a153540c0185") , 57600000000000 }, // Block 19
{0, 1554931243, 1554931243 , 538968063 , 1 , 1 , 46093529 , 57109312 , uint256("087fd470d6597236d9d599e09b525f9d0f2ea3b59b84798c67f0dc38d47ff7a4") , uint256("4f013e148be3bb077eb79624c57cc766f88170b789f93b705e221f4ceb8582ca") , 61200000000000 }, // Block 20
{0, 1554931263, 1554931263 , 538968063 , 6 , 1 , 0 , 0 , uint256("102dfbd725242726f0024f57d8ca989865b7d921793abd7ceb581d38f621e03a") , uint256("db4b40bbbb2b5e4b73300619c5b000f7fcaae6e7b98550703383a7e9480f49ae") , 64800000000000 }, // Block 21
{0, 1554931283, 1554931283 , 538968063 , 8 , 1 , 0 , 0 , uint256("1f5d3e832bdcc85ea30ea265991dd70199201e87dbf7719708bdb738d1b38038") , uint256("fb39804fb0da94d0dba64259b1266cbb41d60f5be402cd7cb3a27833a0004299") , 68400000000000 }, // Block 22
{0, 1554931303, 1554931303 , 538968063 , 7 , 1 , 12925467 , 64877471 , uint256("03d7592f2947bd5f644047bed0b6d411077c76a6afecbdafae07958796eb0ed8") , uint256("63344cf464647becb45f20e231094097f2ba6c5a303220d20cddaa28907f48cf") , 72000000000000 }, // Block 23
{0, 1554931323, 1554931323 , 538968063 , 1 , 1 , 26305447 , 65906031 , uint256("048a18fa80afa01437a80c2a2d7168ed6f4579fca2f5a9fb1eeeb8ea37c1b1a3") , uint256("d9afd0ad9166beba3aeca7f1e2327b3de89009953f56d1465423fcaccdee10f6") , 75600000000000 }, // Block 24
{0, 1554931343, 1554931343 , 538968063 , 1 , 1 , 0 , 0 , uint256("1fe6ea0a6508faf22ee242163db0da91475a0451bf6529dbd32fa4ab65e95fe4") , uint256("236ca3963e957ff5e603de54bfa239231d15a4e0e4e7d9e098eaf06943e00517") , 79200000000000 }, // Block 25
{0, 1554931363, 1554931363 , 538968063 , 2 , 1 , 0 , 0 , uint256("14f3b31ac363988d61a72b93a27f7cdacb7eda5661969368fb2cf4114593866e") , uint256("59addd74c10d18748fbdd9518dd855c4f4385c66879b090c3fe858c189639f3b") , 82800000000000 }, // Block 26
{0, 1554931383, 1554931383 , 538968063 , 1 , 1 , 0 , 0 , uint256("1602ba0f7314f2c0594fe2cd5b55f69ea43211399875460bd49a3cfe72a9f49e") , uint256("c320e08bac91351bbbe510b71570a286006caa231d994796e182828963383a77") , 86400000000000 }, // Block 27
{0, 1554931403, 1554931403 , 538968063 , 5 , 1 , 37714313 , 52064346 , uint256("091c146c540b616f8c2067557bf30b1234118ccd49680c700807b256adcd7f52") , uint256("b68b576e4b24c79338acc961c8a7094ab0b7eec5f78e1a09cbbd44dc8cf39788") , 90000000000000 }, // Block 28
{0, 1554931423, 1554931423 , 538968063 , 2 , 1 , 0 , 0 , uint256("0a12403635b80142391e1c96cdfc025c53df0d1b2baad22f615e7aa5ca653223") , uint256("aad9c974092c7efdfab046f2c1d83db1a7a8fd886e04f8932ca67e56ad68bb5b") , 93600000000000 }, // Block 29
{0, 1554931443, 1554931443 , 538968063 , 2 , 1 , 64717794 , 65606977 , uint256("09475c3056fd7832c221097db0783b03c3483eaaf4e4c91076e18ec99bab9e6d") , uint256("329b94a34c1a489daa9c1796871ef576f6c4772020cd1a52b3788bb14ce5b45c") , 97200000000000 }, // Block 30
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
            if ((GetTime() - nMintableLastCheck > Params().GetTargetSpacing())) {
                nMintableLastCheck = GetTime();
                fMintableCoins = pwallet->MintableCoins();
            }

            while (pwallet->IsLocked() || !fMintableCoins ||
                   (pwallet->GetBalance() > 0 && nReserveBalance >= pwallet->GetBalance())) {
                nLastCoinStakeSearchInterval = 0;
                // Do a separate 1 minute check here to ensure fMintableCoins is updated
                if (!fMintableCoins) {
                    if (GetTime() - nMintableLastCheck > Params().GetTargetSpacing()) // 1 minute check time
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
                if (GetTime() - mapHashedBlocks[chainActive.Tip()->nHeight] < Params().GetTargetSpacing() * 0.75 / 2) // wait half of the nHashDrift
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
