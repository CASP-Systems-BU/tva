#include "../include/tva.h"

using namespace tva::debug;
using namespace tva::service;
using namespace tva::service::mpi_service::replicated_3pc;  // We will use Replicated_3PC

int main(int argc, char** argv) {
    // Initialize tva runtime
    tva_init(argc, argv);
    // The party's unique id
    auto pID = runTime.getPartyID();

    const int MAX_BITS_NUMBER = std::numeric_limits<int>::digits;

    // Input plaintext data for testing secure comparisons
    tva::Vector<int> data_a = {111, -4, -17, 2345,    999, 0,  -28922,  1231241, 0, -23437};
    tva::Vector<int> data_b = {0,   -4, -5,  INT_MIN, 999, 70, -243242, INT_MAX, 0, 78};

    // TODO (john): Add tests with different data types, e.g., unsigned, and check corner cases
    //    tva::Vector<int> data_a = {1 << (MAX_BITS_NUMBER-1)};
    //    tva::Vector<int> data_b = {0};

    assert(data_a.size() == data_b.size());


    // **************************************** //
    //           Comparison primitives          //
    // **************************************** //

    // Secret-share original vectors using boolean sharing
    BSharedVector<int> b_v1 = secret_share_b(data_a, 0);
    BSharedVector<int> b_v2 = secret_share_b(data_b, 0);

    // Apply elementwise secure equality
    BSharedVector<int> c_eq = (b_v1 == b_v2);
    // Open equality bits
    auto c_eq_open = c_eq.open();
    // Get ground truth
    tva::Vector<int> eq_bits = (data_a == data_b);

    // Compare equality bits with ground truth
    assert(c_eq_open.same_as(eq_bits));

    if (pID==0) std::cout << "Equality...OK" << std::endl;

    // Apply elementwise secure greater-than
    BSharedVector<int> c_gr = (b_v1 > b_v2);
    // Open greater-than bits
    auto c_gr_open = c_gr.open();
    // Get ground truth
    tva::Vector<int> gr_bits = (data_a > data_b);

    // Compare greater-than bits with ground truth
    assert(c_gr_open.same_as(gr_bits));

    if (pID==0) std::cout << "Greater-than...OK" << std::endl;

    // Apply elementwise secure greater-or-equal
    BSharedVector<int> c_geq = (b_v1 >= b_v2);
    // Open greater-or-equal bits
    auto c_geq_open = c_geq.open();
    // Get ground truth
    tva::Vector<int> geq_bits = (data_a >= data_b);

    // Compare greater-or_equal bits with ground truth
    assert(c_geq_open.same_as(geq_bits));

    if (pID==0) std::cout << "Greater-or-equal...OK" << std::endl;

    // Apply elementwise secure less-than
    BSharedVector<int> c_lt = (b_v1 < b_v2);
    // Open less-than bits
    auto c_lt_open = c_lt.open();
    // Get ground truth
    tva::Vector<int> lt_bits = (data_a < data_b);

//    print_binary(b_v2.asEVector()(0), pID);

//    if (pID==0){
//    std::cout << "INT_MAX: " << INT_MAX << std::endl;
//    print_binary(-1);
//    print_binary(1);
//    print(c_lt_open);
//    print(lt_bits);
//    }

    // Compare less-than bits with ground truth
    assert(c_lt_open.same_as(lt_bits));

    if (pID==0) std::cout << "Less-than...OK" << std::endl;

    // Apply elementwise secure less-or-equal
    BSharedVector<int> c_leq = (b_v1 <= b_v2);
    // Open less-or-equal bits
    auto c_leq_open = c_leq.open();
    // Get ground truth
    tva::Vector<int> leq_bits = (data_a <= data_b);

    // Compare less-or-equal bits with ground truth
    assert(c_leq_open.same_as(leq_bits));

    if (pID==0) std::cout << "Less-or-equal...OK" << std::endl;

    // Apply elementwise secure less-than-zero
    BSharedVector<int> ltz = b_v1.ltz();
    // Open less-than-zero bits
    auto ltz_open = ltz.open();
    // Get ground truth
    tva::Vector<int> zeros(data_a.size());
    // TODO (john): Add support for elementwise comparisons with constant, i.e., "data_a < 0"
    tva::Vector<int> ltz_bits = (data_a < zeros);
    // Compare less-than-zero bits with ground truth
    assert(ltz_open.same_as(ltz_bits));

    if (pID==0) std::cout << "Less-than-zero...OK" << std::endl;

// Tear down communication
#if defined(MPC_USE_MPI_COMMUNICATOR)
    MPI_Finalize();
#endif
    return 0;
}