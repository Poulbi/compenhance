#include "libs/lr/lr.h"
#define STB_SPRINTF_IMPLEMENTATION
#include "libs/stb_sprintf.h"

#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

//~ Types
struct str8
{
    u8 *Data;
    umm Size;
};
#define S8Lit(String) (str8){.Data = (u8 *)(String), .Size = (sizeof((String)) - 1)}

//~ Globals
global_variable u8 LogBuffer[Kilobytes(64)];

//~ Functions
//- Debug utilities 
void AssertErrnoNotEquals(smm Result, smm ErrorValue)
{
    if(Result == ErrorValue)
    {
        int Errno = errno;
        Assert(0);
    }
}

void AssertErrnoEquals(smm Result, smm ErrorValue)
{
    if(Result != ErrorValue)
    {
        int Errno = errno;
        Assert(0);
    }
}

void LogFormat(char *Format, ...)
{
    va_list Args;
    va_start(Args, Format);
    
    int Length = stbsp_vsprintf((char *)LogBuffer, Format, Args);
    
    smm BytesWritten = write(STDOUT_FILENO, LogBuffer, Length);
    AssertErrnoEquals(BytesWritten, Length);
}

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
    At += 1;
    
    if(At >= Size || In[At] != Char)
    {
        Assert(0 && "Expected Char");
    }
    
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

void ParseNumber(umm Size, u8 *In, umm *Start, f64 *Value)
{
    umm At = *Start;
    
    while(At < Size && In[At] != '.') 
    {
        Assert(In[At] >= '0' && In[At] <= '9');
        
        At += 1;
    }
    Assert(In[At] == '.');
    At += 1;
    
    while(At < Size && !IsWhiteSpace(In[At]))
    {
        Assert(In[At] >= '0' && In[At] <= '9');
        
        At += 1;
    }
    
    *Start = At;
}


//-

int main(int ArgsCount, char *Args[])
{
    
    char *JsonFileName = 0;
    char *AnswersFileName = 0;
    
    
    if(ArgsCount >= 2)
    {
        JsonFileName = Args[1];
    }
    else
    {
        LogFormat("usage: %s <file.json> [answer.f64]\n", Args[0]);
    }
    
    if(ArgsCount >= 3)
    {
        AnswersFileName = Args[1];
    }
    
    if(JsonFileName)
    {
        int JsonFile = open(JsonFileName, O_RDONLY);
        if(JsonFile != -1)
        {
            struct stat StatBuffer = {};
            int Result = fstat(JsonFile, &StatBuffer);
            AssertErrnoNotEquals(Result, -1);
            
            umm FileSize = StatBuffer.st_size;
            u8 *JsonMemory = (u8 *)mmap(0, FileSize, PROT_READ, MAP_PRIVATE, JsonFile, 0);
            AssertErrnoNotEquals((smm)JsonMemory, (smm)MAP_FAILED);
            
            f64 X0, X1, Y0, Y1;
            
            // Json Parsing
            u8 *In = JsonMemory;
            for(umm At = 0;
                At < FileSize;
                At += 1)
            {
                ConsumeWhiteSpacePastChar(FileSize, In, &At, '{');
                ConsumePastJsonString(FileSize, In, &At, S8Lit("pairs"));
                ConsumeWhiteSpacePastChar(FileSize, In, &At, ':');
                ConsumeWhiteSpacePastChar(FileSize, In, &At, '[');
                
                // One pair
                {                
                    ConsumeWhiteSpacePastChar(FileSize, In, &At, '{');
                    
                    ConsumePastJsonString(FileSize, In, &At, S8Lit("x0"));
                    ConsumeWhiteSpacePastChar(FileSize, In, &At, ':');
                    while(At < FileSize && IsWhiteSpace(In[At])) At += 1;
                    At += 1;
                    ParseNumber(FileSize, In, &At, &X0);
                    ConsumeWhiteSpacePastChar(FileSize, In, &At, ',');
                    
                    ConsumePastJsonString(FileSize, In, &At, S8Lit("y0"));
                    ConsumeWhiteSpacePastChar(FileSize, In, &At, ':');
                    while(At < FileSize && IsWhiteSpace(In[At])) At += 1;
                    At += 1;
                    ParseNumber(FileSize, In, &At, &Y0);
                    ConsumeWhiteSpacePastChar(FileSize, In, &At, ',');
                    
                    ConsumePastJsonString(FileSize, In, &At, S8Lit("x1"));
                    ConsumeWhiteSpacePastChar(FileSize, In, &At, ':');
                    while(At < FileSize && IsWhiteSpace(In[At])) At += 1;
                    At += 1;
                    ParseNumber(FileSize, In, &At, &X1);
                    ConsumeWhiteSpacePastChar(FileSize, In, &At, ',');
                    
                    ConsumePastJsonString(FileSize, In, &At, S8Lit("y1"));
                    ConsumeWhiteSpacePastChar(FileSize, In, &At, ':');
                    while(At < FileSize && IsWhiteSpace(In[At])) At += 1;
                    At += 1;
                    ParseNumber(FileSize, In, &At, &Y1);
                    
                    ConsumeWhiteSpacePastChar(FileSize, In, &At, '}');
                }
                
#if 1                
                break;
#endif
                
                ConsumeWhiteSpacePastChar(FileSize, In, &At, ']');
                ConsumeWhiteSpacePastChar(FileSize, In, &At, '}');
            }
            
        }
        
    }
    else
    {
        LogFormat("Error: File not found.\n"
                  "usage: %s <file.json> [answer.f64]\n", Args[0]);
    }
    
    
    return 0;
}