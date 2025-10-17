#include <stdio.h>
#include <string.h>

#include "sim86.h"
#include "sim86_shared.h"

global_variable u8 FlagToCharMapping[] = "CPAZSOIDT";

internal void
SetOrUnsetFlag(u32 *FlagsRegister, b32 Condition, flags_8086 Flag)
{
    if(Condition)
    {
        *FlagsRegister |= Flag;
    }
    else
    {
        *FlagsRegister &= (~Flag);
    }
}

internal u32
FlagsToString(char *Buffer, u32 Flags)
{
    u32 Length = 0;
    
    for(u32 MappingIndex = 0;
        MappingIndex < ArrayCount(FlagToCharMapping);
        MappingIndex++)
    {
        u8 Char = FlagToCharMapping[MappingIndex];
        b32 IsBitSet = (Flags & 1);
        Flags >>= 1;
        if(IsBitSet)
        {
            Buffer[Length++] = Char;
        }
    }
    
    return Length;
}

internal void
FlagsFromValue(u32 *FlagsRegister, u32 InstructionFlags, s32 Value)
{
    u32 OldFlagsRegister = *FlagsRegister;
    
    u32 SignMask = (1 << ((InstructionFlags & Inst_Wide) ? 15 : 7));
    SetOrUnsetFlag(FlagsRegister, (Value & SignMask), Flag_Sign);
    SetOrUnsetFlag(FlagsRegister, (Value == 0),       Flag_Zero);
    
    // NOTE(luca): Parity flag is set when in the lower 8 bits the number of set bits is even.
    u32 OneBitsCount = 0;
    for(u32 BitsIndex = 0;
        BitsIndex < 8;
        BitsIndex++)
    {
        OneBitsCount += (Value & 1);
        Value >>= 1;
    }
    SetOrUnsetFlag(FlagsRegister, (!(OneBitsCount & 1)), Flag_Parity);
    
    // TODO(luca): Overflow flag
    // Were we adding positive numbers but produced a negative number?
    
    // TODO(luca): Carry flag 
    // When twot numbers are added and the 17th bit would be set?
    
    // TODO(luca): Auxiliary carry flag
    // Carry for bottom 8 bits
    if(*FlagsRegister != OldFlagsRegister)
    {
        char OldFlagsString[ArrayCount(FlagToCharMapping)] = {};
        char FlagsString[ArrayCount(FlagToCharMapping)] = {};
        FlagsToString(OldFlagsString, OldFlagsRegister);
        FlagsToString(FlagsString, *FlagsRegister);
        
        printf(" flags:%s->%s", OldFlagsString, FlagsString);
    }
}

struct operands_to_values_result
{
    s32 *Destination;
    s32 *Source;
};
internal operands_to_values_result
OperandsToValues(s32 *Registers,
                 instruction_operand *DestinationOperand, instruction_operand *SourceOperand)
{
    operands_to_values_result Result = {};
    
    if(0) {}
    else if(DestinationOperand->Type == Operand_Register)
    {
        Result.Destination = Registers + DestinationOperand->Register.Index;
    }
    else if(DestinationOperand->Type == Operand_Memory)
    {
        Assert(0 && "not implemented yet.");
    }
    else if(DestinationOperand->Type == Operand_Immediate)
    {
        Result.Destination = &DestinationOperand->Immediate.Value;
    }
    else if(SourceOperand->Type != Operand_None)
    {
        Assert(0);
    }
    
    if(0) {}
    else if(SourceOperand->Type == Operand_Register)
    {
        Result.Source = Registers + SourceOperand->Register.Index;
    }
    else if(SourceOperand->Type == Operand_Immediate)
    {
        Result.Source = &SourceOperand->Immediate.Value;
    }
    else if(SourceOperand->Type == Operand_Memory)
    {
        Assert(0 && "not implemented yet.");
    }
    else if(SourceOperand->Type != Operand_None)
    {
        Assert(0);
    }
    
    return Result;
}


