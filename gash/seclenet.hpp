//
//  lenet5.hpp
//  gash
//
//  Created by Xiaoting Tang on 6/21/18.
//  Copyright © 2018 Xiaoting Tang. All rights reserved.
//

#ifndef seclenet_hpp
#define seclenet_hpp

#include "ss.hpp"

#define LENGTH_KERNEL    5

#define LENGTH_FEATURE0    32
#define LENGTH_FEATURE1    (LENGTH_FEATURE0 - LENGTH_KERNEL + 1)
#define LENGTH_FEATURE2    (LENGTH_FEATURE1 >> 1)
#define LENGTH_FEATURE3    (LENGTH_FEATURE2 - LENGTH_KERNEL + 1)
#define LENGTH_FEATURE4    (LENGTH_FEATURE3 >> 1)
#define LENGTH_FEATURE5    (LENGTH_FEATURE4 - LENGTH_KERNEL + 1)

#define INPUT             1
#define LAYER1            6
#define LAYER2            6
#define LAYER3            16
#define LAYER4            16
#define LAYER5            120
#define OUTPUT            10

#define ALPHA 0.5
#define PADDING 2

typedef unsigned char uint8;
typedef uint8 image[28][28];
typedef secdouble SecImage[28][28];


typedef struct LeNet5
{
    double weight0_1[INPUT][LAYER1][LENGTH_KERNEL][LENGTH_KERNEL];
    double weight2_3[LAYER2][LAYER3][LENGTH_KERNEL][LENGTH_KERNEL];
    double weight4_5[LAYER4][LAYER5][LENGTH_KERNEL][LENGTH_KERNEL];
    double weight5_6[LAYER5 * LENGTH_FEATURE5 * LENGTH_FEATURE5][OUTPUT];
    
    double bias0_1[LAYER1];
    double bias2_3[LAYER3];
    double bias4_5[LAYER5];
    double bias5_6[OUTPUT];
    
}LeNet5;

typedef struct Feature
{
    double input[INPUT][LENGTH_FEATURE0][LENGTH_FEATURE0];
    double layer1[LAYER1][LENGTH_FEATURE1][LENGTH_FEATURE1];
    double layer2[LAYER2][LENGTH_FEATURE2][LENGTH_FEATURE2];
    double layer3[LAYER3][LENGTH_FEATURE3][LENGTH_FEATURE3];
    double layer4[LAYER4][LENGTH_FEATURE4][LENGTH_FEATURE4];
    double layer5[LAYER5][LENGTH_FEATURE5][LENGTH_FEATURE5];
    double output[OUTPUT];
}Feature;


typedef struct SecLeNet5
{
    secdouble weight0_1[INPUT][LAYER1][LENGTH_KERNEL][LENGTH_KERNEL];
    secdouble weight2_3[LAYER2][LAYER3][LENGTH_KERNEL][LENGTH_KERNEL];
    secdouble weight4_5[LAYER4][LAYER5][LENGTH_KERNEL][LENGTH_KERNEL];
    secdouble weight5_6[LAYER5 * LENGTH_FEATURE5 * LENGTH_FEATURE5][OUTPUT];
    
    secdouble bias0_1[LAYER1];
    secdouble bias2_3[LAYER3];
    secdouble bias4_5[LAYER5];
    secdouble bias5_6[OUTPUT];
} SecLeNet5;

typedef struct SecFeature
{
    secdouble input[INPUT][LENGTH_FEATURE0][LENGTH_FEATURE0];
    secdouble layer1[LAYER1][LENGTH_FEATURE1][LENGTH_FEATURE1];
    secdouble layer2[LAYER2][LENGTH_FEATURE2][LENGTH_FEATURE2];
    secdouble layer3[LAYER3][LENGTH_FEATURE3][LENGTH_FEATURE3];
    secdouble layer4[LAYER4][LENGTH_FEATURE4][LENGTH_FEATURE4];
    secdouble layer5[LAYER5][LENGTH_FEATURE5][LENGTH_FEATURE5];
    secdouble output[OUTPUT];
}SecFeature;

void TrainBatch(LeNet5 *lenet, image *inputs, uint8 *labels, int batchSize);

void Train(LeNet5 *lenet, image input, uint8 label);

uint8 Predict(LeNet5 *lenet, image input, uint8 count);

void Initial(LeNet5 *lenet);


void SendInitialShare(LeNet5* lenet, image *inputs, uint8 *labels);
void Monitoring(int batchsize, int count_train);
void ReconstructAsMaster(LeNet5* lenet);
void RecvInitialShare(SecLeNet5* seclenet, SecImage* inputs, secdouble* labels);
void Interact(SecLeNet5* seclenet, SecImage* inputs, secdouble* labels, int batchsize, int count_train);
void ReconstructAsSlave(SecLeNet5* seclenet);





#endif /* lenet5_hpp */
