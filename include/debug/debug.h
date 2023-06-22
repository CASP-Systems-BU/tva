#ifndef TVA_DEBUG_H
#define TVA_DEBUG_H

#define MPC_USE_RANDOM_GENERATOR_TRIPLES 1
#define MPC_USE_MPI_COMMUNICATOR 1
// #define MPC_USE_GRPC_COMMUNICATOR 1

#define MPC_GENERATE_DATA 1
#define MPC_RANDOM_DATA_RANGE 100
#define MPC_USE_RANDOM_GENERATOR_DATA 1
#define MPC_EVALUATE_CORRECT_OUTPUT 1
#define MPC_CHECK_CORRECTNESS 1


// #define MPC_PRINT_RESULT 1
// #define MPC_COMMUNICATOR_PRINT_DATA 1


#define USE_PARALLEL_PREFIX_ADDER 1
#define USE_DIVISION_CORRECTION 1
// #define RECYCLE_THREAD_MEMORY 1
// #define USE_FANTASTIC_FOUR 1



// used in memory.h experiments
// #define random_generation

#include <bitset>
#include <cassert>
#include <cmath>
#include <iostream>
#include <climits> 
#include <sys/time.h>

namespace tva { namespace debug{

    static void print_bin_(const int& num1, const int& num2, bool add_line, int party_num = 0) {
        if (party_num == 0) {
            std::bitset<32> x(num1);
            std::bitset<32> y(num2);
            std::cout << x << "\t\t" << y << "\t\t";

            if (add_line) {
                std::cout << std::endl;
            }
        }
    }

    template <typename VectorType>
    static void print(VectorType& vec, const int& partyID = 0) {
        if (partyID == 0) {
            for (int i = 0; i < vec.size(); ++i) {
                std::cout << vec[i] << "\t\t";
            }
            std::cout << std::endl;
        }
    }

    template <typename TableType>
    static void print_table(const TableType& table, const int& partyID = 0){
        if(partyID == 0) {
            std::cout << "################################################################" << std::endl;
            std::cout << "################################################################" << std::endl;
            for(int i = 0; i < table.size(); ++i){
                print(table[i], partyID);
            }
            std::cout << "################################################################" << std::endl;
            std::cout << "################################################################" << std::endl;
        }
    }

    template <typename DataType>
    DataType get_bit(const DataType& s, int i) {
        using Unsigned_type = typename std::make_unsigned<DataType>::type;
        return ((Unsigned_type) s >> i) & (Unsigned_type) 1;
    }

    /**
     * Prints to stdout the binary representation of `s`.
     * @tparam DataType - The data type of the input `s`
     * @param s - The input whose binary representation we want to print.
     */
    template <typename DataType>
    void print_binary(const DataType& s) {
        // NOTE: Rounding is needed because signed share types have one digit less.
        static const int MAX_BITS_NUMBER = std::pow(2, std::ceil(std::log2(std::numeric_limits<DataType>::digits)));
        char bits[MAX_BITS_NUMBER+1];   // +1 for the final '\0'
        bits[MAX_BITS_NUMBER] = '\0';
        for (int i=0; i<MAX_BITS_NUMBER; i++) {
            if (get_bit(s, i) == 1) {
                bits[MAX_BITS_NUMBER-i-1] = '1';
            }
            else {
                bits[MAX_BITS_NUMBER-i-1] = '0';
            }
        }
        std::cout << bits << "\t";
    }

    /**
     * Prints to stdout the binary representation of the elements in `v`.
     * @tparam VectorType - The vector type.
     * @param v - The input vector.
     * @param partyID - The ID of the party that calls this function.
     */
    template <typename VectorType>
    void print_binary(const VectorType& v, int partyID) {
        std::cout << "[" << partyID << "]: ";
        for (int i=0; i<v.size(); i++)
            print_binary(v[i]);
        std::cout << std::endl;
    }

}} // namespace tva::debug

#endif // TVA_DEBUG_H
