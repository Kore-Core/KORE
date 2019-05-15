// Copyright (c) 2018 The Kore Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "csviterator.h"

#include <iostream>
#include <sstream>

CSVIterator::CSVIterator(std::istream& str) : m_str(str.good() ? &str : NULL) 
{
    
    ++(*this);
}
CSVIterator::CSVIterator(std::istream& str, char separator) : m_str(str.good() ? &str : NULL)
{
    m_separator = separator;
    ++(*this); 
}
CSVIterator::CSVIterator() : m_str(NULL) {}

// Pre Increment
CSVIterator& CSVIterator::operator++()
{
    if (m_str) {
        if (!m_row.readNextRow(*m_str, m_separator)) {
            m_str = NULL;
        }
    }
    return *this;
}
// Post increment
CSVIterator CSVIterator::operator++(int)
{
    CSVIterator tmp(*this);
    ++(*this);
    return tmp;
}
CSVRow const& CSVIterator::operator*() const { return m_row; }
CSVRow const* CSVIterator::operator->() const { return &m_row; }

bool CSVIterator::operator==(CSVIterator const& rhs) { return ((this == &rhs) || ((this->m_str == NULL) && (rhs.m_str == NULL))); }
bool CSVIterator::operator!=(CSVIterator const& rhs) { return !((*this) == rhs); }