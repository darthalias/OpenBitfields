#include <stdint.h>
#include <cassert>

/*
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software
 * and associated documentation files (the "Software"),
 * to deal in the Software without restriction,
 * including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice
 * shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

namespace OpenBitfields{
//-----------------------------------------------------------------
// About:
// I hate the way C++ defines bitfields in a non-portable way.
// It goes beyond endieness, there is no telling where a defined
// bit is positioned in a data store.
// Yet, they are used in real-world applications to map HW registers
// in a lot of code. This code relies on the specifics of a given
// Compiler and architecture.
//
// The OpenBitfields package is an EXPERIMENT to make bitfields
// in C++ that are easy to use, reliable, portable, full of
// syntactic sugar, and don't rely on undefined behavior.
//
// I seem to have failed on the last Goal, as this class does rely on
// undefined behavior. The C++ 20 std is not very clear on some aspects of Unions.
// In specific, I make a call to a non trivial assignment to a
// "non-active" data members.
//
// My Frustrated with the standard:
// I'm not sure how a compiler could compile this wrong.
// The bitfield objects in the union are guaranteed to have the
// same address as other union members (11.5.1 - note 2), the first
// data element are Guaranteed to have the correct data due to common
// initial sequence (11.5.1 - note1). The "this" pointer is
// Guaranteed to point to the first element of the class (which is
// the same as the data store). Thus a call
// to a method of a non active member SHOULD do the right thing here.
//
// Undefined behavior is still Undefined behavior.
//
// No effort is made to deal with endieness.
//
// Experimental: Use at your own risk.
//
//
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Use:
// This file provides three macros that are meant to be used
// inside a named union. Use outside a named union will cause compiler
// or program issues.
//
// Unnamed unions are not supported, but I see no reason that couldn't
// be added.
//
// When defining bitfields you must first define a data storage for
// those bitfields. All bitfields must fit inside this data storage
// type, and the data storage type must be an unsigned and integral
// type. (uint8_t, uint16_t, uint32_t, uint64_t).
//
// OPEN_BITFIELD_STORAGE( TYPE ) - Use first when declaring a union.
//                                 Defines the storage to use for
//                                 the bitfields. MUST BE INTEGRAL
//                                 type.
//
// OPEN_BITFIELD_OPERATORS( NAME ) - Defines assignment and assignment
//                                   operators at the union-level if
//                                   you need them. Optional.
//                                   NAME - the name of the union.
//
// OPEN_BITFIELD_OVERLAY( NAME, START, SIZE ) - Defines a new bitfield.
//                                   NAME - Name of this bitfield.
//                                   START - Starting bit. Bit 0 = LSB.
//                                   SIZE - size of bitfield in bits.
//                                   Multiple bitfields may occupy the
//                                   same bits.
//
// Example, ARM ProgramStatusRegister:
//
// //ProgramStatusRegister is the containing union definition.
//  union ProgramStatusRegister{
//    OPEN_BITFIELD_STORAGE(uint32_t); // Define the data storage size.
//    OPEN_BITFIELD_OPERATORS(ProgramStatusRegister);
//
//    OPEN_BITFIELD_OVERLAY(negative_bit, 31,1);
//    OPEN_BITFIELD_OVERLAY(n, 31,1); // n and negative_bit are the same bit.
//
//    OPEN_BITFIELD_OVERLAY(zero_flag, 30,1);
//    OPEN_BITFIELD_OVERLAY(z, 30,1);
//
//    OPEN_BITFIELD_OVERLAY(carry_flag, 29,1);
//    OPEN_BITFIELD_OVERLAY(c, 29,1);
//
//    OPEN_BITFIELD_OVERLAY(overflow_flag, 28,1);
//    OPEN_BITFIELD_OVERLAY(v, 28,1);
//
//    OPEN_BITFIELD_OVERLAY(i, 8,1);
//    OPEN_BITFIELD_OVERLAY(f, 7,1);
//
//    OPEN_BITFIELD_OVERLAY(thumb, 6,1);
//    OPEN_BITFIELD_OVERLAY(t, 6,1);
//
//    OPEN_BITFIELD_OVERLAY(mode, 0,5);
//    OPEN_BITFIELD_OVERLAY(m0, 0,1);
//    OPEN_BITFIELD_OVERLAY(m1, 1,1);
//    OPEN_BITFIELD_OVERLAY(m2, 2,1);
//    OPEN_BITFIELD_OVERLAY(m3, 3,1);
//    OPEN_BITFIELD_OVERLAY(m4, 4,1);
//
//  };
//-----------------------------------------------------------------


//-----------------------------------------------------------------
// The contiguous_bitmask_definition class abstracts the idea of
// a bitfield where all bits in the bitfield are contiguous, and
// provides a place to define the bit manipulation methods needed
// to provide bitfields.
//
// All the data in this class come from the template parameters, and
// is meant to be used for constexpr definitions and bit
// manipulations to reduce run time computation.
//-----------------------------------------------------------------
template<class T, unsigned int START, unsigned int LENGTH>
struct contiguous_bitmask_definition {
    static_assert(LENGTH >0);
    static_assert(START+LENGTH <= (sizeof(T) *8));
    using THIS_t = contiguous_bitmask_definition<T,START, LENGTH>;
    using STORAGE_TYPE = T;

    constexpr contiguous_bitmask_definition()= default;

    contiguous_bitmask_definition(const THIS_t &)=delete;
    THIS_t& operator= (const THIS_t &) = delete;

    //-----------------------------------------------------------------
    // bitmask() returns the value of the bitfield bits all being "1"
    // and all other bits being "0".
    // Example:  T = uint16_t, START = 2, length =2
    //           bitmask() = 0x0c
    //-----------------------------------------------------------------
    constexpr T bitmask() const {
        constexpr uint64_t max_value = (1 << LENGTH) - 1;
        return max_value << START;
    }

    //-----------------------------------------------------------------
    // extract_bits() masks off all bits not defined in the bitmask.
    // Bits in the bitmask keep the value as passed in.
    // Example: Mask = 0xf, in = 0x55,
    //          extract_bits(in) = 0x5
    //-----------------------------------------------------------------
    T extract_bits(T in)const {
        return in & bitmask();
    }

    //-----------------------------------------------------------------
    // clear_bits() masks off all bits defined in the bitmask.
    // Bits NOT in the bitmask keep the value as passed in.
    // Example: Mask = 0xf, in = 0x55,
    //          clear_bits(in) = 0x50
    //-----------------------------------------------------------------
    T clear_bits(T in)const{
        return in &(~bitmask());
    }

    //-----------------------------------------------------------------
    // get_value() returns the bits in "in" that are in the bitmask
    // right shifted. It is the same as extract_bits() >> START.
    // Example: Bitmask = 0xf0, in = 0xab,
    //          get_value(in) = a
    //-----------------------------------------------------------------
    T get_value(T in) const{
        T temp = this->extract_bits(in);
        return (temp >> START);
    }

    //-----------------------------------------------------------------
    // set_value() Sets the masked bits in in to be the value of
    // "value".
    // Example: Bitmask = 0x00ff0000, in_reg = 0xaabbccdd, in_value = 11
    // set_value(in_reg, in_value) = aa11ccdd
    //-----------------------------------------------------------------
    T set_value(T in_reg, T in_value)const{
        T temp = in_value << START;
        temp = this->extract_bits(temp);
        return this->clear_bits(in_reg) | temp;
    }

};

//-----------------------------------------------------------------
// The bitfield class defines a bitfield. It is assumed this class
// is inside a union, and casting this to the DATASTORE_TYPE will
// be valid.
//-----------------------------------------------------------------
template<class DATASTORE_TYPE, uint16_t START, uint16_t LENGTH>
struct bitfield {

    using THIS_t = bitfield<DATASTORE_TYPE,START, LENGTH>;

    DATASTORE_TYPE m_data;

    static constexpr contiguous_bitmask_definition<DATASTORE_TYPE, START, LENGTH> bitfield_mask{};

    bitfield<DATASTORE_TYPE, START, LENGTH>()= default;

    bitfield<DATASTORE_TYPE, START, LENGTH>(const THIS_t &in) = default;

    const DATASTORE_TYPE* datastore() const {
        return reinterpret_cast<const DATASTORE_TYPE*>(this);
    }

    DATASTORE_TYPE* datastore() {
        return reinterpret_cast<DATASTORE_TYPE*>(this);
    }

    DATASTORE_TYPE value() const {
        return bitfield_mask.get_value(m_data);
    }

    operator DATASTORE_TYPE() const {
        return value();
    }

    void set(DATASTORE_TYPE in) {
        m_data = bitfield_mask.set_value(m_data, in);
    }

    THIS_t& operator=(const DATASTORE_TYPE &in) {
        std::cout <<"\noperator=\n";
        set(in);
        return *this;
    }

    THIS_t& operator=(const THIS_t &in) {
        std::cout <<"\noperator=(this)\n";
        set(in);
        return *this;
    }

    DATASTORE_TYPE get() {
        return value();
    }

    inline THIS_t& operator++() {
        set(get() + 1);
        return *this;
    }

    inline THIS_t& operator--() {
        set(get() - 1);
        return *this;
    }

    inline THIS_t& operator+=(DATASTORE_TYPE in) {
        set(get() + in);
        return *this;
    }

    inline THIS_t& operator-=(DATASTORE_TYPE in) {
        set(get() - in);
        return *this;
    }

    inline THIS_t& operator*=(DATASTORE_TYPE in) {
        set(get() * in);
        return *this;
    }

    inline THIS_t& operator/=(DATASTORE_TYPE in) {
        set(get() / in);
        return *this;
    }

    inline THIS_t& operator%=(DATASTORE_TYPE in) {
        set(get() % in);
        return *this;
    }

    inline THIS_t& operator<<=(DATASTORE_TYPE in) {
        set(get() << in);
        return *this;
    }

    inline THIS_t& operator>>=(DATASTORE_TYPE in) {
        set(get() >> in);
        return *this;
    }

    inline THIS_t& operator|=(DATASTORE_TYPE in) {
        set(get() | in);
        return *this;
    }

    inline THIS_t& operator&=(DATASTORE_TYPE in) {
        set(get() & in);
        return *this;
    }

    inline THIS_t& operator^=(DATASTORE_TYPE in) {
        set(get() ^ in);
        return *this;
    }

    inline DATASTORE_TYPE operator~() {
        return ~get();
    }

};

template< class TYPE >
struct bitfield_data_store_t{
    static_assert(std::is_integral<TYPE>::value);
    using STORAGE_TYPE = TYPE;
    TYPE data;
    bitfield_data_store_t<TYPE> () = default;
    bitfield_data_store_t<TYPE> (TYPE in):data(in){};
    bitfield_data_store_t<TYPE> & operator=(TYPE in){
        data = in.data;
    }
    operator TYPE(){
        return data;
    }
    bool operator==(const bitfield_data_store_t<TYPE> & in) const{
        return data == in.data;
    }

};

#define OPEN_BITFIELD_STORAGE( TYPE ) \
      using STORAGE_TYPE = TYPE; \
      OpenBitfields::bitfield_data_store_t<TYPE> bitfield_data_store{}


#define OPEN_BITFIELD_OPERATORS( NAME ) \
      operator STORAGE_TYPE() { \
          return bitfield_data_store.data; \
      } \
      NAME & operator=(const NAME &in) { \
          bitfield_data_store = in.bitfield_data_store; \
          return *this; \
      }\
      NAME() = default; \
      NAME(const STORAGE_TYPE &in) :bitfield_data_store(in){}; \
      bool operator==(const STORAGE_TYPE &in) const{ \
          return bitfield_data_store == in; \
      }

//-----------------------------------------------------------------
// The size-check static_assert here assures us that no new data
// accidently gets into the bitfield struct. The bitfield definition
// must be "common initial sequence" for our unions to be standard
// compliant.
//-----------------------------------------------------------------

#define OPEN_BITFIELD_OVERLAY( NAME, START, SIZE ) \
                OpenBitfields::bitfield< STORAGE_TYPE , START, SIZE> NAME; \
                static_assert( std::is_standard_layout<STORAGE_TYPE >::value ); \
                static_assert( sizeof(NAME) == sizeof(bitfield_data_store) )


}





















