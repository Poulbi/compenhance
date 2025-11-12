#include "libs/lr/lr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#define MemoryCopy memcpy

#include "listing_065.cpp"

#include "haversine_random.h"

enum generation_method : u32
{
    Method_Uniform,
    Method_Cluster
};

int main(int ArgsCount, char *Args[], char *Env[])
{
    // 1. haversine_generator [uniform/cluster] [random seed] [number of pairs to generate]
    
    if(ArgsCount >= 4)
    {
        u32 Method = 0;
        u64 RandomSeed = 0;;
        u64 PairsCount = 0;
        b32 Error = false;
        
        if(!strcmp(Args[1], "uniform"))
        {
            Method = Method_Uniform;
        }
        else if(!strcmp(Args[1], "cluster"))
        {
            Method = Method_Cluster;
        }
        else
        {
            Error = true;
        }
        
        RandomSeed = atoll(Args[2]);
        
        if(RandomSeed == 0)
        {
            if(Args[2][0] == '0')
            {
                RandomSeed = 0;
            }
            else
            {
                Error = true;
            }
        }
        
        PairsCount = atoll(Args[3]);
        if(PairsCount == 0)
        {
            Error = true;
        }
        
        if(!Error)
        {
            printf("Method: %s\n"
                   "Random seed: %lu\n"
                   "Pairs count: %lu\n"
                   , Args[1], RandomSeed, PairsCount);
            
            // Generate pairs in the following format.
            // 
            // {
            //  "pairs": 
            //   [
            //    { "x0": ..., "y0": ..., "x1": ..., "y1": ... },
            //    { "x0": ..., "y0": ..., "x1": ..., "y1": ... }
            //   ]
            // }
            // 
            
            FILE *Out = fopen("out.json", "wb");
            char *JsonHeader = 
                "{\n"
                " \"pairs\":\n"
                "   [\n";
            char *JsonFooter =
                "   ]\n"
                "}\n";
            
            fprintf(Out, "%s", JsonHeader);
            
            pcg64_random_t RNG = {};
            pcg64_srandom_r(&RNG, RandomSeed, RandomSeed);
            
            f64 AverageSum = 0;
            f64 TotalSum = 0;
            for(u64 PairsIndex = 0;
                PairsIndex < PairsCount;
                PairsIndex++)
            {
                b32 LastPair = false;
                
                f64 X0 = RandomBetween(&RNG, -180.0, 180.0);
                f64 Y0 = RandomBetween(&RNG, -360.0, 360.0);
                f64 X1 = RandomBetween(&RNG, -180.0, 180.0); 
                f64 Y1 = RandomBetween(&RNG, -360.0, 360.0);
                
                f64 Sum = ReferenceHaversine(X0, Y0, X1, Y1, 6372.8);
                TotalSum += Sum;
                
                // NOTE(luca): A double's mantissa is 52 bits.  2^52 - 1 is 4503599627370495 which has 
                // 16 digits.
                fprintf(Out, 
                        "    { \"x0\": %.15f, \"y0\": %.15f, \"x1\": %.15f, \"y1\": %.15f }\n", 
                        X0, Y0, X1, Y1);
                
            }
            AverageSum = TotalSum / (f64)PairsCount;
            
            fprintf(Out, "%s", JsonFooter);
            
            printf("Average sum: %f\n", AverageSum);
        }
        else
        {
            printf("Usage: %s [uniform/cluster] [random seed] [number of pairs to generate]\n", 
                   Args[0]);
        }
    }
    else
    {
        printf("Usage: %s [uniform/cluster] [random seed] [number of pairs to generate]\n", 
               Args[0]);
    }
    
    return 0;
}