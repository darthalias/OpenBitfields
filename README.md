# OpenBitfields
An attempt at a bitfield library. 

 About:
 I hate the way C++ defines bitfields in a non-portable way.
 It goes beyond endieness, there is no telling where a defined
 bit is positioned in a data store.
 Yet, they are used in real-world applications to map HW registers
 in a lot of code. This code relies on the specifics of a given
 Compiler and architecture.

 The OpenBitfields package is an EXPERIMENT to make bitfields
 in C++ that are easy to use, reliable, portable, full of
 syntactic sugar, and don't rely on undefined behavior.

 I seem to have failed on the last Goal, as this class does rely on
 undefined behavior. The C++ 20 std is not very clear on some aspects of Unions.
 In specific, I make a call to a non trivial assignment to a
 "non-active" data members.

 My Frustrated with the standard:
 I'm not sure how a compiler could compile this wrong.
 The bitfield objects in the union are guaranteed to have the
 same address as other union members (11.5.1 - note 2), the first
 data element are Guaranteed to have the correct data due to common
 initial sequence (11.5.1 - note1). The "this" pointer is
 Guaranteed to point to the first element of the class (which is
 the same as the data store). Thus a call
 to a method of a non active member SHOULD do the right thing here.

 Undefined behavior is still Undefined behavior.

 No effort is made to deal with endieness.

 Experimental: Use at your own risk.


-----------------------------------------------------------------

-----------------------------------------------------------------
 Use:
 This file provides three macros that are meant to be used
 inside a named union. Use outside a named union will cause compiler
 or program issues.

 Unnamed unions are not supported, but I see no reason that couldn't
 be added.

 When defining bitfields you must first define a data storage for
 those bitfields. All bitfields must fit inside this data storage
 type, and the data storage type must be an unsigned and integral
 type. (uint8_t, uint16_t, uint32_t, uint64_t).

 OPEN_BITFIELD_STORAGE( TYPE ) - Use first when declaring a union.
                                 Defines the storage to use for
                                 the bitfields. MUST BE INTEGRAL
                                 type.

 OPEN_BITFIELD_OPERATORS( NAME ) - Defines assignment and assignment
                                   operators at the union-level if
                                   you need them. Optional.
                                   NAME - the name of the union.

 OPEN_BITFIELD_OVERLAY( NAME, START, SIZE ) - Defines a new bitfield.
                                   NAME - Name of this bitfield.
                                   START - Starting bit. Bit 0 = LSB.
                                   SIZE - size of bitfield in bits.
                                   Multiple bitfields may occupy the
                                   same bits.

 Example, ARM ProgramStatusRegister:

 //ProgramStatusRegister is the containing union definition.
  union ProgramStatusRegister{
    OPEN_BITFIELD_STORAGE(uint32_t); // Define the data storage size.
    OPEN_BITFIELD_OPERATORS(ProgramStatusRegister);

    OPEN_BITFIELD_OVERLAY(negative_bit, 31,1);
    OPEN_BITFIELD_OVERLAY(n, 31,1); // n and negative_bit are the same bit.

    OPEN_BITFIELD_OVERLAY(zero_flag, 30,1);
    OPEN_BITFIELD_OVERLAY(z, 30,1);

    OPEN_BITFIELD_OVERLAY(carry_flag, 29,1);
    OPEN_BITFIELD_OVERLAY(c, 29,1);

    OPEN_BITFIELD_OVERLAY(overflow_flag, 28,1);
    OPEN_BITFIELD_OVERLAY(v, 28,1);

    OPEN_BITFIELD_OVERLAY(i, 8,1);
    OPEN_BITFIELD_OVERLAY(f, 7,1);

    OPEN_BITFIELD_OVERLAY(thumb, 6,1);
    OPEN_BITFIELD_OVERLAY(t, 6,1);

    OPEN_BITFIELD_OVERLAY(mode, 0,5);
    OPEN_BITFIELD_OVERLAY(m0, 0,1);
    OPEN_BITFIELD_OVERLAY(m1, 1,1);
    OPEN_BITFIELD_OVERLAY(m2, 2,1);
    OPEN_BITFIELD_OVERLAY(m3, 3,1);
    OPEN_BITFIELD_OVERLAY(m4, 4,1);

  };