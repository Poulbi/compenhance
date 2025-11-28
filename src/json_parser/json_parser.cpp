//~ Libraries
//- Standard 
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h> // for haversine
//- External  
#if OS_WINDOWS
# include <windows.h>
# define RADDBG_MARKUP_IMPLEMENTATION
#else
# define RADDBG_MARKUP_STUBS
#endif
#include "raddbg_markup.h"

#include "lr/lr.h"
//- Project 
#include "listing_0065_haversine_formula.cpp"
#include "json_parser.h"
#include "json.cpp"
#include "os.c"

//- Main 

int main(int ArgsCount, char *Args[])
{
    u64 MaxPairCount = 100000000;
    
    str8 In = ReadEntireFileIntoMemory(Args[1]);
    u64 Pos = 0;
    json_token *JSON = JSON_ParseElement(In, &Pos);
    json_token *PairsArray = JSON_LookupIdentifierValue(JSON, S8Lit("\"pairs\""));
    
    u64 PairsCount = 0;
    
    // 30 bytes(characters) is the minimum for one pair
    u64 MaxPairsCount = In.Size/30;
    haversine_pair *HaversinePairs = (haversine_pair *)malloc(sizeof(haversine_pair)*MaxPairsCount);
    
    // Parse haversine pairs
    {            
        for(json_token *Pair = PairsArray->Child;
            Pair;
            Pair = Pair->Next)
        {
            haversine_pair *HaversinePair = HaversinePairs + PairsCount;
            
            HaversinePair->X0 = JSON_StringToFloat(JSON_LookupIdentifierValue(Pair, S8Lit("\"x0\""))->Value);
            HaversinePair->Y0 = JSON_StringToFloat(JSON_LookupIdentifierValue(Pair, S8Lit("\"y0\""))->Value);
            HaversinePair->X1 = JSON_StringToFloat(JSON_LookupIdentifierValue(Pair, S8Lit("\"x1\""))->Value);
            HaversinePair->Y1 = JSON_StringToFloat(JSON_LookupIdentifierValue(Pair, S8Lit("\"y1\""))->Value);
            
            PairsCount += 1;
            
            if(Pair->Next)
            {
                Assert(Pair->Next->Type == JSON_TokenType_Comma);
                Pair = Pair->Next;
            }
        }
    }
    
    // Compute the sum
    {
        f64 AverageSum = 0;
        f64 AverageCoefficient = (1.0/(f64)PairsCount);
        
        for(u64 PairIndex = 0;
            PairIndex < PairsCount;
            PairIndex += 1)
        {
            haversine_pair *HaversinePair = HaversinePairs + PairIndex;
            
            f64 EarthRadius = 6372.8;
            f64 Sum = ReferenceHaversine(HaversinePair->X0, HaversinePair->Y0, 
                                         HaversinePair->X1, HaversinePair->Y1,
                                         EarthRadius);
            AverageSum += AverageCoefficient*Sum;
        }
        
        LogFormat("Average %.16f\n", AverageSum);
    }
    
    return 0;
}