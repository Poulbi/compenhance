enum flags_songs
{
 Flag_French = (1 << 0),
 Flag_Russian = (1 << 1),
};

int songs_strings_count = 2;
char *songs_strings[] =
{
 "French Blues",
 "Russian Blues",
};

enum flags_8086
{
 Flag_Carry = (1 << 0),
 Flag_Parity = (1 << 1),
 Flag_AuxiliaryCarry = (1 << 2),
 Flag_Zero = (1 << 3),
 Flag_Sign = (1 << 4),
 Flag_Overflow = (1 << 5),
 Flag_Interrupt = (1 << 6),
 Flag_Direction = (1 << 7),
 Flag_Trap = (1 << 8),
};

int flags_8086_strings_count = 9;
char *flags_8086_strings[] =
{
 "C",
 "P",
 "A",
 "Z",
 "S",
 "O",
 "I",
 "D",
 "T",
};

enum sim86_enum
{
Sim86_Count
};
