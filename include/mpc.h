#ifndef TVA_MPC_H
#define TVA_MPC_H

// Core - Containers
#include "core/containers/vector.h"
#include "core/containers/e_vector.h"

// Core - Communication
#include "core/communication/communicator.h"
#include "core/communication/mpi_communicator.h"

// Core - Random
#include "core/random/random_generator.h"
#include "core/random/pseudo_random_generator.h"

// Core - Protocols
#include "core/protocols/protocol.h"
#include "core/protocols/replicated_3pc.h"
#include "core/protocols/fantastic_4pc.h"

// Core - Shares
#include "core/containers/encoded_vector.h"
#include "core/containers/shared_vector.h"
#include "core/containers/a_shared_vector.h"
#include "core/containers/b_shared_vector.h"
#include "core/operators/operators.h"

// Relational
#include "relational/encoded_table.h"
#include "relational/shared_column.h"

// Macros Section
#define init_mpc_types(_Element_, _Vector_, _ReplicatedShare_, _EVector_, _Replication_)                            \
template <typename T>                                                                                               \
using ReplicatedShare = _ReplicatedShare_<T>;                                                                       \
                                                                                                                    \
template <typename T>                                                                                               \
using Vector = _Vector_<T>;                                                                                         \
                                                                                                                    \
template <typename T>                                                                                               \
using DataTable = std::vector<_Vector_<T>>;                                                                         \
                                                                                                                    \
template <typename T>                                                                                               \
using EVector = _EVector_<T, _Replication_>;                                                                        \
                                                                                                                    \
                                                                                                                    \
                                                                                                                    \
template<typename T>                                                                                                \
using ASharedVector = tva::ASharedVector<T, _EVector_<T, _Replication_>>;                                       \
                                                                                                                    \
template<typename T>                                                                                                \
using BSharedVector = tva::BSharedVector<T, _EVector_<T, _Replication_>>;                                       \
                                                                                                                    \
                                                                                                                    \
                                                                                                                    \
using EncodedColumn = relational::EncodedColumn;                                                                    \
                                                                                                                    \
template <typename T>                                                                                               \
using SharedColumn = relational::SharedColumn<T, _EVector_<T, _Replication_>>;                                      \
                                                                                                                    \
template<typename T>                                                                                                \
using EncodedTable = tva::relational::EncodedTable<T, SharedColumn<T>,                                          \
                        ASharedVector<T>, BSharedVector<T>, tva::EncodedVector, DataTable<T>>;                  \


#define init_mpc_system(_Communicator_, _RG_, _Protocol_)                                                           \
typedef _Communicator_ Communicator;                                                                                \
typedef _RG_ RG;                                                                                                    \
typedef _Protocol_ <int8_t, ReplicatedShare<int8_t>, Vector<int8_t>, EVector<int8_t>> Protocol_8;                   \
typedef _Protocol_ <int16_t, ReplicatedShare<int16_t>, Vector<int16_t>, EVector<int16_t>> Protocol_16;              \
typedef _Protocol_ <int32_t, ReplicatedShare<int32_t>, Vector<int32_t>, EVector<int32_t>> Protocol_32;              \
typedef _Protocol_ <int64_t, ReplicatedShare<int64_t>, Vector<int64_t>, EVector<int64_t>> Protocol_64;              \


#define init_mpc_functions(_Replication_)                                                                           \
template <typename T, typename...T2>                                                                                \
static EVector<T> secret_share_a(const tva::Vector<T> &data, const T2 & ... args){                              \
    return runTime.secret_share_a<_Replication_>(data, args...);                                                    \
}                                                                                                                   \
                                                                                                                    \
template <typename T, typename...T2>                                                                                \
static EVector<T> secret_share_b(const tva::Vector<T> &data, const T2 & ... args){                              \
    return runTime.secret_share_b<_Replication_>(data, args...);                                                    \
}                                                                                                                   \
                                                                                                                    \
template <typename T>                                                                                               \
static std::vector<std::shared_ptr<EncodedColumn>> secret_share(const DataTable<T> &data_table,                     \
                                                                const std::vector<std::string>& schema){            \
    return EncodedTable<T>:: template secret_share<T, _Replication_>(data_table, schema);                           \
}                                                                                                                   \
                                                                                                                    \
template <typename T>                                                                                               \
static BSharedVector<T> compare_rows(const std::vector<BSharedVector<T>*>& x_vec,                                   \
                                        const std::vector<BSharedVector<T>*>& y_vec,                                \
                                        const std::vector<bool>& inverse){                                          \
    return tva::compare_rows(x_vec, y_vec, inverse);                                                            \
}                                                                                                                   \
                                                                                                                    \
template <typename T>                                                                                               \
static void swap(std::vector<BSharedVector<T>*>& x_vec,                                                             \
                    std::vector<BSharedVector<T>*>& y_vec,                                                          \
                    const BSharedVector<T>& bits){                                                                  \
    tva::swap(x_vec, y_vec, bits);                                                                              \
}                                                                                                                   \
                                                                                                                    \
template <typename T>                                                                                               \
static Vector<T> compare_rows(const std::vector<Vector<T>*>& x_vec,                                                 \
                                const std::vector<Vector<T>*>& y_vec,                                               \
                                const std::vector<bool>& inverse){                                                  \
    return tva::compare_rows(x_vec, y_vec, inverse);                                                            \
}                                                                                                                   \
                                                                                                                    \
template <typename T>                                                                                               \
static void swap(std::vector<Vector<T>*>& x_vec,                                                                    \
                    std::vector<Vector<T>*>& y_vec,                                                                 \
                    const std::vector<bool>& bits){                                                                 \
    tva::swap(x_vec, y_vec, bits);                                                                              \
}                                                                                                                   \
                                                                                                                    \
template<typename T, typename V>                                                                                    \
static void bitonic_sort(std::vector<BSharedVector<T>*> _columns,                                                   \
                            const std::vector<bool> &desc) {                                                        \
    operators::bitonic_sort(_columns, desc);                                                                        \
}                                                                                                                   




#endif // TVA_MPC_H
