// Copyright (c) 2018 The Kore Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef KORE_SUPPORT_CSVROW_H
#define KORE_SUPPORT_CSVROW_H

#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

class CSVRow
{
public:
    std::string const& operator[](std::size_t index) const;
    std::size_t size() const;
    std::istream& readNextRow(std::istream& str, char separator = ',');

    void operator<<(std::istream& str);

private:
    std::vector<std::string> m_data;
    char m_separator;
};

#endif // KORE_SUPPORT_CSVROW_H