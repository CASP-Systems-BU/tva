#include "../include/tva.h"

using namespace tva::debug;
using namespace tva::service::mpi_service::replicated_3pc;


int main(int argc, char **argv) {
    tva_init(argc, argv);

    // Create test vector
    tva::Vector<int> vec_1 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};


    // Testing computation library.
    tva::Vector<int> vec_1_ = tva::service::runTime.execute_parallel(vec_1, &tva::Vector<int>::simple_bit_compress,0, 1, 31, 1);
    tva::Vector<int> vec_1__(vec_1.size());
    tva::service::runTime.modify_parallel(vec_1__, &tva::Vector<int>::simple_bit_decompress, vec_1_, 0, 1, 31, 1);
    assert(vec_1__.same_as(vec_1));
    if (tva::service::runTime.getPartyID() == 0) printf("Execute-Parallel...ok\n");
    if (tva::service::runTime.getPartyID() == 0) printf("Modify-Parallel...ok\n");



    // Testing Communication
    // Sending (receiving) from party 0 to party 1.
    tva::Vector<int> vec_2(vec_1.size());
    if (tva::service::runTime.getPartyID() == 0) {
        tva::service::runTime.communicate_parallel(&tva::Communicator::sendShares,
                                                       vec_1, +1,
                                                       vec_1.size());
    } else if (tva::service::runTime.getPartyID() == 0) {
        tva::service::runTime.communicate_parallel(&tva::Communicator::receiveShares,
                                                       vec_2, +2,
                                                       vec_2.size());
        assert(vec_2.same_as(vec_1));
    }


    // Exchanging communication
    tva::Vector<int> vec_3(vec_1.size());
    tva::service::runTime.communicate_parallel(&tva::Communicator::exchangeShares,
                                                   vec_1, vec_3,
                                                   +1, +2,
                                                   vec_1.size());
    assert(vec_3.same_as(vec_1));
    if (tva::service::runTime.getPartyID() == 0) printf("Communicate-Parallel...ok\n");


    // Testing Generation
    // 1) Generating new random numbers
    const int test_size = 1000;
    tva::Vector<int> vec_4(test_size), vec_5(test_size),
    vec_6(test_size), vec_7(test_size), vec_8(test_size), vec_9(test_size);
    tva::service::runTime.randomGenerators[0].get()->getMultipleNext(vec_4, 2, test_size);
    tva::service::runTime.generate_parallel(&tva::RandomGenerator::getMultipleNext,
                                                vec_5,
                                                2, test_size);
    // 2) then using the random numbers in communication
    if (tva::service::runTime.getPartyID() != 0){
        tva::service::runTime.communicate_parallel(&tva::Communicator::sendShares,
                                                       vec_4, 3 - tva::service::runTime.getPartyID(),
                                                       vec_4.size());
        tva::service::runTime.communicate_parallel(&tva::Communicator::sendShares,
                                                       vec_5, 3 - tva::service::runTime.getPartyID(),
                                                       vec_5.size());
    }else{
        tva::service::runTime.communicate_parallel(&tva::Communicator::receiveShares,
                                                       vec_6, +1,
                                                       vec_6.size());
        tva::service::runTime.communicate_parallel(&tva::Communicator::receiveShares,
                                                       vec_7, +2,
                                                       vec_7.size());
        tva::service::runTime.communicate_parallel(&tva::Communicator::receiveShares,
                                                       vec_8, +1,
                                                       vec_8.size());
        tva::service::runTime.communicate_parallel(&tva::Communicator::receiveShares,
                                                       vec_9, +2,
                                                       vec_9.size());

        assert((vec_4 + vec_6 + vec_7).same_as(tva::Vector<int>(test_size, 0)));
        assert((vec_5 + vec_8 + vec_9).same_as(tva::Vector<int>(test_size, 0)));
    }
    if (tva::service::runTime.getPartyID() == 0) printf("Generate-Parallel...ok\n");



#if defined(MPC_USE_MPI_COMMUNICATOR)
    MPI_Finalize();
#endif
    return 0;
}