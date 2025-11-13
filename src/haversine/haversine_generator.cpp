//~ Libraries
#include "libs/lr/lr.h"
PUSH_WARNINGS
#define STB_SPRINTF_IMPLEMENTATION
#include "libs/stb_sprintf.h"
POP_WARNINGS
#include "libs/listing_065.cpp"

//~ Standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "haversine_random.h"

//~ Macro's
#define MemoryCopy memcpy


//~ Constants
#define ClusterCount 64
// NOTE(luca): A double's mantissa is 52 bits.  2^52 - 1 is 4503599627370495 which has 
// 16 digits.
#define PointJsonFormat "{ \"x0\": %.15f, \"y0\": %.15f, \"x1\": %.15f, \"y1\": %.15f }\n"

//~ Types
#include "generated/types.h"

struct cluster
{
    f64 X;
    f64 Y;
    f64 Width;
    f64 Height;
};

//~ Main
int main(int ArgsCount, char *Args[], char *Env[])
{
    // 1. haversine_generator [uniform/cluster] [random seed] [number of pairs to generate]
    
    if(ArgsCount >= 4)
    {
        u32 Method = 0;
        u64 RandomSeed = 0;;
        u64 PairCount = 0;
        b32 Error = false;
        
        char *MethodString = Args[1];
        char *SeedString = Args[2];
        char *PairCountString = Args[3];
        
        if(!strcmp(MethodString, "uniform"))
        {
            Method = Method_Uniform;
        }
        else if(!strcmp(MethodString, "cluster"))
        {
            Method = Method_Cluster;
        }
        else
        {
            Error = true;
        }
        
        RandomSeed = atoll(SeedString);
        
        if(RandomSeed == 0)
        {
            if(SeedString[0] == '0')
            {
                RandomSeed = 0;
            }
            else
            {
                Error = true;
            }
        }
        
        PairCount = atoll(PairCountString);
        if(PairCount == 0)
        {
            Error = true;
        }
        
        if(!Error)
        {
            printf("Method: %s\n"
                   "Random seed: %lu\n"
                   "Pairs count: %lu\n"
                   , MethodString, RandomSeed, PairCount);
            
            umm JsonMemorySize = Gigabytes(4);
            u8 *JsonMemory = (u8 *)mmap(0, JsonMemorySize, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0);
            u8 *JsonOut = JsonMemory;
            
            umm BinMemorySize = Gigabytes(4);
            u8 *BinMemory = (u8 *)mmap(0, BinMemorySize, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0);
            u8 *BinOut = BinMemory;
            
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
            
            char *JsonHeader = 
                "{\n"
                " \"pairs\":\n"
                "   [\n";
            char *JsonFooter =
                "   ]\n"
                "}\n";
            
            JsonOut += stbsp_sprintf((char *)JsonOut, "%s", JsonHeader);
            
            pcg64_random_t RNG = {};
            pcg64_srandom_r(&RNG, RandomSeed, RandomSeed);
            
            if(0) {}
            else if(Method == Method_Uniform)
            {
                f64 AverageSum = 0;
                f64 TotalSum = 0;
                for(u64 PairsIndex = 0;
                    PairsIndex < PairCount;
                    PairsIndex += 1)
                {
                    f64 X0 = RandomBetween(&RNG, -180.0, 180.0);
                    f64 Y0 = RandomBetween(&RNG, -90.0, 90.0);
                    f64 X1 = RandomBetween(&RNG, -180.0, 180.0); 
                    f64 Y1 = RandomBetween(&RNG, -90.0, 360.0);
                    
                    f64 Sum = ReferenceHaversine(X0, Y0, X1, Y1, 6372.8);
                    TotalSum += Sum;
                    
                    *(f64 *)BinOut = Sum;
                    BinOut += sizeof(Sum);
                    
                    JsonOut += stbsp_sprintf((char *)JsonOut, "    " PointJsonFormat, X0, Y0, X1, Y1);
                }
                AverageSum = TotalSum / (f64)PairCount;
                
                *(f64 *)BinOut = AverageSum;
                BinOut += sizeof(AverageSum);
                
                printf("Average sum: %f\n", AverageSum);
            }
            else if(Method == Method_Cluster)
            {
                cluster Clusters[ClusterCount] = {};
                for(u32 ClusterIndex = 0;
                    ClusterIndex < ClusterCount;
                    ClusterIndex += 1)
                {
                    cluster *ClusterAt = Clusters + ClusterIndex;
                    ClusterAt->X = RandomBetween(&RNG, -180.0, 180.0);
                    ClusterAt->Y = RandomBetween(&RNG, -90.0, 90.0);
                    ClusterAt->Width = RandomBetween(&RNG, 0.0, 180.0);
                    ClusterAt->Height = RandomBetween(&RNG, 0.0, 90.0);
                }
                
                f64 AverageSum = 0;
                f64 TotalSum = 0;
                u32 ClusterIndex = 0;
                for(u32 PairIndex = 0;
                    PairIndex < PairCount;
                    PairIndex += 1)
                {
                    cluster *ClusterAt = Clusters + ClusterIndex;
                    
                    f64 X0 = RandomBetween(&RNG, ClusterAt->X - ClusterAt->Width, ClusterAt->X + ClusterAt->Width);
                    f64 Y0 = RandomBetween(&RNG, ClusterAt->Y - ClusterAt->Height, ClusterAt->Y + ClusterAt->Height);
                    f64 X1 = RandomBetween(&RNG, ClusterAt->X - ClusterAt->Width, ClusterAt->X + ClusterAt->Width);
                    f64 Y1 = RandomBetween(&RNG, ClusterAt->Y - ClusterAt->Height, ClusterAt->Y + ClusterAt->Height);
                    
                    f64 Sum = ReferenceHaversine(X0, Y0, X1, Y1, 6372.8);
                    TotalSum += Sum;
                    
                    *(f64 *)BinOut = Sum;
                    BinOut += sizeof(Sum);
                    
                    JsonOut += stbsp_sprintf((char *)JsonOut, "    " PointJsonFormat, X0, Y0, X1, Y1);
                    
                    ClusterIndex += 1;
                    if(ClusterIndex == ClusterCount)
                    {
                        ClusterIndex -= ClusterCount;
                    }
                    
                }
                AverageSum = TotalSum / (f64)PairCount;
                
                *(f64 *)BinOut = AverageSum;
                BinOut += sizeof(AverageSum);
                
                printf("Average sum: %f\n", AverageSum);
            }
            else
            {
                Assert(0);
            }
            
            JsonOut += stbsp_sprintf((char *)JsonOut, "%s", JsonFooter);
            
            // Write memory to json file
            {            
                char JsonFileName[256] = {};
                stbsp_sprintf(JsonFileName, "data_%lu.json", PairCount);
                
                int File = open(JsonFileName, O_RDWR|O_CREAT|O_TRUNC, 0600);
                Assert(File != -1);
                smm Result = write(File, JsonMemory, JsonOut - JsonMemory);
                Assert(Result == JsonOut - JsonMemory);
            }
            
            // Write memory to binary answer file
            char BinFileName[256] = {};
            {
                stbsp_sprintf(BinFileName, "data_%lu_haveranswer.f64", PairCount);
                int File = open(BinFileName, O_RDWR|O_CREAT|O_TRUNC, 0600);
                Assert(File != -1);
                smm Result = write(File, BinMemory, BinOut - BinMemory);
                Assert(Result == BinOut - BinMemory);
            }
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