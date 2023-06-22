#include "../include/tva.h"

using namespace tva::debug;
using namespace tva::service;
using namespace tva::service::mpi_service::replicated_3pc;  // We will use Replicated_3PC

int main(int argc, char** argv) {
    // Initialize tva runtime
    tva_init(argc, argv);
    // The party's unique id
    auto pID = runTime.getPartyID();

//    // **************************************** //
//    //           Test comparisons               //
//    // **************************************** //
//
//    // Order of greater-than comparisons
//    std::vector<bool> inv = {false};
//    // Apply elementwise secure comparison
//    BSharedVector<int> res = tva::compare(v1, v2, inv);
//    // Apply elementwise plaintext comparison
//    tva::Vector<int> cmp_bits = compare_rows<int>({&data_a}, {&data_b}, inv);
//    // Compare results with ground truth
//    auto res_open = res.open();
//    tva::Vector<int> truth = {1, 0, 0, 0, 1};
//    assert(res_open.same_as(cmp_bits) && res_open.same_as(truth));
//
//    // Reverse order of comparisons and repeat
//    inv[0] = true;
//    res = compare_rows<int>({&v1}, {&v2}, inv);
//    res_open = res.open();
//    cmp_bits = compare_rows<int>({&data_a}, {&data_b}, inv);
//    truth = {0, 1, 0, 1, 0};
//    assert(res_open.same_as(cmp_bits) && res_open.same_as(truth));
//
//    if (pID==0) std::cout << "Comparison...OK" << std::endl;

    // **************************************** //
    //             Test swapping                //
    // **************************************** //

    // Input plaintext data for testing secure comparison
    tva::Vector<int> data_a = {111, -14, 0, -156, INT_MAX};
    tva::Vector<int> data_b = {98,  -4,  0, 2847, INT_MIN};
    // Input bits to test secure swapping (1 means that the respective elements in data_a and data_b will be swapped)
    tva::Vector<int> bits = {0, 1, 1, 1, 0};

    // Secret-share original vectors using boolean sharing
    BSharedVector<int> v1 = secret_share_b(data_a, 0);
    BSharedVector<int> v2 = secret_share_b(data_b, 0);
    // Secret-share original bits using boolean sharing
    BSharedVector<int> vb = secret_share_b(bits, 0);

    // Apply elementwise Secure swap according to the given bits
    tva::operators::swap(v1, v2, vb);
    // Compare swapped vectors with ground truth
    auto v1_open = v1.open();
    auto v2_open = v2.open();
    assert( v1_open.same_as({111, -4,  0, 2847, INT_MAX}) &&
            v2_open.same_as({98,  -14, 0, -156, INT_MIN}) );

    if (pID==0) std::cout << "Swapping...OK" << std::endl;

    // **************************************** //
    //                Test sort                 //
    // **************************************** //

    // Input plaintext data for testing secure sort
    tva::Vector<int> unordered = {0, 5, 4, 7, -1 , -2, 8, 1};

    // Secret-share original vector using boolean sharing
    BSharedVector<int> v = secret_share_b(unordered, 0);

    // Sort vector in ASC order
    tva::operators::bitonic_sort(v);
    // Compare sorted vector with ground truth
    auto asc_open = v.open();
    assert( asc_open.same_as({-2, -1, 0, 1, 4, 5, 7, 8}) );

    // Sort vector in DESC order
    tva::operators::bitonic_sort(v, true);
    // Compare sorted vector with ground truth
    auto desc_open = v.open();
    assert( desc_open.same_as({8, 7, 5, 4, 1, 0, -1, -2}) );

    if (pID==0) std::cout << "Sort...OK" << std::endl;

    // **************************************** //
    //             Test table sort              //
    // **************************************** //

    // Input plaintext table to test secure sort on multiple columns:
    //
    // | col1   |   col2    |   col3 |
    // -------------------------------
    // 111      | -14       | INT_MIN
    // 111      | -4        | 0
    // 0        | 0         | 6423
    // 111      | -14       | -11233
    // INT_MAX  | INT_MIN   | INT_MIN
    // 5        | 13        | 7
    // 5        | 13        | 7
    // 5        | 13        | 7
    std::vector<tva::Vector<int>> columns = {{111,      111,    0,      111,     INT_MAX,   5,      5,     5},
                                                 {-14,      -4,     0,      -14,     INT_MIN,   13,     13,    13},
                                                 {INT_MIN,  0,      6423,   -11233,  INT_MIN,   7,      7,     7}};
    EncodedTable<int> t = secret_share(columns, {"[col_1]", "[col_2]", "[col_3]"});
    // Sort table on all columns in DESC->ASC->DESC order
    t.sort({"[col_1]", "[col_2]", "[col_3]"}, {DESC, ASC, DESC});

    // Compare sorted table with ground truth:
    //
    // | col1   |   col2    |   col3 |
    // -------------------------------
    // INT_MAX  | INT_MIN   | INT_MIN
    // 111      | -14       | -11233
    // 111      | -14       | INT_MIN
    // 111      | -4        | 0
    // 5        | 13        | 7
    // 5        | 13        | 7
    // 5        | 13        | 7
    // 0        | 0         | 6423
    DataTable<int> truth = {{INT_MAX,    111,    111,        111,    5,      5,      5,      0},
                       {INT_MIN,    -14,    -14,        -4,     13,     13,     13,     0},
                       {INT_MIN,    -11233, INT_MIN,    0,      7,      7,      7,      6423}};
    DataTable<int> t_open = t.open();
    for (int i=0; i<t_open.size(); i++)
        assert(t_open[i].same_as(truth[i]));

    if (pID==0) std::cout << "Table sort on multiple columns...OK" << std::endl;

    // Repeat test on table with A-shared columns
    t = secret_share(columns, {"[col_1]", "col_2", "[col_3]"});
    // Sort table on two columns in DESC->ASC order
    t.sort({"[col_1]", "[col_3]"}, {DESC, ASC});

    // Compare sorted table with ground truth:
    //
    // | col1   |   col2    |   col3 |
    // -------------------------------
    // INT_MAX  | INT_MIN   | INT_MIN
    // 111      | -14       | INT_MIN
    // 111      | -14       | -11233
    // 111      | -4        | 0
    // 5        | 13        | 7
    // 5        | 13        | 7
    // 5        | 13        | 7
    // 0        | 0         | 6423
    truth = {{INT_MAX,    111,    111,        111,    5,      5,      5,      0},
             {INT_MIN,    -14,    -14,        -4,     13,     13,     13,     0},
             {INT_MIN,    INT_MIN, -11233,    0,      7,      7,      7,      6423}};
    t_open = t.open();
    for (int i=0; i<t_open.size(); i++)
        assert(t_open[i].same_as(truth[i]));

    if (pID==0) std::cout << "Table sort with mixed columns...OK" << std::endl;

    // Tear down communication
#if defined(MPC_USE_MPI_COMMUNICATOR)
    MPI_Finalize();
#endif
    return 0;
}