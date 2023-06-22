#include "../include/tva.h"

using namespace tva::debug;
using namespace tva::service::mpi_service::replicated_3pc;  // We will use Replicated_3PC

int main(int argc, char **argv) {
    tva_init(argc, argv);

    // Testing Vector elements operations

    // Testing Vector Patterns
    tva::Vector<int> vec_pattern_1 = {0, 1, 2, 3, 4, 5, 6, 7};

    tva::Vector<int> vec_pattern_2 = vec_pattern_1.simple_subset_reference(0);
    assert(vec_pattern_2.same_as(tva::Vector<int>({0, 1, 2, 3, 4, 5, 6, 7})));

    tva::Vector<int> vec_pattern_3 = vec_pattern_1.simple_subset_reference(0, 2);
    assert(vec_pattern_3.same_as(tva::Vector<int>({0, 2, 4, 6})));

    tva::Vector<int> vec_pattern_4 = vec_pattern_1.alternating_subset_reference(2,2)
            .simple_subset_reference(0, 2, 2);
    assert(vec_pattern_4.same_as(tva::Vector<int>({0, 4})));

    tva::Vector<int> vec_pattern_5 = vec_pattern_1.simple_subset_reference(0, 1, 3);
    assert(vec_pattern_5.same_as(tva::Vector<int>({0, 1, 2, 3})));

    tva::Vector<int> vec_pattern_6 = vec_pattern_1.alternating_subset_reference(2, 2);
    assert(vec_pattern_6.same_as(tva::Vector<int>({0, 1, 4, 5})));

    tva::Vector<int> vec_pattern_7 = vec_pattern_1.reversed_alternating_subset_reference(2,2);
    assert(vec_pattern_7.same_as(tva::Vector<int>({1, 0, 5, 4})));

    tva::Vector<int> vec_pattern_8 = vec_pattern_1.simple_subset_reference(2,1)
            .reversed_alternating_subset_reference(2,2);
    assert(vec_pattern_8.same_as(tva::Vector<int>({3, 2, 7, 6})));

    tva::Vector<int> vec_pattern_9 = vec_pattern_1.simple_subset_reference(3,1)
            .reversed_alternating_subset_reference(3,1);
    assert(vec_pattern_9.same_as(tva::Vector<int>({5, 4, 3, 7})));

    tva::Vector<int> vec_pattern_10 = vec_pattern_1.simple_subset_reference(0, 2)
            .repeated_subset_reference(2);
    assert(vec_pattern_10.same_as(tva::Vector<int>({0, 0, 2, 2, 4, 4, 6, 6})));

    tva::Vector<int> vec_pattern_11 = vec_pattern_1.cyclic_subset_reference(2);
    assert(vec_pattern_11.same_as(tva::Vector<int>({0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7})));

    tva::Vector<int> vec_pattern_12 = vec_pattern_1.simple_subset_reference(1,1)
            .reversed_alternating_subset_reference(2,4)
            .repeated_subset_reference(3)
            .cyclic_subset_reference(2);
    assert(vec_pattern_12.same_as(tva::Vector<int>({2, 2, 2, 1, 1, 1, 7, 7, 7, 2, 2, 2, 1, 1, 1, 7, 7, 7})));

    // if(tva::service::runTime.getPartyID() == 0){
    //     print(vec_pattern_1);
    //     print(vec_pattern_2);
    //     print(vec_pattern_3);
    //     print(vec_pattern_4);
    //     print(vec_pattern_5);
    //     print(vec_pattern_6);
    //     print(vec_pattern_7);
    //     print(vec_pattern_8);
    //     print(vec_pattern_9);
    //     print(vec_pattern_10);
    //     print(vec_pattern_11);
    //     print(vec_pattern_12);
    // }
    if (tva::service::runTime.getPartyID() == 0) printf("Vector-Patterns...ok\n");



    // Testing bit manipulation
    tva::Vector<int> vec_bit_01 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    tva::Vector<int> vec_bit_02 = vec_bit_01.simple_bit_compress(0, 1, 0, 1);
    tva::Vector<int> vec_bit_03 = vec_bit_01.simple_bit_compress(0, 1, 0, 2);
    tva::Vector<int> vec_bit_04 = vec_bit_01.simple_bit_compress(1, 1, 1, 1);
    tva::Vector<int> vec_bit_05 = vec_bit_01.simple_bit_compress(1, 1, 1, 2);
    tva::Vector<int> vec_bit_06 = vec_bit_01.simple_bit_compress(0, 1, 3, 1);
    tva::Vector<int> vec_bit_07 = vec_bit_01.simple_bit_compress(0, 1, 3, 2);
    tva::Vector<int> vec_bit_08 = vec_bit_01.simple_bit_compress(0, 2, 3, 1);
    tva::Vector<int> vec_bit_09 = vec_bit_01.simple_bit_compress(0, 2, 3, 2);

    tva::Vector<int> vec_bit_11 = {0x0f0f0f0f, 0x0f0f0f0f, 0x0f0f0f0f, 0x0f0f0f0f};

    tva::Vector<int> vec_bit_12 = vec_bit_11.alternating_bit_compress(0, 1, 2, 30, 1);
    assert(vec_bit_12.same_as(tva::Vector<int>({0x000000ff})));

    tva::Vector<int> vec_bit_13 = vec_bit_11.alternating_bit_compress(0, 1, 4, 4, 1);
    assert(vec_bit_13.same_as(tva::Vector<int>({(int) 0xffffffff, (int) 0xffffffff})));

    tva::Vector<int> vec_bit_14 = vec_bit_11.alternating_bit_compress(0, 8, 8, 8, 1);
    assert(vec_bit_14.same_as(tva::Vector<int>({0x000000ff})));

    tva::Vector<int> vec_bit_15 = vec_bit_11.alternating_bit_compress(0, 8, 8, 8, -1);
    assert(vec_bit_15.same_as(tva::Vector<int>({0x00000000})));


    // if (tva::service::runTime.getPartyID() == 0) {
    //     print_binary(vec_bit_01, 0);
    //     print_binary(vec_bit_02, 0);
    //     print_binary(vec_bit_03, 0);
    //     print_binary(vec_bit_04, 0);
    //     print_binary(vec_bit_05, 0);
    //     print_binary(vec_bit_06, 0);
    //     print_binary(vec_bit_07, 0);
    //     print_binary(vec_bit_08, 0);
    //     print_binary(vec_bit_09, 0);
    //
    //     print_binary(vec_bit_11, 0);
    //     print_binary(vec_bit_12, 0);
    //     print_binary(vec_bit_13, 0);
    //     print_binary(vec_bit_14, 0);
    //     print_binary(vec_bit_15, 0);
    // }

    if (tva::service::runTime.getPartyID() == 0) printf("Vector-bit...ok\n");

#if defined(MPC_USE_MPI_COMMUNICATOR)
    MPI_Finalize();
#endif

    return 0;
}