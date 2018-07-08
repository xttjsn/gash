//
//  seclenet_main.cpp
//  gash
//
//  Created by Xiaoting Tang on 6/21/18.
//  Copyright © 2018 Xiaoting Tang. All rights reserved.
//

#include "seclenet_main.hpp"
#include "seclenet.hpp"

int read_data(unsigned char(*data)[28][28], unsigned char label[], const int count, const char data_file[], const char label_file[])
{
    FILE *fp_image = fopen(data_file, "rb");
    FILE *fp_label = fopen(label_file, "rb");
    if (!fp_image||!fp_label) return 1;
    fseek(fp_image, 16, SEEK_SET);
    fseek(fp_label, 8, SEEK_SET);
    fread(data, sizeof(*data)*count, 1, fp_image);
    fread(label,count, 1, fp_label);
    fclose(fp_image);
    fclose(fp_label);
    return 0;
}

void training(LeNet5 *lenet, image *train_data, uint8 *train_label, int batch_size, int total_size)
{
    for (int i = 0, percent = 0; i <= total_size - batch_size; i += batch_size)
    {
        TrainBatch(lenet, train_data + i, train_label + i, batch_size);
        if (i * 100 / total_size > percent)
            printf("batchsize:%d\ttrain:%2d%%\n", batch_size, percent = i * 100 / total_size);
    }
}

int testing(LeNet5 *lenet, image *test_data, uint8 *test_label,int total_size)
{
    int right = 0, percent = 0;
    for (int i = 0; i < total_size; ++i)
    {
        uint8 l = test_label[i];
        int p = Predict(lenet, test_data[i], 10);
        right += l == p;
        if (i * 100 / total_size > percent)
            printf("test:%2d%%\n", percent = i * 100 / total_size);
    }
    return right;
}

int save(LeNet5 *lenet, char filename[])
{
    FILE *fp = fopen(filename, "wb");
    if (!fp) return 1;
    fwrite(lenet, sizeof(LeNet5), 1, fp);
    fclose(fp);
    return 0;
}

int load(LeNet5 *lenet, char filename[])
{
    FILE *fp = fopen(filename, "rb");
    if (!fp) return 1;
    fread(lenet, sizeof(LeNet5), 1, fp);
    fclose(fp);
    return 0;
}

void seclenet_client_main()
{
    image *train_data = (image *)calloc(COUNT_TRAIN, sizeof(image));
    uint8 *train_label = (uint8 *)calloc(COUNT_TRAIN, sizeof(uint8));
    image *test_data = (image *)calloc(COUNT_TEST, sizeof(image));
    uint8 *test_label = (uint8 *)calloc(COUNT_TEST, sizeof(uint8));
    if (read_data(train_data, train_label, COUNT_TRAIN, FILE_TRAIN_IMAGE, FILE_TRAIN_LABEL))
    {
        printf("ERROR!!!\nDataset File Not Find!Please Copy Dataset to the Floder Included the exe\n");
        free(train_data);
        free(train_label);
        system("pause");
    }
    if (read_data(test_data, test_label, COUNT_TEST, FILE_TEST_IMAGE, FILE_TEST_LABEL))
    {
        printf("ERROR!!!\nDataset File Not Find!Please Copy Dataset to the Floder Included the exe\n");
        free(test_data);
        free(test_label);
        system("pause");
    }
    
    LeNet5 *lenet = (LeNet5 *)malloc(sizeof(LeNet5));
    if (load(lenet, LENET_FILE))
        Initial(lenet);
    
    gash_ss_client_init();
    SendInitialShare(lenet, train_data, train_label);
    
    clock_t start = clock();
    int batches[] = { 300 };
    for (int i = 0; i < sizeof(batches) / sizeof(*batches);++i)
        Monitoring(batches[i], COUNT_TRAIN);
    
    ReconstructAsMaster(lenet);
    int right = testing(lenet, test_data, test_label, COUNT_TEST);
    printf("%d/%d\n", right, COUNT_TEST);
    printf("Time:%u\n", (unsigned)(clock() - start));
    //save(lenet, LENET_FILE);
    free(lenet);
    free(train_data);
    free(train_label);
    free(test_data);
    free(test_label);
    system("pause");
}

void seclenet_p0_main()
{
    SecLeNet5* seclenet = (SecLeNet5*)malloc(sizeof(SecLeNet5));
    SecImage*  train_data = (SecImage*)calloc(COUNT_TRAIN, sizeof(SecImage));
    secdouble*  train_label = (secdouble*)calloc(COUNT_TRAIN, sizeof(secdouble));
    
    gash_init_as_garbler("127.0.0.1");
    gash_ss_garbler_init("127.0.0.1");
    RecvInitialShare(seclenet, train_data, train_label);
    
    int batches[] = { 300 };
    for (int i = 0; i < sizeof(batches) / sizeof(*batches); ++i)
        Interact(seclenet, train_data, train_label, batches[i], COUNT_TRAIN);
    
    ReconstructAsSlave(seclenet);
    free(seclenet);
    free(train_data);
    free(train_label);
}

void seclenet_p1_main()
{
    SecLeNet5* seclenet = (SecLeNet5*)malloc(sizeof(SecLeNet5));
    SecImage*  train_data = (SecImage*)calloc(COUNT_TRAIN, sizeof(SecImage));
    secdouble*  train_label = (secdouble*)calloc(COUNT_TRAIN, sizeof(secdouble));
    
    gash_init_as_evaluator("127.0.0.1");
    gash_ss_evaluator_init("127.0.0.1", "127.0.0.1");
    RecvInitialShare(seclenet, train_data, train_label);
    
    int batches[] = { 300 };
    for (int i = 0; i < sizeof(batches) / sizeof(*batches); ++i)
        Interact(seclenet, train_data, train_label, batches[i], COUNT_TRAIN);
    
    ReconstructAsSlave(seclenet);
    free(seclenet);
    free(train_data);
    free(train_label);
}
