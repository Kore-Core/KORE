// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2016-2017 The KORE developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include "config/bitcoin-config.h"
#endif

#include "tinyformat.h"
#include "utiltime.h"

#include <boost/thread.hpp>
#include <thread>

using namespace std;

static int64_t nMockTime = 0; //! For unit testing

#ifdef __APPLE__
std::chrono::system_clock::time_point GetEpochTimePoint() {
    std::tm t = {};
    std::istringstream ss("1970-01-01T00:00:00Z");
    ss >> std::get_time(&t, "%Y-%m-%dT%H:%M:%SZ");
    return chrono::system_clock::from_time_t(std::mktime(&t));
};
#else
std::chrono::high_resolution_clock::time_point GetEpochTimePoint() {
    std::tm t = {};
    std::istringstream ss("1970-01-01T00:00:00Z");
    ss >> std::get_time(&t, "%Y-%m-%dT%H:%M:%SZ");
    return chrono::high_resolution_clock::from_time_t(std::mktime(&t));
};
#endif

int64_t GetTime()
{
    if (nMockTime) return nMockTime;

    return time(NULL);
}

void SetMockTime(int64_t nMockTimeIn)
{
    nMockTime = nMockTimeIn;
}

int64_t GetTimeSeconds()
{
#ifdef __APPLE__
    return (chrono::system_clock::now() - GetEpochTimePoint()) / chrono::seconds(1);
#else
    return (chrono::high_resolution_clock::now() - GetEpochTimePoint()) / chrono::seconds(1);
#endif
}

int64_t GetTimeMillis()
{
#ifdef __APPLE__
    return (chrono::system_clock::now() - GetEpochTimePoint()) / chrono::milliseconds(1);
#else
    return (chrono::high_resolution_clock::now() - GetEpochTimePoint()) / chrono::milliseconds(1);
#endif
}

int64_t GetTimeMicros()
{
#ifdef __APPLE__
    return (chrono::system_clock::now() - GetEpochTimePoint()) / chrono::microseconds(1);
#else
    return (chrono::high_resolution_clock::now() - GetEpochTimePoint()) / chrono::microseconds(1);
#endif
}

void MilliSleep(int64_t n)
{
/**
 * Boost's sleep_for was uninterruptable when backed by nanosleep from 1.50
 * until fixed in 1.52. Use the deprecated sleep method for the broken case.
 * See: https://svn.boost.org/trac/boost/ticket/7238
 */
//#if defined(HAVE_WORKING_BOOST_SLEEP_FOR)
//    boost::this_thread::sleep_for(boost::chrono::milliseconds(n));
//#else
    this_thread::sleep_for(std::chrono::milliseconds(n));
//#endif
}

std::string DateTimeStrFormat(const char* pszFormat, int64_t nTime)
{
    time_t rawtime(nTime);
    struct tm * timeinfo = gmtime(&rawtime);
    char buffer [80];

    strftime(buffer,80,pszFormat,timeinfo);

    return string(buffer);
}

std::string DurationToDHMS(int64_t nDurationTime)
{
    int seconds = nDurationTime % 60;
    nDurationTime /= 60;
    int minutes = nDurationTime % 60;
    nDurationTime /= 60;
    int hours = nDurationTime % 24;
    int days = nDurationTime / 24;
    if (days)
        return strprintf("%dd %02dh:%02dm:%02ds", days, hours, minutes, seconds);
    if (hours)
        return strprintf("%02dh:%02dm:%02ds", hours, minutes, seconds);
    return strprintf("%02dm:%02ds", minutes, seconds);
}
