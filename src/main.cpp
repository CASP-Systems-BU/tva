#include "../include/tva.h"

#ifdef USE_FANTASTIC_FOUR
using namespace tva::service::mpi_service::fantastic_4pc;
#else
using namespace tva::service::mpi_service::replicated_3pc;
#endif

int main(int argc, char** argv) {
    tva_init(argc, argv);

    // Testing the runtime environment
    Vector<int32_t> a(10), b(10);
    Vector<int32_t> c = a * b;

    ASharedVector<int32_t> a_(10), b_(10);
    ASharedVector<int32_t> c_ = a_ * b_;

#if defined(MPC_USE_MPI_COMMUNICATOR)
    MPI_Finalize();
#endif

    return 0;
}
