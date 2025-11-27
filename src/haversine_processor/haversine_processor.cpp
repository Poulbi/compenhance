#include "../lib/lr/lr.h"
#include "../lib/listing_0070_platform_metrics.cpp"

#include <stdio.h>
#include <math.h>

//~ Types
struct str8
{
    u8 *Data;
    umm Size;
};
#define S8Lit(String) (str8){.Data = (u8 *)(String), .Size = (sizeof((String)) - 1)}

//~ Functions

static void PrintTimeElapsed(char const *Label, u64 TotalTSCElapsed, u64 Begin, u64 End)
{
    u64 Elapsed = End - Begin;
    f64 Percent = 100.0 * ((f64)Elapsed / (f64)TotalTSCElapsed);
    printf("  %s: %llu (%.2f%%)\n", Label, Elapsed, Percent);
}

u64 GuessCPUFreq(u64 OSElapsed, u64 CPUElapsed, u64 OSFreq)
{
    u64 CPUFreq = 0;
    if(OSElapsed)
	{
		CPUFreq = OSFreq * CPUElapsed / OSElapsed;
	}
	
    return CPUFreq;
}

u64 CPUElapsedSince(u64 Start)
{
    u64 Result = ReadCPUTimer() - Start;
    return Result;
}

//- Haversine 
static f64 Square(f64 A)
{
    f64 Result = (A*A);
    return Result;
}

static f64 RadiansFromDegrees(f64 Degrees)
{
    f64 Result = 0.01745329251994329577 * Degrees;
    return Result;
}

// NOTE(casey): EarthRadius is generally expected to be 6372.8
static f64 ReferenceHaversine(f64 X0, f64 Y0, f64 X1, f64 Y1, f64 EarthRadius)
{
    /* NOTE(casey): This is not meant to be a "good" way to calculate the Haversine distance.
       Instead, it attempts to follow, as closely as possible, the formula used in the real-world
       question on which these homework exercises are loosely based.
    */
    
    f64 lat1 = Y0;
    f64 lat2 = Y1;
    f64 lon1 = X0;
    f64 lon2 = X1;
    
    f64 dLat = RadiansFromDegrees(lat2 - lat1);
    f64 dLon = RadiansFromDegrees(lon2 - lon1);
    lat1 = RadiansFromDegrees(lat1);
    lat2 = RadiansFromDegrees(lat2);
    
    f64 a = Square(sin(dLat/2.0)) + cos(lat1)*cos(lat2)*Square(sin(dLon/2));
    f64 c = 2.0*asin(sqrt(a));
    
    f64 Result = EarthRadius * c;
    
    return Result;
}

#include "os.c"

//- Parsing utilities 

b32 IsWhiteSpace(u8 Char)
{
    b32 Result = (Char == ' ' || Char == '\t' || Char == '\n');
    return Result;
}

void ConsumeWhiteSpacePastChar(umm Size, u8 *In, umm *Start, u8 Char)
{
    umm At = *Start;
    
    while(At < Size && IsWhiteSpace(In[At])) At += 1;
    
    if(At >= Size || In[At] != Char)
    {
        Assert(0 && "Expected Char");
    }
    At += 1;
    
    *Start = At;
}

void ConsumePastJsonString(umm Size, u8 *In, umm *Start, str8 String)
{
    umm At = *Start;
    
    ConsumeWhiteSpacePastChar(Size, In, &At, '"');
    
    if(At + String.Size < Size)
    {
        b32 Match = true;
        for(umm MatchIndex = 0;
            MatchIndex < String.Size;
            MatchIndex += 1)
        {
            if(In[At + MatchIndex] != String.Data[MatchIndex])
            {
                Match = false;
                break;
            }
        }
        
        if(Match)
        {
            At += String.Size;
            
            if(In[At] == '"')
            {
                At += 1;
            }
            else
            {
                Assert(In[At] == '"');
            }
        }
        else
        {
            Assert(Match && "Expected String");
        }
        
    }
    else
    {
        Assert(0 && "Expected '\"'");
    }
    
    *Start = At;
}

struct parse_number_result
{
    f64 Value;
    umm At;
};

void ParseFloatNumber(umm Size, u8 *In, umm *Start, f64 *Value)
{
    umm At = *Start;
    
    b32 Negative = false;
    f64 Integer = 0;
    
    if(In[At] == '-')
    {
        At += 1;
        Negative = true;
    }
    
    while(At < Size && (In[At] >= '0' && In[At] <= '9')) 
    {
        Integer = 10*Integer + (In[At] - '0');
        At += 1;
    }
    Assert(In[At] == '.');
    At += 1;
    
    *Value = Integer;
    
    f64 Divider = 10;
    while(At < Size && (In[At] >= '0' && In[At] <= '9'))
    {
        f64 Digit = (f64)(In[At] - '0');
        
        *Value += (Digit / Divider);
        Divider *= 10;
        
        At += 1;
    }
    
    if(Negative)
    {
        *Value = -*Value;
    }
    
    *Start = At;
}

void ConsumeHaversineJsonNumber(umm Size, u8 *In, umm *Start, str8 Name, f64 *Value, b32 NumberLeft)
{
    umm At = *Start;
    
    ConsumePastJsonString(Size, In, &At, Name);
    ConsumeWhiteSpacePastChar(Size, In, &At, ':');
    while(At < Size && IsWhiteSpace(In[At])) At += 1;
    ParseFloatNumber(Size, In, &At, Value);
    
    if(NumberLeft)
    {
        ConsumeWhiteSpacePastChar(Size, In, &At, ',');
    }
    
    *Start = At;
}

//-

