/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

#include <stdio.h>
#include <string.h>

#include "sim86.h"
#include "sim86_shared.h"

internal void
Run8086(psize DisassemblySize, u8 *Disassembly)
{
    s32 Registers[Register_count] = {}; 
    
    u32 Offset = 0;
    while(Offset < DisassemblySize)
    {
        instruction Decoded;
        Sim86_Decode8086Instruction(DisassemblySize - Offset, Disassembly + Offset, &Decoded);
        if(Decoded.Op)
        {
            Offset += Decoded.Size;
            
#if SIM86_INTERNAL           
            printf("Size:%u Op:%s Flags:0x%x", Decoded.Size, Sim86_MnemonicFromOperationType(Decoded.Op), Decoded.Flags);
#endif
            if(0) {}
            else if(Decoded.Op == Op_mov)
            {
                s32 *Destination = 0;
                s32 *Source = 0;
                
                if(0) {}
                else if(Decoded.Operands[0].Type == Operand_Register)
                {
                    Destination = &Registers[Decoded.Operands[0].Register.Index];
                }
                else if(Decoded.Operands[0].Type == Operand_Memory)
                {
                    Assert(0 && "mov to memory not implemented yet.");
                }
                else if(Decoded.Operands[0].Type == Operand_Immediate)
                {
                    Assert(0 && "Cannot move to immediate value.");
                }
                else
                {
                    Assert(0);
                }
                
                if(0) {}
                else if(Decoded.Operands[1].Type == Operand_Register)
                {
                    Source = &Registers[Decoded.Operands[1].Register.Index];
                }
                else if(Decoded.Operands[1].Type == Operand_Immediate)
                {
                    Source = &Decoded.Operands[1].Immediate.Value;
                }
                else if(Decoded.Operands[1].Type == Operand_Memory)
                {
                    Assert(0 && "mov from memory not implemented yet.");
                }
                else
                {
                    Assert(0);
                }
                
                s32 Old = *Destination;
                if(Decoded.Flags & Inst_Wide)
                {
                    *Destination = *Source;
                }
                else
                {
                    // NOTE(luca): We assume that an immediate will have an Offset of 0.
                    u32 DestOffset = Decoded.Operands[0].Register.Offset;
                    u32 SourceOffset = Decoded.Operands[1].Register.Offset;
                    
                    u8 *SourceByte = (u8 *)Source + SourceOffset;
                    u8 *DestByte = (u8 *)Destination + DestOffset;
                    
                    *DestByte = *SourceByte;
                }
                
#if SIM86_INTERNAL                    
                printf(" ; %s:0x%x->0x%x", Sim86_RegisterNameFromOperand(&Decoded.Operands[0].Register),
                       Old, *Destination);
#endif
                
            }
            else
            {
                Assert(0 && "Op not implemented yet.");
            }
            
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
            printf("     %s: 0x%04x (%d)\n", 
                   Sim86_RegisterNameFromOperand(&Register),
                   Value, Value);
        }
    }
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
