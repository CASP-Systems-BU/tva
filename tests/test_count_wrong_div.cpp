#include "../include/tva.h"

#ifdef EXTRA_ADDITIONAL_FLAG
using namespace tva::service::mpi_service::fantastic_4pc;
#else
using namespace tva::service::mpi_service::replicated_3pc;
#endif

int main(int argc, char** argv) {
    tva_init(argc, argv);

    // Testing the runtime environment
    const int64_t expSize = 4194;
    const int64_t dataRange = 4194304;
    const int64_t divisor = 1323;

    tva::Vector<int64_t> a_data(expSize, 0);
    for(int i = 0; i < expSize; ++i){
        a_data[i] = rand()%dataRange;
    }

    ASharedVector<int64_t> a = secret_share_a(a_data, 0);
    ASharedVector<int64_t> res = a.div(divisor);
    auto res_open = res.open();

    int wrongCount = 0;
    for(int i= 0; i < expSize; ++i){
        if(res_open[i] != (a_data[i]/divisor)){
            if(tva::service::runTime.getPartyID() == 0){
               std::cout << "real: " << a_data[i]/divisor << "\t" << "found: " << res_open[i] << std::endl;
            }
            wrongCount++;
        }
    }

    if(tva::service::runTime.getPartyID() == 0){
        std::cout << "wrong divisions: " << wrongCount << std::endl;
    }


#if defined(MPC_USE_MPI_COMMUNICATOR)
    MPI_Finalize();
#endif

    return 0;
}
