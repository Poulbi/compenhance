/* date = November 28th 2025 1:49 am */

#ifndef JSON_PARSER_H
#define JSON_PARSER_H

struct str8
{
    u8 *Data;
    u64 Size;
};
typedef struct str8 str8;
raddbg_type_view(str8, no_addr(array((char *)Data, Size)));
#define S8Lit(String) (str8){.Data = (u8 *)(String), .Size = (sizeof((String)) - 1)}

struct haversine_pair
{
    f64 X0, Y0, X1, Y1;
};

#endif //JSON_PARSER_H
