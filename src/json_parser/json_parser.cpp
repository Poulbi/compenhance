#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#if OS_WINDOWS
# include <windows.h>
# define RADDBG_MARKUP_IMPLEMENTATION
#else
# define RADDBG_MARKUP_STUBS
#endif

#include "raddbg_markup.h"
#include "lr/lr.h"

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
    JSONTokenType_None = 0,
    
    JSONTokenType_Object,
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

b32 IsDigit(u8 Char)
{
	b32 Result = ((Char >= '0') && (Char <= '9'));
	return Result;
}

json_token *GetJSONToken(str8 Buffer, u64 *Pos);

void GetJSONTokenList(str8 Buffer, json_token *Token, json_token_type Type, u64 *Pos, u8 EndChar)
{
    Token->Type = Type;
    *Pos += 1;
    json_token *Child = 0;
    while(*Pos < Buffer.Size && Buffer.Data[*Pos] != EndChar)
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
        while(*Pos < Buffer.Size && IsWhiteSpace(Buffer.Data[*Pos])) *Pos += 1;
    }
}

json_token *GetJSONToken(str8 Buffer, u64 *Pos)
{
    json_token *Token = (json_token *)malloc(sizeof(json_token));
    Assert(Token);
    *Token = {};
    
    if(*Pos < Buffer.Size)
    {
        while(*Pos < Buffer.Size && IsWhiteSpace(Buffer.Data[*Pos])) *Pos += 1;
        Assert(*Pos < Buffer.Size);
        
        u64 TokenStart = *Pos;
        
        str8 Keyword = {};
        
        switch(Buffer.Data[*Pos])
        {
            case '{': GetJSONTokenList(Buffer, Token, JSONTokenType_Object, Pos, '}'); break;
            case '[': GetJSONTokenList(Buffer, Token, JSONTokenType_Array,  Pos, ']'); break;
            
            case ':': Token->Type = JSONTokenType_Colon; break;
            case ',': Token->Type = JSONTokenType_Comma; break;
            
            case 't': Keyword = S8Lit("true");  Token->Type = JSONTokenType_True;  break;
            case 'f': Keyword = S8Lit("false"); Token->Type = JSONTokenType_False; break;
            case 'n': Keyword = S8Lit("null");  Token->Type = JSONTokenType_Null;  break;
            
            case '"':
            {
                Token->Type = JSONTokenType_String;
                
                u64 At = *Pos;
                Token->Value.Data = Buffer.Data + At;
                while(*Pos < Buffer.Size)
                {
                    *Pos += 1;
                    if(Buffer.Data[*Pos] == '\\') continue;
                    if(Buffer.Data[*Pos] == '"')  break;
                }
                
                if(*Pos >= Buffer.Size)
                {
                    Token->Type = JSONTokenType_Error;
                    Token->ErrorAt = At;
                }
            } break;
            
            default:
            {
                if(Buffer.Data[*Pos] == '-' ||
                   (Buffer.Data[*Pos] >= '0' && Buffer.Data[*Pos] <= '9'))
                {
                    Token->Type = JSONTokenType_Number;
                    
                    f64 Number = 0;
                    
                    if(Buffer.Data[*Pos] == '-')
                    {
                        *Pos += 1;
                    }
                    
                    while(*Pos < Buffer.Size)
                    {
                        if(0) {}
                        else if(IsDigit(Buffer.Data[*Pos]))
                        {
                        }
                        else if(Buffer.Data[*Pos] == '.')
                        {
                            *Pos += 1;
                            while(*Pos < Buffer.Size && IsDigit(Buffer.Data[*Pos])) *Pos += 1;
                            break;
                        }
                        
                        *Pos += 1;
                    }
                    
                    if(Buffer.Data[*Pos] == 'e' || Buffer.Data[*Pos] == 'E')
                    {
                        *Pos += 1;
                        
                        if(Buffer.Data[*Pos] == '+')
                        {
                            *Pos += 1;
                        }
                        else if(Buffer.Data[*Pos] == '-')
                        {
                            *Pos += 1;
                        }
                        
                        while(*Pos < Buffer.Size && IsDigit(Buffer.Data[*Pos]))
                        {
                            *Pos += 1;
                        }
                        
                    }
                    
                    *Pos -= 1;
                    
                }
            } break;
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
    }
    
    return Token;
}

json_token *JSONLookupIdentifierValue(json_token *Token, str8 Identifier)
{
    json_token *Found = 0;
    
    for(json_token *Search = Token->Child;
        Search;
        Search = Search->Next)
    {
        if(MatchString(Search->Value, Identifier, 0))
        {
            Found = Search;
            break;
        }
        
        Assert(Search->Next->Type == JSONTokenType_Colon);
        Assert(Search->Next->Next->Type == JSONTokenType_Number);
        Search = Search->Next->Next;
        
        if(Search->Next)
        {
            Assert(Search->Next->Type == JSONTokenType_Comma);
            Search = Search->Next;
        }
    }
    
    if(Found)
    {
        Assert(Found->Next->Type == JSONTokenType_Colon);
        Found = Found->Next->Next;
    }
    
    return Found;
}

int main(int ArgsCount, char *Args[])
{
    u64 MaxPairCount = 100000000;
    
    str8 In = ReadEntireFileIntoMemory(Args[1]);
    u64 Pos = 0;
    json_token *JSON = GetJSONToken(In, &Pos);
    
    if(JSON->Type != JSONTokenType_None)
    {
        
        if(MatchString(JSON->Child->Value, S8Lit("\"pairs\""), 0))
        {
            json_token *Pairs = JSON->Child;
            // get object pair value
            
            Assert(Pairs->Type == JSONTokenType_String);
            Assert(Pairs->Next->Type == JSONTokenType_Colon);
            Assert(Pairs->Next->Next->Type == JSONTokenType_Array);
            
            json_token *PairsArray = Pairs->Next->Next;
            
            for(json_token *Pair = PairsArray->Child;
                Pair;
                Pair = Pair->Next)
            {
                LogFormat("%.*s\n", Pair->Value.Size, Pair->Value.Data);
                
                json_token *X0 = JSONLookupIdentifierValue(Pair, S8Lit("\"x0\""));
                // TODO(luca): Convert to floating point number.
                f64 X0Value = 0;
                
                json_token *Y0 = JSONLookupIdentifierValue(Pair, S8Lit("\"y0\""));
                json_token *X1 = JSONLookupIdentifierValue(Pair, S8Lit("\"x1\""));
                json_token *Y1 = JSONLookupIdentifierValue(Pair, S8Lit("\"y1\""));
                
                if(Pair->Next)
                {
                    Assert(Pair->Next->Type == JSONTokenType_Comma);
                    Pair = Pair->Next;
                }
            }
            
        }
        
    }
    
    return 0;
}