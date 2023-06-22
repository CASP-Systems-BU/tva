#include "../../include/tva.h"

using namespace tva::debug;
using namespace tva::service;

#ifdef USE_FANTASTIC_FOUR
using namespace tva::service::mpi_service::fantastic_4pc;
#else
using namespace tva::service::mpi_service::replicated_3pc;
#endif


int main(int argc, char** argv) {
    // Initialize tva runtime [executable - threads_num - p_factor - batch_size]
    tva_init(argc, argv);
    auto pID = runTime.getPartyID();
    int test_size = 128;
    if(argc >= 5){
        test_size = atoi(argv[4]);
    }

    std::vector<std::string> schema = {"[SEL]", "DATA", "[DATA]", "SUM", "[MAX]", "[MIN]"};
    std::vector<tva::Vector<int>> data(schema.size(), test_size);
    EncodedTable<int> table = secret_share(data, schema);

    // start timer
    struct timeval begin, end; long seconds, micro; double elapsed;
    gettimeofday(&begin, 0);

    table.odd_even_aggregation({"[SEL]"}, {"DATA"}, {"SUM"}, tva::operators::AggregationType::SUM, false);

    // stop timer
    gettimeofday(&end, 0);
    seconds = end.tv_sec - begin.tv_sec; micro = end.tv_usec - begin.tv_usec; elapsed = seconds + micro * 1e-6;
    if(pID == 0) {std::cout << "SUM_AGGREGATION:\t\t\t" << test_size << "\t\telapsed\t\t" << elapsed << std::endl;}



    gettimeofday(&begin, 0);

    table.odd_even_aggregation({"[SEL]"}, {"[DATA]"}, {"[MIN]"}, tva::operators::AggregationType::MIN, false);

    gettimeofday(&end, 0);
    seconds = end.tv_sec - begin.tv_sec; micro = end.tv_usec - begin.tv_usec; elapsed = seconds + micro * 1e-6;
    if(pID == 0) {std::cout << "MIN_AGGREGATION:\t\t\t" << test_size << "\t\telapsed\t\t" << elapsed << std::endl;}


    
    gettimeofday(&begin, 0);

    table.odd_even_aggregation({"[SEL]"}, {"[DATA]"}, {"[MAX]"}, tva::operators::AggregationType::MAX, false);

    gettimeofday(&end, 0);
    seconds = end.tv_sec - begin.tv_sec; micro = end.tv_usec - begin.tv_usec; elapsed = seconds + micro * 1e-6;
    if(pID == 0) {std::cout << "MAX_AGGREGATION:\t\t\t" << test_size << "\t\telapsed\t\t" << elapsed << std::endl;}


#if defined(MPC_USE_MPI_COMMUNICATOR)
    MPI_Finalize();
#endif
    return 0;
}