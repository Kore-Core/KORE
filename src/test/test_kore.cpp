// Copyright (c) 2011-2013 The Bitcoin Core developers
// Copyright (c) 2017 The KORE developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define BOOST_TEST_MODULE Kore Test Suite

#include "random.h"
#include "txdb.h"
#include "tests_util.h"
#include "ui_interface.h"
#include "util.h"
#ifdef ENABLE_WALLET
#include "db.h"
#include "wallet.h"
#endif

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>

CClientUIInterface uiInterface;

extern bool fPrintToConsole;
extern void noui_connect();

struct TestingSetup {
    boost::filesystem::path pathTemp;
    boost::thread_group threadGroup;
    ECCVerifyHandle globalVerifyHandle;

    TestingSetup() {
        ECC_Start();
        SetupEnvironment();
        fPrintToDebugLog = true; // don't want to write to debug.log file
        fCheckBlockIndex = true;
        fDebug = true;
        SelectParams(CBaseChainParams::UNITTEST);
        noui_connect();
        //pathTemp = GetTempPath() / strprintf("test_kore_%lu_%i", (unsigned long)GetTime(), (int)(GetRand(100000)));
        pathTemp = boost::filesystem::path("./delete-me") / strprintf("test_kore_%lu_%i", (unsigned long)GetTime(), (int)(GetRand(100000)));
        cout << "Running at :" << pathTemp << endl;
        boost::filesystem::create_directories(pathTemp);
        mapArgs["-datadir"] = pathTemp.string();
        mapArgs["-printstakemodifier"] = "1";
        InitializeDBTest(pathTemp);
        nScriptCheckThreads = 3;
        for (int i=0; i < nScriptCheckThreads-1; i++)
            threadGroup.create_thread(&ThreadScriptCheck);
        RegisterNodeSignals(GetNodeSignals());
    }
    ~TestingSetup()
    {
        threadGroup.interrupt_all();
        threadGroup.join_all();
        UnregisterNodeSignals(GetNodeSignals());
        FinalizeDBTest(true);
        ECC_Stop();
        //boost::filesystem::remove_all(pathTemp);
    }
};

BOOST_GLOBAL_FIXTURE(TestingSetup);

void Shutdown(void* parg)
{
  exit(0);
}

void StartShutdown()
{
  exit(0);
}

bool ShutdownRequested()
{
  return false;
}
