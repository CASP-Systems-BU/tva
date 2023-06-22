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
                                        "[PATIENT_ID]", "[GLUCOSE]", "INSULIN",
                                        "[THRESHOLD_WINDOW]", "TOTAL_INSULIN_EVENTS"};
    std::vector<tva::Vector<int>> medical_data (schema.size(), test_size);

    EncodedTable<int> medical_table = secret_share(medical_data, schema);

    // start timer
    struct timeval begin, end; long seconds, micro; double elapsed;
    gettimeofday(&begin, 0);


    medical_table.sort({"[PATIENT_ID]", "[TIMESTAMP]"}, {false, false}, {"[GLUCOSE]", "INSULIN"});
    medical_table.threshold_session_window({"[PATIENT_ID]"},
                                           "[GLUCOSE]", "[TIMESTAMP]",
                                           "[THRESHOLD_WINDOW]", 5, false);
    medical_table.odd_even_aggregation({"[PATIENT_ID]", "[THRESHOLD_WINDOW]"},
                                       {"INSULIN"}, {"TOTAL_INSULIN_EVENTS"},
                                       tva::operators::AggregationType::SUM);

    // stop timer
    gettimeofday(&end, 0);
    seconds = end.tv_sec - begin.tv_sec; micro = end.tv_usec - begin.tv_usec; elapsed = seconds + micro * 1e-6;
    if(pID == 0) {std::cout << "MEDICAL_QUERY:\t\t\t" << test_size << "\t\telapsed\t\t" << elapsed << std::endl;}


#if defined(MPC_USE_MPI_COMMUNICATOR)
    MPI_Finalize();
#endif
    return 0;
}