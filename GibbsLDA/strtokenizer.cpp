/*
 * Copyright (C) 2007 by
 * 
 *         Xuan-Hieu Phan
 *        hieuxuan@ecei.tohoku.ac.jp or pxhieu@gmail.com
 *         Graduate School of Information Sciences
 *         Tohoku University
 *
 * GibbsLDA++ is a free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * GibbsLDA++ is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GibbsLDA++; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#include <string>
#include <vector>
#include "strtokenizer.h"
 
using namespace std;

strtokenizer::strtokenizer(string str, string seperators) {
    parse(str, seperators);
}

void strtokenizer::parse(string str, string seperators) {
    int n = str.length();
    int start, stop, delim;
    
    start = str.find_first_not_of(seperators);
    while (start >= 0 && start < n) {
        stop = str.find_first_of(seperators, start);
        if (stop < 0 || stop > n) {
            stop = n;
        }
        delim = str.find_first_of(':', start);
        int value;
        if (delim <= 0 || delim >= stop) {
            // it's raw string without value
            delim = stop;
            value = 0;
        } else {
            value = atoi(str.substr(delim + 1, stop - delim - 1).c_str());
        }
        string word = str.substr(start, delim - start);
        //printf("word: |%s|; count: %d\n", word.c_str(), value);
        tokens.push_back(pair<string, int>(word, value));
        start = str.find_first_not_of(seperators, stop + 1);
    }
    
    start_scan();
}

int strtokenizer::count_tokens() {
    return tokens.size();
}

void strtokenizer::start_scan() {
    idx = 0;
}

pair<string, int> strtokenizer::next_token() {
    if (idx >= 0 && idx < tokens.size()) {
        return tokens[idx++];
    } else {
        return pair<string, int>();
    }
}

pair<string, int> strtokenizer::token(int i) {
    if (i >= 0 && i < tokens.size()) {
        return tokens[i];
    } else {
        return pair<string, int>();
    }
}

