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

enum flags_8086
{
    Flag_None           = 0,
    Flag_Carry          =(1 << 0),
    Flag_Parity         =(1 << 1),
    Flag_AuxiliaryCarry =(1 << 2),
    Flag_Zero           =(1 << 3),
    Flag_Sign           =(1 << 4),
    Flag_Overflow       =(1 << 5),
    Flag_Interrupt      =(1 << 6),
    Flag_Direction      =(1 << 7),
    Flag_Trap           =(1 << 8),
    Flag_Count          = 11,
};

#endif //SIM86_H