internal void
Run8086(psize DisassemblySize, u8 *Disassembly)
{
    s32 Registers[Register_count] = {}; 
    u32 FlagsRegister = 0;
    u32 IPRegister = 0;
    
    while(IPRegister < DisassemblySize)
    {
        instruction Decoded;
        Sim86_Decode8086Instruction(DisassemblySize - IPRegister, Disassembly + IPRegister, &Decoded);
        if(Decoded.Op)
        {
            u32 OldIPRegister = IPRegister;
            IPRegister += Decoded.Size;
            
#if SIM86_INTERNAL           
            printf("Size:%u Op:%s Flags:0x%x ;", Decoded.Size, Sim86_MnemonicFromOperationType(Decoded.Op), Decoded.Flags);
#endif
            
            instruction_operand *DestinationOperand = Decoded.Operands;
            instruction_operand *SourceOperand = Decoded.Operands + 1;
            operands_to_values_result OperandsValues = OperandsToValues(Registers, DestinationOperand, SourceOperand);
            s32 *Destination = OperandsValues.Destination;
            s32 *Source = OperandsValues.Source;
            
            if(0) {}
            else if(Decoded.Op == Op_mov)
            {
                Assert(DestinationOperand->Type == Operand_Register);
                Assert(SourceOperand->Type == Operand_Register || SourceOperand->Type == Operand_Immediate);
                
                s32 Old = *Destination;
                if(Decoded.Flags & Inst_Wide)
                {
                    *Destination = *Source;
                }
                else
                {
                    // NOTE(luca): We assume that an immediate will have an Offset of 0.
                    u32 DestOffset = DestinationOperand->Register.Offset;
                    u32 SourceOffset = SourceOperand->Register.Offset;
                    
                    u8 *SourceByte = (u8 *)Source + SourceOffset;
                    u8 *DestByte = (u8 *)Destination + DestOffset;
                    
                    *DestByte = *SourceByte;
                }
                
#if SIM86_INTERNAL                    
                printf(" %s:0x%x->0x%x", Sim86_RegisterNameFromOperand(&DestinationOperand->Register),
                       Old, *Destination);
#endif
            }
            else if(Decoded.Op == Op_cmp)
            {
                Assert(DestinationOperand->Type == Operand_Register);
                Assert(SourceOperand->Type == Operand_Register || SourceOperand->Type == Operand_Immediate);
                
                s32 Value = (u16)((u16)(*Destination) - ((u16)(*Source)));
                
                FlagsFromValue(&FlagsRegister, Decoded.Flags, Value);
            }
            else if(Decoded.Op == Op_sub)
            {
                Assert(DestinationOperand->Type == Operand_Register);
                Assert(SourceOperand->Type == Operand_Register || SourceOperand->Type == Operand_Immediate);
                
                s32 Old = *Destination;
                *Destination = (u16)((u16)(*Destination) - ((u16)(*Source)));
                printf(" %s:0x%x->0x%x", 
                       Sim86_RegisterNameFromOperand(&DestinationOperand->Register),
                       Old, *Destination);
                
                FlagsFromValue(&FlagsRegister, Decoded.Flags, *Destination);
                
            }
            else if(Decoded.Op == Op_add)
            {
                Assert(DestinationOperand->Type == Operand_Register);
                Assert(SourceOperand->Type == Operand_Register || SourceOperand->Type == Operand_Immediate);
                
                s32 Old = *Destination;
                *Destination = (u16)((u16)(*Destination) + ((u16)(*Source)));
                printf(" %s:0x%x->0x%x", 
                       Sim86_RegisterNameFromOperand(&DestinationOperand->Register),
                       Old, *Destination);
                
                FlagsFromValue(&FlagsRegister, Decoded.Flags, *Destination);
            }
            else if(Decoded.Op == Op_jne)
            {
                if(!(FlagsRegister & Flag_Zero))
                {
                    IPRegister += *Destination;
                }
            }
            else if(Decoded.Op == Op_je)
            {
                if((FlagsRegister & Flag_Zero))
                {
                    IPRegister += *Destination;
                }
            }
            else
            {
                Assert(0 && "Op not implemented yet.");
            }
            
#if SIM86_INTERNAL
            printf(" ip:0x%x->0x%x", OldIPRegister, IPRegister);
#endif
            
        }
        else
        {
            printf("Unrecognized instruction\n");
            break;
        }
        
#if SIM86_INTERNAL
        printf("\n");
#endif
        
    }
    
    printf("Final registers:\n");
    for(u32 RegisterIndex = Register_a;
        RegisterIndex < Register_ds + 1;
        RegisterIndex++)
    {
        register_access Register = {};
        Register.Index = RegisterIndex;
        Register.Offset = 0;
        Register.Count = 2;
        
        u32 Value = Registers[RegisterIndex];
        if(Value > 0)
        {
            printf("     %s: 0x%0x (%d)\n", 
                   Sim86_RegisterNameFromOperand(&Register),
                   Value, Value);
        }
    }
    printf("     ip: 0x%04x (%d)\n", IPRegister, IPRegister);
    
    char FlagsString[ArrayCount(FlagToCharMapping)] = {};
    FlagsToString(FlagsString, FlagsRegister);
    printf("  flags: %s\n", FlagsString);
}

void PrintUsage(char *ExePath)
{
    fprintf(stderr, "usage: %s [-exec] <assembly>\n", ExePath);
}

int main(int ArgCount, char *Args[])
{
    u32 Version = Sim86_GetVersion();
    
#if SIM86_INTERNAL    
    printf("Sim86 Version: %u (expected %u)\n", Version, SIM86_VERSION);
#endif
    
    if(Version != SIM86_VERSION)
    {
        printf("ERROR: Header file version doesn't match DLL.\n");
        return -1;
    }
    
    instruction_table Table;
    Sim86_Get8086InstructionTable(&Table);
    
#if SIM86_INTERNAL    
    printf("8086 Instruction Instruction Encoding Count: %u\n", Table.EncodingCount);
#endif
    
    // print (default)
    // -exec (also execute the disassembled instructinos)
    
    b32 Execute = false;
    char *FileName = 0;
    if(ArgCount == 2)
    {
        FileName = Args[1];
        Execute = false;
        // Print disassembly from assembly
    }
    else if(ArgCount == 3)
    {
        char *Command = Args[1];
        FileName = Args[2];
        if(!strcmp(Command, "-exec"))
        {
            Execute = true;
        }
        else
        {
            fprintf(stderr, "ERROR: Unknown command %s.\n", Command);
            PrintUsage(Args[0]);
        }
    }
    else
    {
        PrintUsage(Args[0]);
    }
    
    u8 Memory[1024] = {};
    if(FileName)
    {
        FILE *File = fopen(FileName, "rb");
        if(File)
        {
            psize Result = fread(Memory, 1, sizeof(Memory), File);
            fclose(File);
            
            if(Execute)
            {
                printf("--- %s execution ---\n", FileName);
                
                Run8086(Result, Memory);
            }
            else
            {
                fprintf(stderr, "ERROR: Disassembling not implemented yet.\n");
            }
            
        }
        else
        {
            fprintf(stderr, "ERROR: Unable to open %s.\n", FileName);
            PrintUsage(Args[0]);
        }
    }
    
    return 0;
}
