// Copyright (c) 2018 The Kore Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef KORE_SUPPORT_CSVITERATOR_H
#define KORE_SUPPORT_CSVITERATOR_H

#include "csvrow.h"

class CSVIterator
{
public:
    typedef std::input_iterator_tag iterator_category;
    typedef CSVRow value_type;
    typedef std::size_t difference_type;
    typedef CSVRow* pointer;
    typedef CSVRow& reference;

    CSVIterator(std::istream& str);
    CSVIterator(std::istream& str, char separator);
    CSVIterator();

    // Pre Increment
    CSVIterator& operator++();
    // Post increment
    CSVIterator operator++(int);
    CSVRow const& operator*() const;
    CSVRow const* operator->() const;

    bool operator==(CSVIterator const& rhs);
    bool operator!=(CSVIterator const& rhs);

private:
    std::istream* m_str;
    CSVRow m_row;
    char m_separator = ',';
};

#endif // KORE_SUPPORT_CSVITERATOR_H