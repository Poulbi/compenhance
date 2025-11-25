
#include <stdint.h>
#include <stddef.h>

#if OS_WINDOWS
# include <windows.h>
#endif

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
    
    JSONTokenType_True,
    JSONTokenType_False,
    JSONTokenType_Null,
    
    JSONTokenType_Count,
};


struct json_token
{
    json_token_type Type;
    str8 Value;
};

struct haversine_pair
{
    f64 X0, Y0, X1, Y1;
};

b32 IsWhiteSpace(u8 Char)
{
    b32 Result = ((Char == ' ') || (Char == '\t') || (Char == '\n') || (Char == '\r'));
    return Result;
}


json_token *ParseJSONElement(str8 Buffer, u64 Pos)
{
    json_token *Result = 0;
    Assert(Pos < Buffer.Size);
    
    while(Pos < Buffer.Size && IsWhiteSpace(Buffer.Data[Pos])) Pos += 1;
    Assert(Pos < Buffer.Size);
    
    switch(Buffer.Data[Pos])
    {
        case '{':
        {
            
        } break;
        case '[':
        {
            
        } break;
        
        case ':':
        {
            
        } break;
        case ',':
        {
            
        } break;
        
        // true, false, null
        case 't':
        {
            
        } break;
        case 'f':
        {
            
        } break;
        case 'n':
        {
            
        } break;
        
        default:
        {
            Assert(0 && "Syntax error(?)");
        } break;
    }
    
    return Result;
}

json_token *ParseJSONList(str8 Buffer, u64 Pos)
{
    json_token *Result = 0;
    
    return Result;
}

json_token *ParseJSON(str8 Buffer)
{
    json_token *Result = ParseJSONElement(Buffer, 0);
    return Result;
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
    json_token *JSON = ParseJSON(In);
    
    free(Pairs);
    
    return 0;
}