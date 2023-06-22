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

    // TODO: do aggregation by device_id
    std::vector<std::string> schema = {"[TIMESTAMP]", "TIMESTAMP", "[DEVICE_ID]",
                                       "TUMBLING_WINDOW_PER_HOUR", "[TUMBLING_WINDOW_PER_HOUR]",
                                       "ENERGY_CONSUMPTION", "TOTAL_CONSUMPTION"};
    std::vector<tva::Vector<int8_t>> energy_data(schema.size(), test_size);

    EncodedTable<int8_t> energy_table = secret_share(energy_data, schema);
    ASharedVector<int64_t> timestamp_a(test_size);


    // start timer
    struct timeval begin, end; long seconds, micro; double elapsed;
    gettimeofday(&begin, 0);

    // energy_table.tumbling_window("TIMESTAMP", 3600, "TUMBLING_WINDOW_PER_HOUR");
    ASharedVector<int64_t> window_id = timestamp_a.div(3600);
    (*energy_table["TUMBLING_WINDOW_PER_HOUR"].contents.get()) = window_id;
    energy_table.convert_a2b("TUMBLING_WINDOW_PER_HOUR", "[TUMBLING_WINDOW_PER_HOUR]");
    // energy_table.sort({"[TUMBLING_WINDOW_PER_HOUR]"}, {false}, {"ENERGY_CONSUMPTION"});
    energy_table.odd_even_aggregation({"[TUMBLING_WINDOW_PER_HOUR]"},
                                      {"ENERGY_CONSUMPTION"}, {"TOTAL_CONSUMPTION"},
                                      tva::operators::AggregationType::SUM);


    // stop timer
    gettimeofday(&end, 0);
    seconds = end.tv_sec - begin.tv_sec; micro = end.tv_usec - begin.tv_usec; elapsed = seconds + micro * 1e-6;
    if(pID == 0) {std::cout << "WALDO_ENERGY_QUERY:\t\t\t" << test_size << "\t\telapsed\t\t" << elapsed << std::endl;}
    

#if defined(MPC_USE_MPI_COMMUNICATOR)
    MPI_Finalize();
#endif
    return 0;
}