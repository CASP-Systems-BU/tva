#include "../include/tva.h"

using namespace tva::debug;
using namespace tva::service;
using namespace tva::service::mpi_service::replicated_3pc;  // We will use Replicated_3PC

int main(int argc, char** argv) {
    // Initialize tva runtime
    tva_init(argc, argv);
    // The party's unique id
    auto pID = runTime.getPartyID();

//    tva::Vector<int> data_1 = {111, 1231241, 999, 0, -4, -17, 2345, -28922};
    tva::Vector<int> data_1 = {1, 2, 3, 4, 5, 6, 7, 8};
    tva::Vector<int> data_2 = {0, -4, -5, -123556, 999, 70, -243242, INT_MAX};
    tva::Vector<int> grouping_bits = {1, 1, 1, 1, 0, 0, 0, 0};

    ASharedVector<int> data_1_a = secret_share_a(data_1, 0);
    ASharedVector<int> data_2_a = secret_share_a(data_2, 0);
    ASharedVector<int> grouping_bits_a = secret_share_a(grouping_bits, 0);


    BSharedVector<int> data_1_b = secret_share_b(data_1, 0);
    BSharedVector<int> data_2_b = secret_share_b(data_2, 0);
    BSharedVector<int> grouping_bits_b = secret_share_b(grouping_bits, 0);

    ASharedVector<int> res_a(8);
    BSharedVector<int> res_b(8);

    ASharedVector<int> res_a_reverse(8);
    BSharedVector<int> res_b_reverse(8);

    std::vector<BSharedVector<int>> g_vec({grouping_bits_b});
    std::vector<ASharedVector<int>> d_vec({data_1_a});
    std::vector<ASharedVector<int>> r_vec({res_a});
    std::vector<ASharedVector<int>> r_vec_reverse({res_a_reverse});
    std::vector<BSharedVector<int>> d_vec_b({data_1_b});
    std::vector<BSharedVector<int>> r_vec_b({res_b});
    std::vector<BSharedVector<int>> r_vec_reverse_b({res_b_reverse});


    tva::operators::odd_even_aggregation(g_vec, d_vec, r_vec, &tva::operators::sum_aggregation, false);
    auto res_a_opened = res_a.open();
    assert(res_a_opened.same_as({10, 9, 7, 4, 26, 21, 15, 8}));
    // if(pID == 0) {
    //     print(res_a_opened);
    //     std::cout << "----------------------------------------------------------------" << std::endl;
    // }
    if (pID==0) std::cout << "Odd Even Aggregation Operator(SUM)...OK" << std::endl;


    tva::operators::odd_even_aggregation(g_vec, d_vec, r_vec_reverse, &tva::operators::sum_aggregation, true);
    auto res_a_opened_reverse = res_a_reverse.open();
    assert(res_a_opened_reverse.same_as({1, 3, 6, 10, 5, 11, 18, 26}));
    // if(pID == 0) {
    //     print(res_a_opened_reverse);
    //     std::cout << "----------------------------------------------------------------" << std::endl;
    // }
    if (pID==0) std::cout << "Reverse Odd Even Aggregation Operator(SUM)...OK" << std::endl;


    tva::operators::odd_even_aggregation(g_vec, d_vec_b, r_vec_b, &tva::operators::max_aggregation, false);
    auto res_b_opened = res_b.open();
    assert(res_b_opened.same_as({4, 4, 4, 4, 8, 8, 8, 8}));
    // if(pID == 0) {
    //     print(res_b_opened);
    //     std::cout << "----------------------------------------------------------------" << std::endl;
    // }
    if (pID==0) std::cout << "Odd Even Aggregation Operator(MAX)...OK" << std::endl;

    tva::operators::odd_even_aggregation(g_vec, d_vec_b, r_vec_reverse_b, &tva::operators::max_aggregation, true);
    auto res_b_opened_reverse = res_b_reverse.open();
    assert(res_b_opened_reverse.same_as({1, 2, 3, 4, 5, 6, 7, 8}));
    // if(pID == 0) {
    //     print(res_b_opened_reverse);
    //     std::cout << "----------------------------------------------------------------" << std::endl;
    // }
    if (pID==0) std::cout << "Reverse Odd Even Aggregation Operator(MAX)...OK" << std::endl;

    tva::operators::distinct( d_vec_b, res_b);
    res_b_opened = res_b.open();
    assert(res_b_opened.same_as({1, 1, 1, 1, 1, 1, 1, 1}));
    // if(pID == 0) {
    //     print(res_b_opened);
    //     std::cout << "----------------------------------------------------------------" << std::endl;
    // }
    if (pID==0) std::cout << "Distinct Operator...OK" << std::endl;



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
    std::vector<tva::Vector<int>> columns = {{1, 1, 1, 1, 0, 0, 0, 0},
                                                 {1, 2, 3, 4, 5, 6, 7, 8},
                                                 {1, 2, 3, 4, 5, 6, 7, 8},
                                                 {0, 0, 0, 0, 0, 0, 0, 0},
                                                 {0, 0, 0, 0, 0, 0, 0, 0},
                                                 {0, 0, 0, 0, 0, 0, 0, 0},
                                                 {0, 0, 0, 0, 0, 0, 0, 0},
                                                 {0, 0, 0, 0, 0, 0, 0, 0},
                                                 {0, 0, 0, 0, 0, 0, 0, 0},
                                                 {0, 0, 0, 0, 0, 0, 0, 0},
                                                 {0, 0, 0, 0, 0, 0, 0, 0},
                                                 {0, 0, 0, 0, 0, 0, 0, 0},
                                                 {0, 0, 0, 0, 0, 0, 0, 0}};
    EncodedTable<int> t = secret_share(columns, {"[SEL]", "[DATA]", "DATA",
                                            "SUM", "SUM_R", "[MAX]", "[MAX_R]", "[MIN]", "[MIN_R]",
                                            "TUMBLING_WINDOW", "[GAP_WINDOW]", "[THRESHOLD_WINDOW]",
                                            "[DISTINCT]"});

    t.sort({"[DATA]"}, {1});

    t.odd_even_aggregation({"[SEL]"}, {"DATA"}, {"SUM"}, tva::operators::AggregationType::SUM, false);
    t.odd_even_aggregation({"[SEL]"}, {"DATA"}, {"SUM_R"}, tva::operators::AggregationType::SUM, true);

    t.odd_even_aggregation({"[SEL]"}, {"[DATA]"}, {"[MAX]"}, tva::operators::AggregationType::MAX, false);
    t.odd_even_aggregation({"[SEL]"}, {"[DATA]"}, {"[MAX_R]"}, tva::operators::AggregationType::MAX, true);

    t.odd_even_aggregation({"[SEL]"}, {"[DATA]"}, {"[MIN]"}, tva::operators::AggregationType::MIN, false);
    t.odd_even_aggregation({"[SEL]"}, {"[DATA]"}, {"[MIN_R]"}, tva::operators::AggregationType::MIN, true);

    t.distinct({"[SEL]"}, "[DISTINCT]");

    t.tumbling_window("DATA", 2, "TUMBLING_WINDOW");
    t.gap_session_window({"[SEL]"}, "DATA", "[DATA]", "[GAP_WINDOW]", 2);
    t.threshold_session_window({"[SEL]"}, "[DATA]", "[DATA]", "[THRESHOLD_WINDOW]", 3);

    auto t_open = t.open();
    std::vector<std::vector<int>> t_real = {
        {1, 1, 1, 1, 0, 0, 0, 0},
        {1, 2, 3, 4, 5, 6, 7, 8},
        {1, 2, 3, 4, 5, 6, 7, 8},
        {1, 3, 6, 10, 5, 11, 18, 26},
        {10, 9, 7, 4, 26, 21, 15, 8},
        {1, 2, 3, 4, 5, 6, 7, 8},
        {4, 4, 4, 4, 8, 8, 8, 8},
        {1, 1, 1, 1, 5, 5, 5, 5},
        {1, 2, 3, 4, 5, 6, 7, 8},
        {0, 1, 1, 2, 2, 3, 3, 4},
        {1, 1, 1, 1, -1, -1, -1, -1},
        {31, -1, -1, 4, -1, -1, -1, -1},
        {0, 0, 0, 1, 0, 0, 0, 1}
    };

    for(int i = 0; i < t_real.size(); ++i){
        for(int j = 0; j < t_real[i].size(); ++j){
            assert(t_open[i][j] == t_real[i][j]);
        }
    }

    if (pID==0) std::cout << "Table Operators...OK" << std::endl;


    // if(pID == 0) {
    //     std::cout << "################################################################" << std::endl;
    //     std::cout << "################################################################" << std::endl;
    //     for(int i = 0; i < t_open.size(); ++i){
    //         print(t_open[i]);
    //     }
    //     std::cout << "################################################################" << std::endl;
    //     std::cout << "################################################################" << std::endl;
    // }



// Tear down communication
#if defined(MPC_USE_MPI_COMMUNICATOR)
    MPI_Finalize();
#endif
    return 0;
}
