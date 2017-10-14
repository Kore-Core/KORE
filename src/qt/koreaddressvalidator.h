// Copyright (c) 2011-2014 The KoreCore developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_BITCOINADDRESSVALIDATOR_H
#define BITCOIN_QT_BITCOINADDRESSVALIDATOR_H

#include <QValidator>

/** Base58 entry widget validator, checks for valid characters and
 * removes some whitespace.
 */
class KoreAddressEntryValidator : public QValidator
{
    Q_OBJECT

public:
    explicit KoreAddressEntryValidator(QObject *parent);

    State validate(QString &input, int &pos) const;
};

/** Koreaddress widget validator, checks for a valid kore address.
 */
class KoreAddressCheckValidator : public QValidator
{
    Q_OBJECT

public:
    explicit KoreAddressCheckValidator(QObject *parent);

    State validate(QString &input, int &pos) const;
};

#endif // BITCOIN_QT_BITCOINADDRESSVALIDATOR_H
