Kore Core integration/staging tree
=====================================

[![Build Status](https://travis-ci.org/kore/kore.svg?branch=master)](https://travis-ci.org/kore/kore)

https://kore.life

What is Kore?
----------------

Kore is an experimental new digital currency that enables instant payments to
anyone, anywhere in the world. Kore uses peer-to-peer technology to operate
with no central authority: managing transactions and issuing money are carried
out collectively by the network. Kore Core is the name of open source
software which enables the use of this currency.

License
-------

Kore Core is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see https://opensource.org/licenses/MIT.

Development Process
-------------------

The `master` branch is regularly built and tested, but is not guaranteed to be
completely stable. [Tags](https://github.com/kore/kore/tags) are created
regularly to indicate new official, stable release versions of Kore Core.

The contribution workflow is described in [CONTRIBUTING.md](CONTRIBUTING.md).

The developer [mailing list](https://lists.linuxfoundation.org/mailman/listinfo/kore-dev)
should be used to discuss complicated or controversial changes before working
on a patch set.

Developer IRC can be found on Freenode at #kore-core-dev.

Testing
-------

Testing and code review is the bottleneck for development; we get more pull
requests than we can review and test on short notice. Please be patient and help out by testing
other people's pull requests, and remember this is a security-critical project where any mistake might cost people
lots of money.

### Automated Testing

Developers are strongly encouraged to write [unit tests](/doc/unit-tests.md) for new code, and to
submit new unit tests for old code. Unit tests can be compiled and run
(assuming they weren't disabled in configure) with: `make check`

There are also [regression and integration tests](/qa) of the RPC interface, written
in Python, that are run automatically on the build server.
These tests can be run (if the [test dependencies](/qa) are installed) with: `qa/pull-tester/rpc-tests.py`

The Travis CI system makes sure that every pull request is built for Windows
and Linux, OS X, and that unit and sanity tests are automatically run.

### Manual Quality Assurance (QA) Testing

Changes should be tested by somebody other than the developer who wrote the
code. This is especially important for large or high-risk changes. It is useful
to add a test plan to the pull request description if testing the changes is
not straightforward.

Translations
------------

Changes to translations as well as new translations can be submitted to
[Kore Core's Transifex page](https://www.transifex.com/projects/p/kore/).

Translations are periodically pulled from Transifex and merged into the git repository. See the
[translation process](doc/translation_process.md) for details on how this works.

**Important**: We do not accept translation changes as GitHub pull requests because the next
pull from Transifex would automatically overwrite them again.

Translators should also subscribe to the [mailing list](https://groups.google.com/forum/#!forum/kore-translators).

Wallet Updates
--------------

1-11-2018 - v0.12.4.0

NOTE: The wallet still has the disappearing coins and reindexing
issues experienced by many PoSv3 coins at this current time.
The KORE wallet update to version 0.12.4.0 includes:
Partial memory leak fixes, other mods and fixes.
Rather than remove the Bittrex trading window the wallet has been updated with 
a check box toggle to "Enable Trading," found at the bottom of the page.
The wallet opens with the trading window disabled. Click to enable it.
Once finished with trades it is recommended to disable it.
The box to unlock for staking only is auto checked.
The staking icon remains on for a longer period of time when coins are sent out to stake.
Various links and spelling corrections.
Commented out a buggy non functioning wallet repair option.
Updated nodes list.
Added zapwallettxes to the kore.conf file.
Windows 32bit version.
Updated curl and dependency lists.
Increased outbound connections.
Increased the stake split threshold.
Lowered difficulty.
Updated ssl.
Various other bug fixes.
