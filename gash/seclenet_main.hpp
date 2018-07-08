//
//  seclenet_main.hpp
//  gash
//
//  Created by Xiaoting Tang on 6/21/18.
//  Copyright © 2018 Xiaoting Tang. All rights reserved.
//

#ifndef seclenet_main_hpp
#define seclenet_main_hpp

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define FILE_TRAIN_IMAGE        "train-images-idx3-ubyte"
#define FILE_TRAIN_LABEL        "train-labels-idx1-ubyte"
#define FILE_TEST_IMAGE         "t10k-images-idx3-ubyte"
#define FILE_TEST_LABEL         "t10k-labels-idx1-ubyte"
#define LENET_FILE              "model.dat"
#define COUNT_TRAIN             60000
#define COUNT_TEST              10000

void seclenet_main();
void seclenet_p0_main();
void seclenet_p1_main();


#endif /* seclenet_main_hpp */
