/* date = August 13th 2025 1:51 pm */

#ifndef SIM86_H
#define SIM86_H

#define internal static
#define global_variable static
#define local_persist static

typedef size_t psize;

#define Assert(Expression) if(!(Expression)) { __asm__ volatile("int3"); }
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

//~ Stolen from the decoder.
enum register_mapping_8086
{
    Register_none,
    
    Register_a,
    Register_b,
    Register_c,
    Register_d,
    Register_sp,
    Register_bp,
    Register_si,
    Register_di,
    Register_es,
    Register_cs,
    Register_ss,
    Register_ds,
    Register_ip,
    Register_flags,
    
    Register_count,
};


#endif //SIM86_H
