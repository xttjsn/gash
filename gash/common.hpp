//
//  common.hpp
//  gash
//
//  Created by Xiaoting Tang on 6/18/18.
//  Copyright © 2018 Xiaoting Tang. All rights reserved.
//

#ifndef common_hpp
#define common_hpp

#include <stdio.h>
#include <iostream>

#define WARNING(str) std::cerr << str << std::endl

#define FATAL(str)                                                              \
    std::cerr << str << std::endl;                                              \
    abort()

#define REQUIRE_NOT_NULL(expr)                                                          \
    do {                                                                                \
        if (expr == NULL) {                                                             \
            FATAL(__FILE__ << ": " << __LINE__ << ": " << #expr << " is NULL");         \
        }                                                                               \
    } while (0)

#define NOT_YET_IMPLEMENTED(f) FATAL("Not yet implemented error:" << f)

#define GASSERT(expr)                                                                   \
    do {                                                                                \
        if (!(expr)) {                                                                  \
            FATAL(__FILE__ << ": " << __LINE__ << ": " << #expr << " failed");          \
        }                                                                               \
    } while (0)

#define PORT_GC 45556
#define PORT_OT 41231

#define INDENTx1 "    "
#define INDENTx2 "        "

//#define GASH_DEBUG

#endif /* common_hpp */