int main(int ArgsCount, char *Args[])
{
    str8 Answers = {};
    umm AnswerIndex = 0;
    str8 Json = {};
    
    u64 OSFreq = GetOSTimerFreq();
    u64 OSStart = ReadOSTimer();
    u64 CPUStart = ReadCPUTimer();
    
    u64 Prof_Begin = 0;
    u64 Prof_End = 0;
    u64 Prof_Startup = 0;
    u64 Prof_Read = 0;
    u64 Prof_MiscSetup= 0;
    u64 Prof_Parse = 0;
    u64 Prof_Sum = 0;
    u64 Prof_MiscOutput= 0;
    
    Prof_Begin = ReadCPUTimer();
    
    Prof_Read = ReadCPUTimer();
    
    if(ArgsCount >= 2)
    {
        Json = ReadEntireFileIntoMemory(Args[1]);
    }
    else
    {
        LogFormat("usage: %s <file.json> [answer.f64]\n", Args[0]);
    }
    
    if(ArgsCount >= 3)
    {
        Answers = ReadEntireFileIntoMemory(Args[2]);
    }
    
    Prof_MiscSetup = ReadCPUTimer();
    
    if(Json.Size)
    {
        f64 X0 = 0.0;
        f64 X1 = 0.0;
        f64 Y0 = 0.0;
        f64 Y1 = 0.0;
        
        // Json Parsing
        u8 *In = Json.Data;
        
        umm At = 0;
        ConsumeWhiteSpacePastChar(Json.Size, In, &At, '{');
        ConsumePastJsonString(Json.Size, In, &At, S8Lit("pairs"));
        ConsumeWhiteSpacePastChar(Json.Size, In, &At, ':');
        ConsumeWhiteSpacePastChar(Json.Size, In, &At, '[');
        
        f64 TotalSum = 0;
        umm PairCount = 0;
        b32 PairsRemaining = true;
        
        Prof_Parse = ReadCPUTimer();
        
        // One pair
        while(PairsRemaining && (At < Json.Size))
        {                
            ConsumeWhiteSpacePastChar(Json.Size, In, &At, '{');
            
            ConsumeHaversineJsonNumber(Json.Size, In, &At, S8Lit("x0"), &X0, true);
            ConsumeHaversineJsonNumber(Json.Size, In, &At, S8Lit("y0"), &Y0, true);
            ConsumeHaversineJsonNumber(Json.Size, In, &At, S8Lit("x1"), &X1, true);
            ConsumeHaversineJsonNumber(Json.Size, In, &At, S8Lit("y1"), &Y1, false);
            
            f64 Sum = ReferenceHaversine(X0, Y0, X1, Y1, 6372.8);
            TotalSum += Sum;
            
            ConsumeWhiteSpacePastChar(Json.Size, In, &At, '}');
            
            if(Answers.Size)
            {
                // NOTE(luca): What to do here?
                //f64 AnswerSum = ((f64 *)Answers.Data)[AnswerIndex];
                
                AnswerIndex += 1;
            }
            
            if(At < Json.Size && In[At] == ',')
            {
                At += 1;
            }
            
            while(At < Json.Size && IsWhiteSpace(In[At])) At += 1;
            PairsRemaining = (In[At] == '{' || At >= Json.Size);
            
            PairCount += 1;
        }
        
        ConsumeWhiteSpacePastChar(Json.Size, In, &At, ']');
        ConsumeWhiteSpacePastChar(Json.Size, In, &At, '}');
        
        Prof_Sum = ReadCPUTimer();
        
        u64 MiscOutputStart = ReadCPUTimer();
        f64 AverageSum = TotalSum / (f64)PairCount;
        
        Prof_MiscOutput = ReadCPUTimer();
        LogFormat("Input size: %lu\n"
                  "Pair count: %lu\n"
                  "Haversine sum: %.16f\n"
                  , Json.Size, PairCount, AverageSum);
        
        if(Answers.Size)
        {
            f64 AnswerAverageSum = ((f64 *)Answers.Data)[AnswerIndex];
            AnswerIndex += 1;
            
            Assert((AnswerIndex * sizeof(f64)) == Answers.Size);
            
            LogFormat("\nValidation\n"
                      "Reference sum: %.16f\n"
                      "Difference: %.16f\n"
                      , AnswerAverageSum, AnswerAverageSum - AverageSum);
            
            
        }
        
    }
    else
    {
        LogFormat("Error: File not found.\n"
                  "usage: %s <file.json> [answer.f64]\n", Args[0]);
    }
    
    Prof_End = ReadCPUTimer();
    
    u64 OSElapsed = ReadOSTimer() - OSStart;
    u64 CPUElapsed = CPUElapsedSince(Prof_Begin);
    u64 CPUFreq = GuessCPUFreq(OSElapsed, CPUElapsed, OSFreq);
    
    printf("\n");
    printf("Elapsed: %llu\n", CPUElapsed);
    printf("Total time: %.4fms (CPU freq %llu)\n", (f64)OSElapsed/(f64)OSFreq, CPUFreq);
    
    PrintTimeElapsed("Startup", CPUElapsed, Prof_Begin, Prof_Read);
    PrintTimeElapsed("Read", CPUElapsed, Prof_Read, Prof_MiscSetup);
    PrintTimeElapsed("MiscSetup", CPUElapsed, Prof_MiscSetup, Prof_Parse);
    PrintTimeElapsed("Parse", CPUElapsed, Prof_Parse, Prof_Sum);
    PrintTimeElapsed("Sum", CPUElapsed, Prof_Sum, Prof_MiscOutput);
    PrintTimeElapsed("MiscOutput", CPUElapsed, Prof_MiscOutput, Prof_End);
    
    return 0;
}
