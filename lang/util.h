/*
 * util.h -- Utilities
 *
 * Author: Xiaoting Tang <tang_xiaoting@brown.edu>
 * Copyright: Xiaoting Tang (2018)
 *
* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
  * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "../common.h"

#ifndef GASH_LANG_UTIL_H
#define GASH_LANG_UTIL_H

namespace gashlang {

  /**
   * Parse an array definition like 'int64[10]'.
   * Depending on what i is (either 0 or 1), return either the size of the element (64) or
   * the length of the array (10).
   *
   * @param s
   * @param i
   *
   * @return
   */
  inline u32 parse_num_array_expression(char* s, u32 i) {
    std::vector<std::string> parts;
    char* res;
    res = strtok(s, "[");
    while (res != NULL) {
      parts.push_back(std::string(res));
      res = strtok(NULL, "[");
    }

    u32 int_size = atoi(parts[0].c_str()+3);

    char part1[parts[1].size() + 1];
    strcpy(part1, parts[1].c_str());
    res = strtok(part1, "[");
    parts.clear();
    parts.push_back(string(res));

    u32 arr_size = atoi(parts[0].c_str());

    if (i == 0)
      return int_size;
    else if (i == 1)
      return arr_size;
    else
      FATAL("i can only be 0 or 1");
  }

}  // gashlang

#endif
