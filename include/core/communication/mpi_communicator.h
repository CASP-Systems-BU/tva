#ifndef TVA_MPI_COMMUNICATOR_H
#define TVA_MPI_COMMUNICATOR_H

#include "../../debug/debug.h"
#include "communicator.h"

#if defined(MPC_USE_MPI_COMMUNICATOR)
#include "mpi.h"
#endif

namespace tva {

    // TODO:
    // 1 - Send and receive in parallel. >> Done
    // 2 - Add parallelism_factor. >> Done
    // 3 - Remove the code redundancy to choose between (int8_t, int32_t, int64_t)

    class MPICommunicator : public Communicator {
        int numParties;
        int msg_tag;
        int parallelism_factor;

    public:

        MPICommunicator(const int &_currentId, const int &_numParties,
                        const int &_msg_tag = 7, const int &_parallelism_factor = 1)
                : Communicator(_currentId),
                  numParties(_numParties), msg_tag(_msg_tag),
                  parallelism_factor(_parallelism_factor) {

#if defined(MPC_USE_MPI_COMMUNICATOR)
            int __rank, __num_parties;

            MPI_Comm_rank(MPI_COMM_WORLD, &__rank);
            MPI_Comm_size(MPI_COMM_WORLD, &__num_parties);

            // this protocol works with 3 parties only
            if (__rank == 0 && __num_parties != numParties) {
                fprintf(stderr, "ERROR: The number of MPI processes must be %d not %d\n", numParties, __num_parties);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
#endif
        }

        ~MPICommunicator() {}

        void sendShares(int8_t share, PartyID _id) {
            int ind = (numParties + _id + this->currentId) % numParties;
#if defined(MPC_USE_MPI_COMMUNICATOR)
            MPI_Send(&share, 1, MPI_INT8_T, ind, msg_tag, MPI_COMM_WORLD);
#endif
        }

        void sendShares(int32_t share, PartyID _id) {
            int ind = (numParties + _id + this->currentId) % numParties;
#if defined(MPC_USE_MPI_COMMUNICATOR)
            MPI_Send(&share, 1, MPI_INT32_T, ind, msg_tag, MPI_COMM_WORLD);
#endif
        }

        void sendShares(int64_t share, PartyID _id) {
            int ind = (numParties + _id + this->currentId) % numParties;
#if defined(MPC_USE_MPI_COMMUNICATOR)
            MPI_Send(&share, 1, MPI_INT64_T, ind, msg_tag, MPI_COMM_WORLD);
#endif
        }

        void sendShares(Vector<int8_t> _shares, PartyID _id, int _size) {
            int to_ind = (numParties + _id + this->currentId) % numParties;
#if defined(MPC_USE_MPI_COMMUNICATOR)
            std::vector<MPI_Request> requests;

            int start = 0;
            int __size = _size / parallelism_factor;
            for (int i = 0; i < parallelism_factor; ++i) {
                int ___size = (i == parallelism_factor - 1) ? _size - start : __size;

                requests.push_back(MPI_Request());

                MPI_Isend(&_shares[start], ___size, MPI_INT8_T, to_ind, msg_tag + i, MPI_COMM_WORLD, &requests[i]);
                start += ___size;
            }

            for (uint i = 0; i < requests.size(); ++i) {
                MPI_Wait(&requests[i], MPI_STATUS_IGNORE);
            }
#endif
        }

        void sendShares(Vector<int32_t> _shares, PartyID _id, int _size) {
            int to_ind = (numParties + _id + this->currentId) % numParties;
#if defined(MPC_USE_MPI_COMMUNICATOR)
            std::vector<MPI_Request> requests;

            int start = 0;
            int __size = _size / parallelism_factor;
            for (int i = 0; i < parallelism_factor; ++i) {
                int ___size = (i == parallelism_factor - 1) ? _size - start : __size;

                requests.push_back(MPI_Request());

                MPI_Isend(&_shares[start], ___size, MPI_INT32_T, to_ind, msg_tag + i, MPI_COMM_WORLD, &requests[i]);
                start += ___size;
            }

            for (uint i = 0; i < requests.size(); ++i) {
                MPI_Wait(&requests[i], MPI_STATUS_IGNORE);
            }
#endif
        }

        void sendShares(Vector<int64_t> _shares, PartyID _id, int _size) {
            int to_ind = (numParties + _id + this->currentId) % numParties;
#if defined(MPC_USE_MPI_COMMUNICATOR)
            std::vector<MPI_Request> requests;

            int start = 0;
            int __size = _size / parallelism_factor;
            for (int i = 0; i < parallelism_factor; ++i) {
                int ___size = (i == parallelism_factor - 1) ? _size - start : __size;

                requests.push_back(MPI_Request());

                MPI_Isend(&_shares[start], ___size, MPI_INT64_T, to_ind, msg_tag + i, MPI_COMM_WORLD, &requests[i]);
                start += ___size;
            }

            for (uint i = 0; i < requests.size(); ++i) {
                MPI_Wait(&requests[i], MPI_STATUS_IGNORE);
            }
#endif
        }

        void receiveShare(int8_t &_share, PartyID _id) {
            int ind = (numParties + _id + this->currentId) % numParties;
#if defined(MPC_USE_MPI_COMMUNICATOR)
            MPI_Recv(&_share, 1, MPI_INT8_T, ind, msg_tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
#endif
        }

        void receiveShare(int32_t &_share, PartyID _id) {
            int ind = (numParties + _id + this->currentId) % numParties;
#if defined(MPC_USE_MPI_COMMUNICATOR)
            MPI_Recv(&_share, 1, MPI_INT32_T, ind, msg_tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
#endif
        }

        void receiveShare(int64_t &_share, PartyID _id) {
            int ind = (numParties + _id + this->currentId) % numParties;
#if defined(MPC_USE_MPI_COMMUNICATOR)
            MPI_Recv(&_share, 1, MPI_INT64_T, ind, msg_tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
#endif
        }

        void receiveShares(Vector<int8_t> &_shareVector, PartyID _id, int _size) {
#if defined(MPC_USE_MPI_COMMUNICATOR)
            std::vector<MPI_Request> requests;

            int from_ind = (numParties + _id + this->currentId) % numParties;

            int start = 0;
            int __size = _size / parallelism_factor;
            for (int i = 0; i < parallelism_factor; ++i) {
                int ___size = (i == parallelism_factor - 1) ? _size - start : __size;
                requests.push_back(MPI_Request());

                MPI_Irecv(&_shareVector[start], ___size, MPI_INT8_T, from_ind, msg_tag + i, MPI_COMM_WORLD,
                          &requests[i]);
                start += ___size;
            }

            for (uint i = 0; i < requests.size(); ++i) {
                MPI_Wait(&requests[i], MPI_STATUS_IGNORE);
            }
#endif
        }

        void receiveShares(Vector<int32_t> &_shareVector, PartyID _id, int _size) {
#if defined(MPC_USE_MPI_COMMUNICATOR)
            std::vector<MPI_Request> requests;

            int from_ind = (numParties + _id + this->currentId) % numParties;

            int start = 0;
            int __size = _size / parallelism_factor;
            for (int i = 0; i < parallelism_factor; ++i) {
                int ___size = (i == parallelism_factor - 1) ? _size - start : __size;
                requests.push_back(MPI_Request());

                MPI_Irecv(&_shareVector[start], ___size, MPI_INT32_T, from_ind, msg_tag + i, MPI_COMM_WORLD,
                          &requests[i]);
                start += ___size;
            }

            for (uint i = 0; i < requests.size(); ++i) {
                MPI_Wait(&requests[i], MPI_STATUS_IGNORE);
            }
#endif
        }

        void receiveShares(Vector<int64_t> &_shareVector, PartyID _id, int _size) {
#if defined(MPC_USE_MPI_COMMUNICATOR)
            std::vector<MPI_Request> requests;

            int from_ind = (numParties + _id + this->currentId) % numParties;

            int start = 0;
            int __size = _size / parallelism_factor;
            for (int i = 0; i < parallelism_factor; ++i) {
                int ___size = (i == parallelism_factor - 1) ? _size - start : __size;
                requests.push_back(MPI_Request());

                MPI_Irecv(&_shareVector[start], ___size, MPI_INT64_T, from_ind, msg_tag + i, MPI_COMM_WORLD,
                          &requests[i]);
                start += ___size;
            }

            for (uint i = 0; i < requests.size(); ++i) {
                MPI_Wait(&requests[i], MPI_STATUS_IGNORE);
            }
#endif
        }

        void exchangeShares(Vector<int8_t> sent_shares, Vector<int8_t>& received_shares, PartyID _id, int _size) {
            exchangeShares(sent_shares, received_shares, _id, _id, _size);
        }

        void exchangeShares(Vector<int32_t> sent_shares, Vector<int32_t>& received_shares, PartyID _id, int _size) {
            exchangeShares(sent_shares, received_shares, _id, _id, _size);
        }

        void exchangeShares(Vector<int64_t> sent_shares, Vector<int64_t>& received_shares, PartyID _id, int _size) {
            exchangeShares(sent_shares, received_shares, _id, _id, _size);
        }

        void exchangeShares(Vector<int8_t> sent_shares, Vector<int8_t>& received_shares, PartyID to_id, PartyID from_id, int _size) {
            int to_ind = (numParties + to_id + this->currentId) % numParties;
            int from_ind = (numParties + from_id + this->currentId) % numParties;

#if defined(MPC_USE_MPI_COMMUNICATOR)
            std::vector<MPI_Request> requests;

            int start = 0;
            int __size = _size / parallelism_factor;
            for (int i = 0; i < parallelism_factor; ++i) {
                int ___size = (i == parallelism_factor - 1) ? _size - start : __size;

                requests.push_back(MPI_Request());
                requests.push_back(MPI_Request());

                MPI_Irecv(&received_shares[start], ___size, MPI_INT8_T, from_ind, msg_tag + i, MPI_COMM_WORLD,
                          &requests[2 * i]);
                MPI_Isend(&sent_shares[start], ___size, MPI_INT8_T, to_ind, msg_tag + i, MPI_COMM_WORLD,
                          &requests[2 * i + 1]);
                start += ___size;
            }

            for (uint i = 0; i < requests.size(); ++i) {
                MPI_Wait(&requests[i], MPI_STATUS_IGNORE);
            }
#endif
        }

        void exchangeShares(Vector<int32_t> sent_shares, Vector<int32_t>& received_shares, PartyID to_id, PartyID from_id, int _size) {
            int to_ind = (numParties + to_id + this->currentId) % numParties;
            int from_ind = (numParties + from_id + this->currentId) % numParties;

#if defined(MPC_USE_MPI_COMMUNICATOR)
            std::vector<MPI_Request> requests;

            int start = 0;
            int __size = _size / parallelism_factor;
            for (int i = 0; i < parallelism_factor; ++i) {
                int ___size = (i == parallelism_factor - 1) ? _size - start : __size;

                requests.push_back(MPI_Request());
                requests.push_back(MPI_Request());

                MPI_Irecv(&received_shares[start], ___size, MPI_INT32_T, from_ind, msg_tag + i, MPI_COMM_WORLD,
                          &requests[2 * i]);
                MPI_Isend(&sent_shares[start], ___size, MPI_INT32_T, to_ind, msg_tag + i, MPI_COMM_WORLD,
                          &requests[2 * i + 1]);
                start += ___size;
            }

            for (uint i = 0; i < requests.size(); ++i) {
                MPI_Wait(&requests[i], MPI_STATUS_IGNORE);
            }
#endif
        }

        void exchangeShares(Vector<int64_t> sent_shares, Vector<int64_t>& received_shares, PartyID to_id, PartyID from_id, int _size) {
            int to_ind = (numParties + to_id + this->currentId) % numParties;
            int from_ind = (numParties + from_id + this->currentId) % numParties;

#if defined(MPC_USE_MPI_COMMUNICATOR)
            std::vector<MPI_Request> requests;

            int start = 0;
            int __size = _size / parallelism_factor;
            for (int i = 0; i < parallelism_factor; ++i) {
                int ___size = (i == parallelism_factor - 1) ? _size - start : __size;

                requests.push_back(MPI_Request());
                requests.push_back(MPI_Request());

                MPI_Irecv(&received_shares[start], ___size, MPI_INT64_T, from_ind, msg_tag + i, MPI_COMM_WORLD,
                          &requests[2 * i]);
                MPI_Isend(&sent_shares[start], ___size, MPI_INT64_T, to_ind, msg_tag + i, MPI_COMM_WORLD,
                          &requests[2 * i + 1]);
                start += ___size;
            }

            for (uint i = 0; i < requests.size(); ++i) {
                MPI_Wait(&requests[i], MPI_STATUS_IGNORE);
            }
#endif
        }
    };
}

#endif //TVA_MPI_COMMUNICATOR_H
