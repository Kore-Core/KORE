// Copyright (c) 2011-2013 The Bitcoin Core developers
// Copyright (c) 2017 The KORE developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

//
// Unit tests for block-chain checkpoints
//

#include "checkpoints.h"

#include "uint256.h"

#include <boost/test/unit_test.hpp>

using namespace std;

BOOST_AUTO_TEST_SUITE(Checkpoints_tests)

BOOST_AUTO_TEST_CASE(sanity)
{
    uint256 p0 = uint256("0x0aab10677b4fe0371a67f99e78a69e7d9fa03a1c7d48747978da405dc5abeb99");
    uint256 p5 = uint256("0x00eaaa465402e6bcf745c00c38c0033a26e4dea19448d9109e4555943d677a31");
    uint256 p1000 = uint256("0x2073f0a245cedde8344c2d0b48243a58908ffa50b02e2378189f2bb80037abd9");
    uint256 p40000 = uint256("0x572b31cc34f842aecbbc89083f7e40fff6a07e73e6002be75cb95468f4e3b4ca");
    uint256 p80000 = uint256("0x070aa76a8a879f3946322086a542dd9e4afca81efafd7642192ed9fe56ba74f1");
    uint256 p120000 = uint256("0x70edc85193638b8adadb71ea766786d207f78a173dd13f965952eb76932f5729");
    uint256 p209536 = uint256("0x8a718dbb44b57a5693ac70c951f2f81a01b39933e3e19e841637f757598f571a");

    BOOST_CHECK(Checkpoints::CheckBlock(0, p0));
    BOOST_CHECK(Checkpoints::CheckBlock(5, p5));
    BOOST_CHECK(Checkpoints::CheckBlock(1000, p1000));
    BOOST_CHECK(Checkpoints::CheckBlock(40000, p40000));
    BOOST_CHECK(Checkpoints::CheckBlock(80000, p80000));
    BOOST_CHECK(Checkpoints::CheckBlock(120000, p120000));
    BOOST_CHECK(Checkpoints::CheckBlock(209536, p209536));

    // Wrong hashes at checkpoints should fail:
    BOOST_CHECK(Checkpoints::CheckBlock(0, p120000) == false);
    BOOST_CHECK(Checkpoints::CheckBlock(5, p209536) == false);
    BOOST_CHECK(Checkpoints::CheckBlock(1000, p80000) == false);
    BOOST_CHECK(Checkpoints::CheckBlock(40000, p5) == false);
    BOOST_CHECK(Checkpoints::CheckBlock(80000, p40000) == false);
    BOOST_CHECK(Checkpoints::CheckBlock(120000, p1000) == false);
    BOOST_CHECK(Checkpoints::CheckBlock(209536, p0) == false);

    // ... but any hash not at a checkpoint should succeed:
    BOOST_CHECK(Checkpoints::CheckBlock(0 + 1, p0));
    BOOST_CHECK(Checkpoints::CheckBlock(5 + 1, p5));
    BOOST_CHECK(Checkpoints::CheckBlock(1000 + 1, p1000));
    BOOST_CHECK(Checkpoints::CheckBlock(40000 + 1, p40000));
    BOOST_CHECK(Checkpoints::CheckBlock(80000 + 1, p80000));
    BOOST_CHECK(Checkpoints::CheckBlock(120000 + 1, p120000));
    BOOST_CHECK(Checkpoints::CheckBlock(209536 + 1, p209536));

    BOOST_CHECK(Checkpoints::GetTotalBlocksEstimate() >= 209536);
}

BOOST_AUTO_TEST_SUITE_END()
