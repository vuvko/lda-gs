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

#ifndef _STRTOKENIZER_H
#define _STRTOKENIZER_H

#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>
#include <map>

using namespace std;

class strtokenizer {
protected:
    vector<pair<string, int> > tokens;
    int idx;

public:
    strtokenizer(string str, string seperators = " ");    
    
    void parse(string str, string seperators);
    
    int count_tokens();
    pair<string, int> next_token();
    void start_scan();

    pair<string, int> token(int i);
};

#endif

