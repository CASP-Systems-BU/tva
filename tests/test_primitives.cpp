#include "../include/tva.h"

using namespace tva::debug;
using namespace tva::service;
using namespace tva::service::mpi_service::replicated_3pc;  // We will use Replicated_3PC

int main(int argc, char** argv) {
    // Initialize tva runtime
    tva_init(argc, argv);
    // The party's unique id
    auto pID = runTime.getPartyID();

    // Input plaintext data for testing secure comparisons
    tva::Vector<int> data_a = {111, -4, -17, 2345, 999, 0, -28922, 1231241, 0, -23437};
    tva::Vector<int> data_b = {0, -4, -5, -123556, 999, 70, -243242, INT_MAX, 0, 78};

    assert(data_a.size() == data_b.size());

    // **************************************** //
    //           Arithmetic primitives          //
    // **************************************** //
    ASharedVector<int> a_v1 = secret_share_a(data_a, 0);
    ASharedVector<int> a_v2 = secret_share_a(data_b, 0);

    // Apply elementwise secure addition
    ASharedVector<int> c_add = a_v1 + a_v2;
    // Open added numbers
    auto c_add_open = c_add.open();
    // Get ground truth
    tva::Vector<int> sum = (data_a + data_b);
    // Compare sums with ground truth
    assert(c_add_open.same_as(sum));

    if (pID==0) std::cout << "Addition...OK" << std::endl;

    // Apply elementwise secure addition
    ASharedVector<int> c_sub = a_v1 - a_v2;
    // Open added numbers
    auto c_sub_open = c_sub.open();
    // Get ground truth
    tva::Vector<int> sub = (data_a - data_b);
    // Compare diffs with ground truth
    assert(c_sub_open.same_as(sub));

    if (pID==0) std::cout << "Subtraction...OK" << std::endl;

    // Apply elementwise secure multiplication
    ASharedVector<int> c_mul = a_v1 * a_v2;
    // Open added numbers
    auto c_mul_open = c_mul.open();
    // Get ground truth
    tva::Vector<int> mul = (data_a * data_b);
    // Compare multiplication results with ground truth
    assert(c_mul_open.same_as(mul));

    if (pID==0) std::cout << "Multiplication...OK" << std::endl;

    // TODO: check overflowing when dividing INT_MAX in fantastic four
    // tva::Vector<int> div_data = {111, 1231241, 999, 109, 0, -4, -17, 2345,   -28922,  -23437};
    // tva::Vector<int> div_data = {0, -4, -5, -123556, 999, 70, -243242, INT_MAX, 0, 78};
    tva::Vector<int> div_data = {0, 4, 5, 123556, 999, 70, 243242, INT_MAX, 0, 78};
    ASharedVector<int> shared_div_data = secret_share_a(div_data, 0);

    // Apply elementwise secure public constant division
    ASharedVector<int> c_div = shared_div_data.div(8);
    // Open divided numbers
    auto c_div_open = c_div.open();
    // Get ground truth
    tva::Vector<int> div = div_data/8;
    // Compare division results with ground truth
    assert(c_div_open.same_as(div));

    if (pID==0) std::cout << "Public Constant Division...OK" << std::endl;

    // Apply elementwise secure arithmetic negation
    ASharedVector<int> c_a_neg = -a_v1;
    // Open added numbers
    auto c_a_neg_open = c_a_neg.open();
    // Get ground truth
    tva::Vector<int> a_neg = -data_a;
    // Compare negated elements with ground truth
    assert(c_a_neg_open.same_as(a_neg));

    if (pID==0) std::cout << "Arithmetic negation...OK" << std::endl;

    // **************************************** //
    //             Boolean primitives           //
    // **************************************** //

    // Secret-share original vectors using boolean sharing
    BSharedVector<int> b_v1 = secret_share_b(data_a, 0);
    BSharedVector<int> b_v2 = secret_share_b(data_b, 0);

    // Apply elementwise secure boolean AND
    BSharedVector<int> c_and = b_v1 & b_v2;
    // Open AND results
    auto c_and_open = c_and.open();
    // Get ground truth
    tva::Vector<int> a_and = data_a & data_b;
    // Compare AND results with ground truth
    assert(c_and_open.same_as(a_and));

    if (pID==0) std::cout << "Bitwise AND...OK" << std::endl;

    // Apply elementwise secure boolean XOR
    BSharedVector<int> c_xor = b_v1 ^ b_v2;
    // Open XOR results
    auto c_xor_open = c_xor.open();
    // Get ground truth
    tva::Vector<int> a_xor = data_a ^ data_b;
    // Compare XOR results with ground truth
    assert(c_xor_open.same_as(a_xor));

    if (pID==0) std::cout << "Bitwise XOR...OK" << std::endl;

    // Apply elementwise secure boolean OR
    BSharedVector<int> c_or = b_v1 | b_v2;
    // Open OR results
    auto c_or_open = c_or.open();
    // Get ground truth
    tva::Vector<int> a_or = data_a | data_b;
    // Compare OR results with ground truth
    assert(c_or_open.same_as(a_or));

    if (pID==0) std::cout << "Bitwise OR...OK" << std::endl;

    // Apply elementwise secure boolean negation
    BSharedVector<int> c_b_neg = !(b_v1 == b_v2);
    // Open NOT results
    auto c_b_neg_open = c_b_neg.open();
    // Get ground truth
    tva::Vector<int> a_b_neg = !(data_a==data_b);

    // Compare NOT results with ground truth
    assert(c_b_neg_open.same_as(a_b_neg));

    if (pID==0) std::cout << "Boolean NOT...OK" << std::endl;

    // Apply elementwise secure boolean negation
    BSharedVector<int> c_b_comp = ~b_v1;
    // Open NOT results
    auto c_b_comp_open = c_b_comp.open();
    // Get ground truth
    tva::Vector<int> a_b_comp = ~data_a;

    // Compare NOT results with ground truth
    assert(c_b_comp_open.same_as(a_b_comp));

    if (pID==0) std::cout << "Boolean complement...OK" << std::endl;

    // Apply elementwise secure boolean addition with PPA
    BSharedVector<int> c_b_add_ppa = parallel_prefix_adder(b_v1, b_v2);
    // Open addition results
    auto c_b_add_ppa_open = c_b_add_ppa.open();
    // Get ground truth
    tva::Vector<int> a_b_add = data_a + data_b;

    // Compare addition results with ground truth
    assert(c_b_add_ppa_open.same_as(a_b_add));
    if (pID==0) std::cout << "Parallel prefix adder...OK" << std::endl;

    // Apply elementwise secure boolean addition with RCA
    // NOTE: this does not currently pass
    BSharedVector<int> c_b_add_rca = ripple_carry_adder(b_v1, b_v2);
    // Open addition results
    auto c_b_add_rca_open = c_b_add_rca.open();

    // Compare addition results with ground truth
    assert(c_b_add_rca_open.same_as(a_b_add));
    if (pID==0) std::cout << "Ripple carry adder...OK" << std::endl;



    // **************************************** //
    //             Other primitives             //
    // **************************************** //

    //TODO (john): Add tests for mask() and set_bit();

    BSharedVector<int> a2b1 = a_v1.a2b();
    BSharedVector<int> a2b2 = a_v2.a2b();

    auto a2b1_open = a2b1.open();
    auto a2b2_open = a2b2.open();

    assert(a2b1_open.same_as(data_a));
    assert(a2b2_open.same_as(data_b));

    if (pID == 0) std::cout << "Arithmetic to boolean conversion...OK" << std::endl;

// Tear down communication
#if defined(MPC_USE_MPI_COMMUNICATOR)
    MPI_Finalize();
#endif
    return 0;
}
