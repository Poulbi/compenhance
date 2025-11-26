
#include <stdint.h>
#include <stddef.h>

#if OS_WINDOWS
# include <windows.h>
# define RADDBG_MARKUP_IMPLEMENTATION
#else
# define RADDBG_MARKUP_STUBS
#endif

#include "libs/raddbg_markup.h"

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;
typedef size_t umm;
typedef int32_t s32;
typedef s32 b32;

typedef float f32;
typedef double f64;

struct str8
{
    u8 *Data;
    u64 Size;
};
raddbg_type_view(str8, no_addr(array((char *)Data, Size)));
#define S8Lit(String) (str8){.Data = (u8 *)(String), .Size = (sizeof((String)) - 1)}

#define Assert(Expression) if(!(Expression)) { *(int *)0 = 0; }

#include "os.c"

// {
//  "pairs":
//  [
//   { 
//    "x0": 118.9680008758136864, "y0": 63.6727798452497993, 
//    "x1": 96.0760594261872427, "y1": // 76.3004991664638509
//   }
//  ]
// }

enum json_token_type
{
    JSONTokenType_Object = 0,
    JSONTokenType_Array,
    JSONTokenType_Number,
    JSONTokenType_String,
    JSONTokenType_Colon,
    JSONTokenType_Comma,
    // keywords
    JSONTokenType_True,
    JSONTokenType_False,
    JSONTokenType_Null,
    
    JSONTokenType_Error,
    
    JSONTokenType_Count,
};


struct json_token
{
    json_token_type Type;
    str8 Value;
    f64 Number;
    
    u64 ErrorAt;
    
    json_token *Child;
    json_token *Next;
};

struct haversine_pair
{
    f64 X0, Y0, X1, Y1;
};

b32 MatchString(str8 Buffer, str8 Match, u64 Offset)
{
    b32 Result = true;
    
    Assert(Offset < Buffer.Size);
    Buffer.Data += Offset;
    Buffer.Size -= Offset;
    
    if(Buffer.Size < Match.Size)
    {
        return false;
    }
    
    for(u64 At = 0;
        At < Match.Size;
        At += 1)
    {
        if(Buffer.Data[At] != Match.Data[At])
        {
            Result = false;
            break;
        }
    }
    
    return Result;
}

b32 IsWhiteSpace(u8 Char)
{
    b32 Result = ((Char == ' ') || (Char == '\t') || (Char == '\n') || (Char == '\r'));
    return Result;
}

json_token *GetJSONToken(str8 Buffer, u64 *Pos)
{
    json_token *Token = (json_token *)malloc(sizeof(json_token));
    *Token = {};
    
    Assert(*Pos < Buffer.Size);
    
    while(*Pos < Buffer.Size && IsWhiteSpace(Buffer.Data[*Pos])) *Pos += 1;
    Assert(*Pos < Buffer.Size);
    
    u64 TokenStart = *Pos;
    
    str8 Keyword = {};
    
    switch(Buffer.Data[*Pos])
    {
        case '{':
        {
            Token->Type = JSONTokenType_Object;
            *Pos += 1;
            json_token *Child = 0;
            while(*Pos < Buffer.Size && Buffer.Data[*Pos] != '}')
            {
                if(!Token->Child)
                {
                    Child = GetJSONToken(Buffer, Pos);
                    Token->Child = Child;
                }
                else
                {
                    Child->Next = GetJSONToken(Buffer, Pos);
                    Child = Child->Next;
                }
            }
            
        } break;
        
        case '[':
        {
            Token->Type = JSONTokenType_Array;
            // TODO: Parse list until ']'
        } break;
        
        case '"':
        {
            Token->Type = JSONTokenType_String;
            
            u64 At = *Pos;
            Token->Value.Data = Buffer.Data + At;
            while(*Pos < Buffer.Size)
            {
                *Pos += 1;
                if(Buffer.Data[*Pos] == '\\') continue;
                if(Buffer.Data[*Pos] == '"') break;
            }
            
            if(*Pos >= Buffer.Size)
            {
                Token->Type = JSONTokenType_Error;
                Token->ErrorAt = At;
            }
            
        } break;
        
        case ':': Token->Type = JSONTokenType_Colon; break;
        case ',': Token->Type = JSONTokenType_Comma; break;
        case 't': Keyword = S8Lit("true");  Token->Type = JSONTokenType_True;  break;
        case 'f': Keyword = S8Lit("false"); Token->Type = JSONTokenType_False; break;
        case 'n': Keyword = S8Lit("null");  Token->Type = JSONTokenType_Null;  break;
    }
    
    if(Buffer.Data[*Pos] == '-' ||
       (Buffer.Data[*Pos] >= '0' &&
        Buffer.Data[*Pos] <= '9'))
    {
        Token->Type = JSONTokenType_Number;
        
        f64 Number;
        
        if(Buffer.Data[*Pos] == '-')
        {
            *Pos += 1;
        }
        
    }
    
    if(Keyword.Size)
    {
        if(MatchString(Buffer, Keyword, *Pos))
        {
            *Pos += Keyword.Size - 1;
        }
        else
        {
            Token->Type = JSONTokenType_Error;
            Token->ErrorAt = *Pos;
        }
    }
    
    *Pos += 1;
    
    Token->Value.Data = Buffer.Data + TokenStart;
    Token->Value.Size = *Pos - TokenStart;
    
    return Token;
}

u64 ParseHaversinePairs(str8 In, u64 MaxPairCount, haversine_pair *Pairs)
{
    u64 Result = 0;
    
    return Result;
}

int main(void)
{
    u64 MaxPairCount = 100000000;
    
    haversine_pair *Pairs = (haversine_pair *)malloc(sizeof(haversine_pair)*1000);
    
    str8 In= S8Lit("{\"count\": 3}");
    
    u64 Pos = 0;
    json_token *JSON = GetJSONToken(In, &Pos);
    
    free(Pairs);
    
    return 0;
}