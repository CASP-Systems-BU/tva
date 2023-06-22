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

    std::vector<std::string> schema = {"[TIMESTAMP]", "TIMESTAMP",
                                       "[MACHINE_TYPE]", "[EVENT_TYPE]", "[JOB_ID]",
                                       "[SEL]", "SEL", "[GAP_WINDOW]", "TOTAL_TASKS_PER_SESSION"};
    std::vector<tva::Vector<int>> cloud_data(schema.size(), test_size);
    EncodedTable<int> cloud_table = secret_share(cloud_data, schema);

    // TODO: use operation with public constant
    EncodedTable<int> constant_table = secret_share(std::vector<tva::Vector<int>>(1, test_size),
                                               {"[ZERO]"});

    // start timer
    struct timeval begin, end; long seconds, micro; double elapsed;
    gettimeofday(&begin, 0);

    // selection on event type == 0
    // sort by machine type, timestamp
    // keyed_gap window on machine type
    //      -- compute sum of selected per session and machine type
    cloud_table["[SEL]"] = cloud_table["[EVENT_TYPE]"] == constant_table["[ZERO]"];
    cloud_table.convert_b2a_bit("[SEL]", "SEL");
    cloud_table.sort({"[MACHINE_TYPE]", "[TIMESTAMP]"}, {false, false}, {"TIMESTAMP", "SEL"});
    cloud_table.gap_session_window({"[MACHINE_TYPE]"},
                                   "TIMESTAMP", "[TIMESTAMP]",
                                   "[GAP_WINDOW]", 10, false);
    cloud_table.odd_even_aggregation({"[MACHINE_TYPE]", "[GAP_WINDOW]"},
                                     {"SEL"}, {"TOTAL_TASKS_PER_SESSION"},
                                     tva::operators::AggregationType::SUM);

    // stop timer
    gettimeofday(&end, 0);
    seconds = end.tv_sec - begin.tv_sec; micro = end.tv_usec - begin.tv_usec; elapsed = seconds + micro * 1e-6;
    if(pID == 0) {std::cout << "CLOUD_QUERY:\t\t\t" << test_size << "\t\telapsed\t\t" << elapsed << std::endl;}


#if defined(MPC_USE_MPI_COMMUNICATOR)
    MPI_Finalize();
#endif
    return 0;
}