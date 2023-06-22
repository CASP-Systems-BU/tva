#ifndef TVA_MPI_SERVICE_SETUP_H
#define TVA_MPI_SERVICE_SETUP_H

#include "../../mpc.h"
#include "../common/runtime.h"

namespace tva{ namespace service{ namespace mpi_service { namespace fantastic_4pc{

            init_mpc_types(int, tva::Vector, std::vector, tva::EVector, 3)
            init_mpc_system(tva::MPICommunicator, tva::PseudoRandomGenerator, tva::Fantastic_4PC)
            init_mpc_functions(3)

            static void tva_init(int argc, char** argv){
                int rank;

#if defined(MPC_USE_MPI_COMMUNICATOR)
                int provided;

                // MPI_Init(&argc, &argv);
                MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
                if (provided != MPI_THREAD_MULTIPLE){
                    printf("Sorry, this MPI implementation does not support multiple threads\n");
                    MPI_Abort(MPI_COMM_WORLD, 1);
                }

                MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif

                // Set Batch Size / Number of Threads
                if(argc >= 2 ){
                    runTime.threads_num = atoi(argv[1]);
                }else{
                    runTime.threads_num = 1;
                }

                int p_factor;
                if (argc >= 3){
                    p_factor = atoi(argv[2]);
                }else{
                    p_factor = 1;
                }

                if (argc >= 4){
                    runTime.batch_size = atoi(argv[3]);
                }else{
                    runTime.batch_size = 10000;
                }

                
                runTime.setup_threads();

                for(int i = 0; i < runTime.threads_num; ++i){
                    runTime.communicators.push_back(std::unique_ptr<tva::Communicator>(
                            new Communicator(rank, 4, 700 + (i*10), p_factor)));

                    // TODO: exchange PRG Seeds here and send to constructor
                    runTime.randomGenerators.push_back(std::unique_ptr<tva::RandomGenerator>(
                            new RG(std::vector<unsigned short>({0, 0, 0}),
                                   std::vector<unsigned short>({0, 0, 0}),
                                   std::vector<unsigned short>({0, 0, 0}),
                                   std::vector<unsigned short>({0, 0, 0}))));

                    runTime.protocols_8.push_back(std::unique_ptr<tva::ProtocolBase>(
                            new Protocol_8(rank,
                                         runTime.communicators[i].get(),
                                         runTime.randomGenerators[i].get())));

                    runTime.protocols_32.push_back(std::unique_ptr<tva::ProtocolBase>(
                            new Protocol_32(rank,
                                         runTime.communicators[i].get(),
                                         runTime.randomGenerators[i].get())));   

                    runTime.protocols_64.push_back(std::unique_ptr<tva::ProtocolBase>(
                            new Protocol_64(rank,
                                         runTime.communicators[i].get(),
                                         runTime.randomGenerators[i].get())));
                }
            }
} } } } // namespace tva::service::mpi_service::fantastic_4pc


namespace tva{ namespace service { namespace mpi_service { namespace replicated_3pc{

    init_mpc_types(int, tva::Vector, std::vector, tva::EVector, 2)
    init_mpc_system(tva::MPICommunicator, tva::PseudoRandomGenerator, tva::Replicated_3PC)
    init_mpc_functions(2)
    static void tva_init(int argc, char** argv){
        int rank;

#if defined(MPC_USE_MPI_COMMUNICATOR)
        int provided;

        // MPI_Init(&argc, &argv);
        MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
        if (provided != MPI_THREAD_MULTIPLE){
            printf("Sorry, this MPI implementation does not support multiple threads\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif

        // Set Batch Size / Number of Threads
        if(argc >= 2 ){
            runTime.threads_num = atoi(argv[1]);
        }else{
            runTime.threads_num = 1;
        }

        int p_factor;
        if (argc >= 3){
            p_factor = atoi(argv[2]);
        }else{
            p_factor = 1;
        }

        if (argc >= 4){
            runTime.batch_size = atoi(argv[3]);
        }else{
            runTime.batch_size = 10000;
        }

        runTime.setup_threads();


        for(int i = 0; i < runTime.threads_num; ++i){
            runTime.communicators.push_back(std::unique_ptr<tva::Communicator>(
                    new Communicator(rank, 3, 700 + (i*10), p_factor)));

            // TODO: exchange PRG Seeds here and send to constructor
            runTime.randomGenerators.push_back(std::unique_ptr<tva::RandomGenerator>(
                    new RG(std::vector<unsigned short>({0, 0, 0}),
                           std::vector<unsigned short>({0, 0, 0}),
                           std::vector<unsigned short>({0, 0, 0}))));

            runTime.protocols_8.push_back(std::unique_ptr<tva::ProtocolBase>(
                    new Protocol_8(rank,
                                 runTime.communicators[i].get(),
                                 runTime.randomGenerators[i].get())));
            

            runTime.protocols_32.push_back(std::unique_ptr<tva::ProtocolBase>(
                    new Protocol_32(rank,
                                 runTime.communicators[i].get(),
                                 runTime.randomGenerators[i].get())));



            runTime.protocols_64.push_back(std::unique_ptr<tva::ProtocolBase>(
                    new Protocol_64(rank,
                                 runTime.communicators[i].get(),
                                 runTime.randomGenerators[i].get())));
        }

    }

} } } } // namespace tva::service::mpi_service::replicated_3pc

#endif // TVA_MPI_SERVICE_SETUP_H
