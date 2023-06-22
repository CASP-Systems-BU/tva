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

    BSharedVector<int> a(test_size), b(test_size);

    // start timer
    struct timeval begin, end; long seconds, micro; double elapsed;
    gettimeofday(&begin, 0);

    BSharedVector<int> c_1 = tva::ripple_carry_adder(a, b);

    // stop timer
    gettimeofday(&end, 0);
    seconds = end.tv_sec - begin.tv_sec; micro = end.tv_usec - begin.tv_usec; elapsed = seconds + micro * 1e-6;
    if(pID == 0) {std::cout << "RCA_QUERY:\t\t\t" << test_size << "\t\telapsed\t\t" << elapsed << std::endl;}



    gettimeofday(&begin, 0);

    BSharedVector<int> c_2 = tva::parallel_prefix_adder(a, b);

    gettimeofday(&end, 0);
    seconds = end.tv_sec - begin.tv_sec; micro = end.tv_usec - begin.tv_usec; elapsed = seconds + micro * 1e-6;
    if(pID == 0) {std::cout << "CLA_QUERY:\t\t\t" << test_size << "\t\telapsed\t\t" << elapsed << std::endl;}


#if defined(MPC_USE_MPI_COMMUNICATOR)
    MPI_Finalize();
#endif
    return 0;
}