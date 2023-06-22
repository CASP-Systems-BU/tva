#ifndef TVA_SERVICE_RUNTIME_H
#define TVA_SERVICE_RUNTIME_H

#include <thread>
#include <sched.h>
#include <pthread.h>
#include <unistd.h>
#include <atomic>

#include "task.h"


// TODO:
//  1- In order to optimize this call, you have to make data a pointer
//  currently, the whole EVector input object is being copied for each call
//  because we are using EVector not EVector&. (Done)
//  2- Also, the whole EVector output is copied because of the protocol->func signature.
//  We should modify the function so that it takes the output and modify it.

// cpu_set_t cpuset;
// CPU_ZERO(&cpuset);
// CPU_SET(thread_ind_, &cpuset);
// int rc = pthread_setaffinity_np(threads[thread_ind_].native_handle(),
//                                 sizeof(cpu_set_t), &cpuset);
// printf("Thread #%d : on CPU %d\n", thread_ind_, sched_getcpu());


// Ideally we want to give this functions inputs, output, object to call the function on;
// for now let's use the protocols
#define runtime_evaluate_using_objects_2(_input_type_, _output_type_,                               \
                                        _object_type_, _objects_vector_,                            \
                                        _func_);                                                    \
template<int ReplicationNumber, typename...T>                                                       \
_output_type_                                                                                       \
_func_(_input_type_ x,                                                                              \
       _input_type_ y,                                                                              \
       const T & ... args) {                                                                        \
                                                                                                    \
    auto res = _output_type_ (x.total_size());                                                      \
                                                                                                    \
    for (int thread_ind = 0; thread_ind < threads_num; ++thread_ind) {                              \
        task_set_mutex[thread_ind]->lock();                                                         \
                                                                                                    \
        int start_ind, end_ind;                                                                     \
        getThreadBatchBoundary(x.total_size(), batch_size,                                          \
                                thread_ind, threads_num,                                            \
                                start_ind, end_ind);                                                \
                                                                                                    \
        tasks[thread_ind].reset();                                                                  \
                                                                                                    \
        tasks[thread_ind] = std::unique_ptr<Task>(                                                  \
            new Task_ARGS_RTR_2<_input_type_, _output_type_>(                                       \
                x, y, res,                                                                          \
                start_ind, end_ind, batch_size,                                                     \
                [=](_input_type_ &_x, _input_type_ &_y, _output_type_ &_res,                        \
                        const int &_start, const int &_end) {                                       \
                    _x.set_batch(_start, _end);                                                     \
                    _y.set_batch(_start, _end);                                                     \
                    _res.set_batch(_start, _end);                                                   \
                    _res = ((_object_type_ *) _objects_vector_[thread_ind].get())->                 \
                                _func_(_x, _y, args...);                                            \
                    return true;                                                                    \
                }                                                                                   \
        ));                                                                                         \
                                                                                                    \
        task_execute_mutex[thread_ind]->unlock();                                                   \
    }                                                                                               \
                                                                                                    \
    for(int i = 0; i < threads_num; ++i){                                                           \
          task_finished_mutex[i]->lock();                                                           \
    }                                                                                               \
                                                                                                    \
    return res;                                                                                     \
}                                                                                                   \



#define runtime_evaluate_using_objects_1(_input_type_, _output_type_,                               \
                                        _object_type_, _objects_vector_,                            \
                                        _func_);                                                    \
template<int ReplicationNumber, typename...T>                                                       \
_output_type_                                                                                       \
_func_ (_input_type_ x, const T & ... args) {                                                       \
                                                                                                    \
                                                                                                    \
    auto res = _output_type_(x.total_size());                                                       \
                                                                                                    \
    for (int thread_ind = 0; thread_ind < threads_num; ++thread_ind) {                              \
        task_set_mutex[thread_ind]->lock();                                                         \
                                                                                                    \
        int start_ind, end_ind;                                                                     \
        getThreadBatchBoundary(x.total_size(), batch_size,                                          \
                                thread_ind, threads_num,                                            \
                                start_ind, end_ind);                                                \
                                                                                                    \
        tasks[thread_ind].reset();                                                                  \
                                                                                                    \
        tasks[thread_ind] = std::unique_ptr<Task>(                                                  \
            new Task_ARGS_RTR_1<_input_type_, _output_type_>(                                       \
                x, res,                                                                             \
                start_ind, end_ind, batch_size,                                                     \
                [=](_input_type_ &_x, _output_type_ &_res, const int &_start, const int &_end) {    \
                    _x.set_batch(_start, _end);                                                     \
                    _res.set_batch(_start, _end);                                                   \
                    _res = ((_object_type_ *) _objects_vector_[thread_ind].get())                   \
                            ->_func_(_x, args...);                                                  \
                    return true;                                                                    \
                }                                                                                   \
        ));                                                                                         \
                                                                                                    \
        task_execute_mutex[thread_ind]->unlock();                                                   \
    }                                                                                               \
                                                                                                    \
    for(int i = 0; i < threads_num; ++i){                                                           \
          task_finished_mutex[i]->lock();                                                           \
    }                                                                                               \
                                                                                                    \
    return res;                                                                                     \
}                                                                                                   \


#define runtime_evaluate_using_objects_1_v(_input_type_, _output_type_,                             \
                                        _object_type_, _objects_vector_,                            \
                                        _func_, count_func_);                                       \
template<int ReplicationNumber, typename...T>                                                       \
std::vector<_output_type_>                                                                          \
_func_ (_input_type_ x, const T & ... args) {                                                       \
                                                                                                    \
    auto objects_count = ((_object_type_ *) _objects_vector_[0].get())->count_func_();              \
    auto res = std::vector<_output_type_>();                                                        \
    for (int i = 0; i < objects_count; ++i) {                                                       \
        res.push_back(_output_type_(x.total_size()));                                               \
    }                                                                                               \
                                                                                                    \
                                                                                                    \
    for (int thread_ind = 0; thread_ind < threads_num; ++thread_ind) {                              \
        task_set_mutex[thread_ind]->lock();                                                         \
                                                                                                    \
        int start_ind, end_ind;                                                                     \
        getThreadBatchBoundary(x.total_size(), batch_size,                                          \
                                thread_ind, threads_num,                                            \
                                start_ind, end_ind);                                                \
                                                                                                    \
        tasks[thread_ind].reset();                                                                  \
                                                                                                    \
        tasks[thread_ind] = std::unique_ptr<Task>(                                                  \
            new Task_ARGS_RTR_1<_input_type_, std::vector<_output_type_>>(                          \
                x, res,                                                                             \
                start_ind, end_ind, batch_size,                                                     \
                [=](_input_type_ &_x, std::vector<_output_type_> &_res,                             \
                                    const int &_start, const int &_end) {                           \
                    _x.set_batch(_start, _end);                                                     \
                                                                                                    \
                    auto __res = ((_object_type_ *) _objects_vector_[thread_ind].get())             \
                            ->_func_(_x, args...);                                                  \
                                                                                                    \
                    for(int i = 0; i < _res.size(); ++i){                                           \
                        _res[i].set_batch(_start, _end);                                            \
                        _res[i] = __res[i];                                                         \
                    }                                                                               \
                                                                                                    \
                    return true;                                                                    \
                }                                                                                   \
        ));                                                                                         \
                                                                                                    \
        task_execute_mutex[thread_ind]->unlock();                                                   \
    }                                                                                               \
                                                                                                    \
    for(int i = 0; i < threads_num; ++i){                                                           \
          task_finished_mutex[i]->lock();                                                           \
    }                                                                                               \
                                                                                                    \
    return res;                                                                                     \
}                                                                                                   \


#define EVector_macro_2(_share_type_, _replication_number_)                                         \
tva::EVector<_share_type_, _replication_number_>

#define EVector_macro_1(_share_type_)                                                               \
tva::EVector<_share_type_, ReplicationNumber>

#define ProtocolType_macro_2(_share_type_, _replication_number_)                                    \
Protocol<_share_type_, std::vector<_share_type_>,                                                   \
        Vector<_share_type_>,                                                                       \
        EVector<_share_type_, _replication_number_>>                                                \

#define ProtocolType_macro_1(_share_type_)                                                          \
Protocol<_share_type_, std::vector<_share_type_>,                                                   \
        Vector<_share_type_>,                                                                       \
        EVector<_share_type_,ReplicationNumber>>                                                    \


#define runtime_evaluate_using_objects_2_S_S(_share_type_, _replication_number_,                    \
                                            _object_type_, _objects_vector_,                        \
                                            _func_);                                                \
runtime_evaluate_using_objects_2(EVector_macro_1(_share_type_),                                     \
                                EVector_macro_1(_share_type_),                                      \
                                ProtocolType_macro_1(_share_type_),                                 \
                                _objects_vector_,                                                   \
                                _func_);                                                            \


#define runtime_evaluate_using_objects_2_V_S(_share_type_, _replication_number_,                    \
                                            _object_type_, _objects_vector_,                        \
                                            _func_);                                                \
runtime_evaluate_using_objects_2(tva::EVector<_share_type_>,                                    \
                                EVector_macro_1(_share_type_),                                      \
                                ProtocolType_macro_1(_share_type_),                                 \
                                _objects_vector_,                                                   \
                                _func_);                                                            \


#define runtime_evaluate_using_objects_2_S_V(_object_type_, _objects_vector_, _func_);              \
runtime_evaluate_using_objects_2(EVector_macro_1(_share_type_),                                     \
                            tva::EVector<_share_type_>,                                         \
                            ProtocolType_macro_1(_share_type_),                                     \
                            _objects_vector_,                                                       \
                            _func_);                                                                \


#define runtime_evaluate_using_objects_1_S_S(_share_type_, _replication_number_,                    \
                                            _object_type_, _objects_vector_,                        \
                                            _func_);                                                \
runtime_evaluate_using_objects_1(EVector_macro_1(_share_type_),                                     \
                                EVector_macro_1(_share_type_),                                      \
                                ProtocolType_macro_1(_share_type_),                                 \
                                _objects_vector_,                                                   \
                                _func_);                                                            \


#define runtime_evaluate_using_objects_1_V_S(_share_type_, _replication_number_,                    \
                                            _object_type_, _objects_vector_,                        \
                                            _func_);                                                \
runtime_evaluate_using_objects_1(tva::Vector<_share_type_>,                                     \
                                EVector_macro_1(_share_type_),                                      \
                                ProtocolType_macro_1(_share_type_),                                 \
                                _objects_vector_,                                                   \
                                _func_);                                                            \


#define runtime_evaluate_using_objects_1_S_V(_share_type_, _replication_number_,                    \
                                            _object_type_, _objects_vector_,                        \
                                            _func_);                                                \
runtime_evaluate_using_objects_1(EVector_macro_1(_share_type_),                                     \
                                tva::Vector<_share_type_>,                                      \
                                ProtocolType_macro_1(_share_type_),                                 \
                                _objects_vector_,                                                   \
                                _func_);                                                            \


#define runtime_declare_protocol_functions(_share_type_, _objects_vector_);                                                                                                 \
runtime_evaluate_using_objects_2_S_S(_share_type_, ReplicationNumber, ProtocolType_macro_1(_share_type_), _objects_vector_, add_a);                                         \
runtime_evaluate_using_objects_2_S_S(_share_type_, ReplicationNumber, ProtocolType_macro_1(_share_type_), _objects_vector_, sub_a);                                         \
runtime_evaluate_using_objects_2_S_S(_share_type_, ReplicationNumber, ProtocolType_macro_1(_share_type_), _objects_vector_, multiply_a);                                    \
                                                                                                                                                                            \
runtime_evaluate_using_objects_1_S_S(_share_type_, ReplicationNumber, ProtocolType_macro_1(_share_type_), _objects_vector_, neg_a);                                         \
                                                                                                                                                                            \
runtime_evaluate_using_objects_2_S_S(_share_type_, ReplicationNumber, ProtocolType_macro_1(_share_type_), _objects_vector_, xor_b);                                         \
runtime_evaluate_using_objects_2_S_S(_share_type_, ReplicationNumber, ProtocolType_macro_1(_share_type_), _objects_vector_, and_b);                                         \
                                                                                                                                                                            \
runtime_evaluate_using_objects_1_S_S(_share_type_, ReplicationNumber, ProtocolType_macro_1(_share_type_), _objects_vector_, not_b);                                         \
runtime_evaluate_using_objects_1_S_S(_share_type_, ReplicationNumber, ProtocolType_macro_1(_share_type_), _objects_vector_, not_b_1);                                       \
                                                                                                                                                                            \
runtime_evaluate_using_objects_1_S_S(_share_type_, ReplicationNumber, ProtocolType_macro_1(_share_type_), _objects_vector_, ltz);                                           \
runtime_evaluate_using_objects_1_S_S(_share_type_, ReplicationNumber, ProtocolType_macro_1(_share_type_), _objects_vector_, b2a_bit);                                           \
                                                                                                                                                                            \
runtime_evaluate_using_objects_1(tva::Vector<_share_type_>,                                                                                                             \
                                std::vector<EVector_macro_1(_share_type_)>,                                                                                                 \
                                ProtocolType_macro_1(_share_type_),                                                                                                         \
                                _objects_vector_,                                                                                                                           \
                                reconstruct_from_a);                                                                                                                        \
runtime_evaluate_using_objects_1(tva::Vector<_share_type_>,                                                                                                             \
                                std::vector<EVector_macro_1(_share_type_)>,                                                                                                 \
                                ProtocolType_macro_1(_share_type_),                                                                                                         \
                                _objects_vector_,                                                                                                                           \
                                reconstruct_from_b);                                                                                                                        \
                                                                                                                                                                            \
runtime_evaluate_using_objects_1_S_V(_share_type_, ReplicationNumber, ProtocolType_macro_1(_share_type_), _objects_vector_, open_shares_a);                                 \
runtime_evaluate_using_objects_1_S_V(_share_type_, ReplicationNumber, ProtocolType_macro_1(_share_type_), _objects_vector_, open_shares_b);                                 \
                                                                                                                                                                            \
runtime_evaluate_using_objects_1(std::vector<EVector_macro_1(_share_type_)>,                                                                                                \
                                tva::Vector<_share_type_>,                                                                                                              \
                                ProtocolType_macro_1(_share_type_),                                                                                                         \
                                _objects_vector_,                                                                                                                           \
                                get_shares_a);                                                                                                                              \
runtime_evaluate_using_objects_1(std::vector<EVector_macro_1(_share_type_)>,                                                                                                \
                                tva::Vector<_share_type_>,                                                                                                              \
                                ProtocolType_macro_1(_share_type_),                                                                                                         \
                                _objects_vector_,                                                                                                                           \
                                get_shares_b);                                                                                                                              \
runtime_evaluate_using_objects_1(tva::Vector<_share_type_>,                                                                                                             \
                                EVector_macro_1(_share_type_),                                                                                                              \
                                ProtocolType_macro_1(_share_type_),                                                                                                         \
                                _objects_vector_,                                                                                                                           \
                                secret_share_a);                                                                                                                            \
runtime_evaluate_using_objects_1(tva::Vector<_share_type_>,                                                                                                             \
                                EVector_macro_1(_share_type_),                                                                                                              \
                                ProtocolType_macro_1(_share_type_),                                                                                                         \
                                _objects_vector_,                                                                                                                           \
                                secret_share_b);                                                                                                                            \
                                                                                                                                                                            \
runtime_evaluate_using_objects_1_v(EVector_macro_1(_share_type_),                                                                                                           \
                                EVector_macro_1(_share_type_),                                                                                                              \
                                ProtocolType_macro_1(_share_type_),                                                                                                         \
                                _objects_vector_,                                                                                                                           \
                                div_const_a, div_const_a_count);                                                                                                            \
                                                                                                                                                                            \
runtime_evaluate_using_objects_1_v(EVector_macro_1(_share_type_),                                                                                                           \
                                EVector_macro_1(_share_type_),                                                                                                              \
                                ProtocolType_macro_1(_share_type_),                                                                                                         \
                                _objects_vector_,                                                                                                                           \
                                redistribute_shares_b, redistribute_shares_b_count);                                                                                        \
                                                                                                                                                                            \



// NOTE: What does runtime do?
//  1- Controls batch sizes.
//  2- Controls parallelism.
//  3- One runtime per execution.
//  4- Multiple Communicators/RGs per execution.
//  5- One Communicator and RG per thread.
//  5- Communicators/RGs live as long as the runtime live.
//  6- One Protocol Object per each datatype in each thread.


// TODO: to implement runtime
//  1- Make Vector's data a pointer so it persists in all threads.              >> Done
//  2- Modify the Vector to be working from just start = 0 to end = end.        >> Done
//  3- Add runtime inside ASharedVector / BSharedVector.                        >> Done
//  4- Add Batching to runtime.                                                 >> Done
//  5- Add Threading to runtime.                                                >> Done

namespace tva{ namespace service {

    static void getThreadBatchBoundary(const int &total_size, const int &batch_size,
                                       const int &thread_ind, const int &threads_num,
                                       int &start_ind, int &end_ind) {

        const int total_batches_number = total_size / batch_size;
        const int thread_batches_number = total_batches_number / threads_num;
        const int remaining_batches = total_batches_number % threads_num;

        const int this_thread_batches_number = thread_ind >= remaining_batches ?
                                               thread_batches_number : thread_batches_number + 1;

        const int previous_batches_number_inc = thread_ind >= remaining_batches ?
                                                remaining_batches : thread_ind;
        const int previous_batches_number = thread_ind * thread_batches_number
                                            + previous_batches_number_inc;

        start_ind = previous_batches_number * batch_size;
        end_ind = (thread_ind == (threads_num - 1)) ?
                  total_size :
                  start_ind + this_thread_batches_number * batch_size;
    }

    class RunTime{
    private:
    
    public:
        int batch_size;
        int threads_num;
        int remaining_connections;

        std::vector<std::unique_ptr<Communicator>> communicators;
        std::vector<std::unique_ptr<RandomGenerator>> randomGenerators;
        
        std::vector<std::unique_ptr<ProtocolBase>> protocols_8;
        std::vector<std::unique_ptr<ProtocolBase>> protocols_32;
        std::vector<std::unique_ptr<ProtocolBase>> protocols_64;

        std::mutex runtime_mutex;
        std::mutex setup_finished;
        std::mutex execution_finished;

        std::vector<std::thread> threads;
        std::vector<std::unique_ptr<Task>> tasks;
        std::vector<std::unique_ptr<std::mutex>> task_set_mutex;
        std::vector<std::unique_ptr<std::mutex>> task_execute_mutex;
        std::vector<std::unique_ptr<std::mutex>> task_finished_mutex;

        bool keep_running;

        ~RunTime(){
            keep_running = false;

            for(int i = 0; i < threads_num; ++i){
                task_execute_mutex[i]->unlock();
                threads[i].join();
            }
        }

        // TODO: Complete a generic parallelism function.
        // template<typename ShareType, int ReplicationNumber, typename ObjectType, typename... T>
        // EVector<ShareType, ReplicationNumber> threaded_execution(ObjectType *object,
        //                                                          EVector<ShareType, ReplicationNumber> x,
        //                                                          T... args) {

        // }

        void setup_threads(){
            keep_running = true;

            threads.resize(threads_num);
            tasks.resize(threads_num);
            task_set_mutex.resize(threads_num);
            task_execute_mutex.resize(threads_num);
            task_finished_mutex.resize(threads_num);
            
            std::shared_ptr<std::atomic<int>> counter = std::shared_ptr<std::atomic<int>>(new std::atomic<int>(threads_num));
            for (int i = 0; i < threads_num; ++i) {
                const int i_ = i;
                task_set_mutex[i_] = std::unique_ptr<std::mutex>(new std::mutex());
                task_execute_mutex[i_] = std::unique_ptr<std::mutex>(new std::mutex());
                task_finished_mutex[i_] = std::unique_ptr<std::mutex>(new std::mutex());
                threads[i_] = std::thread([this, i_, counter]() {
                    task_finished_mutex[i_]->lock();
                    task_execute_mutex[i_]->lock();
                    (*(counter.get()))--;
                    task_execute_mutex[i_]->lock();
                    while (keep_running) {
                        tasks[i_].get()->execute();
                        task_finished_mutex[i_]->unlock();
                        task_set_mutex[i_]->unlock();
                        task_execute_mutex[i_]->lock();
                    }
                });
            }

            while((*(counter.get()))){
                usleep(10);
            }
        }

        RunTime(const int& _batch_size = 10000,
                const int& _threads_num = 1):
                batch_size(_batch_size),
                threads_num(_threads_num){}

        runtime_declare_protocol_functions(int8_t, protocols_8);
        runtime_declare_protocol_functions(int32_t, protocols_32);
        runtime_declare_protocol_functions(int64_t, protocols_64);

        // &_object_type_::_func_
        template<typename ObjectType, typename...T>
        ObjectType
        execute_parallel (const ObjectType& x , ObjectType (ObjectType::*func)(const T &...) const, const T & ... args) {


            auto res = ObjectType(x.total_size());

            for (int thread_ind = 0; thread_ind < threads_num; ++thread_ind) {
                task_set_mutex[thread_ind]->lock();

                int start_ind, end_ind;
                getThreadBatchBoundary(x.total_size(), batch_size,
                                        thread_ind, threads_num,
                                        start_ind, end_ind);

                tasks[thread_ind].reset();

                tasks[thread_ind] = std::unique_ptr<Task>(
                        new Task_ARGS_RTR_1<ObjectType, ObjectType>(
                                x, res,
                                start_ind, end_ind, batch_size,
                                [=](ObjectType &_x, ObjectType &_res, const int &_start, const int &_end) {
                                    _x.set_batch(_start, _end);
                                    _res.set_batch(_start, _end);
                                    _res = (_x.*func)(args...);
                                    return true;
                                }
                        ));

                task_execute_mutex[thread_ind]->unlock();
            }

            for(int i = 0; i < threads_num; ++i){
                task_finished_mutex[i]->lock();
            }

            return res;
        }

        template<typename ObjectType, typename...T1, typename...T2>
        void
        modify_parallel (const ObjectType& x , void (ObjectType::*func)( const T1 &..., const T2 &...), const T1 & ... args1, const T2 & ... args2) {

            for (int thread_ind = 0; thread_ind < threads_num; ++thread_ind) {
                task_set_mutex[thread_ind]->lock();

                int start_ind, end_ind;
                getThreadBatchBoundary(x.total_size(), batch_size,
                                       thread_ind, threads_num,
                                       start_ind, end_ind);

                tasks[thread_ind].reset();

                tasks[thread_ind] = std::unique_ptr<Task>(
                        new Task_ARGS_VOID_1<ObjectType>(
                                x,
                                start_ind, end_ind, batch_size,
                                [=](ObjectType& _x, const int &_start, const int &_end) {
                                    _x.set_batch(_start, _end);
                                    (_x.*func)(args1..., args2...);
                                    return true;
                                }
                        ));

                task_execute_mutex[thread_ind]->unlock();
            }

            for(int i = 0; i < threads_num; ++i){
                task_finished_mutex[i]->lock();
            }
        }


        template<typename InputType, typename...T2>
        void
        communicate_parallel (void (tva::Communicator::*func)(InputType, T2 ...), InputType & input_1, const T2 & ... args2) {

            for (int thread_ind = 0; thread_ind < threads_num; ++thread_ind) {
                task_set_mutex[thread_ind]->lock();

                int start_ind, end_ind;
                getThreadBatchBoundary(input_1.size(), batch_size,
                                       thread_ind, threads_num,
                                       start_ind, end_ind);

                tasks[thread_ind].reset();

                tasks[thread_ind] = std::unique_ptr<Task>(
                        new Task_ARGS_VOID_1<InputType>(
                                input_1,
                                start_ind, end_ind, batch_size,
                                [=](InputType& _x, const int &_start, const int &_end) {
                                    _x.set_batch(_start, _end);
                                    (((tva::Communicator*)(this->communicators[thread_ind].get()))->*func)(_x, args2...);
                                    return true;
                                }
                        ));

                task_execute_mutex[thread_ind]->unlock();
            }

            for(int i = 0; i < threads_num; ++i){
                task_finished_mutex[i]->lock();
            }
        }

        template<typename InputType, typename...T2>
        void
        communicate_parallel (void (tva::Communicator::*func)(InputType&, T2 ...), InputType & input_1, const T2 & ... args2) {

            for (int thread_ind = 0; thread_ind < threads_num; ++thread_ind) {
                task_set_mutex[thread_ind]->lock();

                int start_ind, end_ind;
                getThreadBatchBoundary(input_1.size(), batch_size,
                                       thread_ind, threads_num,
                                       start_ind, end_ind);

                tasks[thread_ind].reset();

                tasks[thread_ind] = std::unique_ptr<Task>(
                        new Task_ARGS_VOID_1<InputType>(
                                input_1,
                                start_ind, end_ind, batch_size,
                                [=](InputType& _x, const int &_start, const int &_end) {
                                    _x.set_batch(_start, _end);
                                    (((tva::Communicator*)(this->communicators[thread_ind].get()))->*func)(_x, args2...);
                                    return true;
                                }
                        ));

                task_execute_mutex[thread_ind]->unlock();
            }

            for(int i = 0; i < threads_num; ++i){
                task_finished_mutex[i]->lock();
            }
        }


        template<typename InputType, typename...T2>
        void
        communicate_parallel (void (tva::Communicator::*func)(InputType, InputType &, T2 ...), InputType& input_1, InputType & input_2, const T2 & ... args2) {

            for (int thread_ind = 0; thread_ind < threads_num; ++thread_ind) {
                task_set_mutex[thread_ind]->lock();

                int start_ind, end_ind;
                getThreadBatchBoundary(input_1.size(), batch_size,
                                       thread_ind, threads_num,
                                       start_ind, end_ind);

                tasks[thread_ind].reset();

                tasks[thread_ind] = std::unique_ptr<Task>(
                        new Task_ARGS_VOID_2<InputType>(
                                input_1, input_2,
                                start_ind, end_ind, batch_size,
                                [=](InputType& _x, InputType& _y, const int &_start, const int &_end) {
                                    _x.set_batch(_start, _end);
                                    _y.set_batch(_start, _end);
                                    (((tva::Communicator*)(this->communicators[thread_ind].get()))->*func)(_x, _y, args2...);
                                    return true;
                                }
                        ));

                task_execute_mutex[thread_ind]->unlock();
            }

            for(int i = 0; i < threads_num; ++i){
                task_finished_mutex[i]->lock();
            }
        }


        template<typename InputType, typename...T2>
        void
        generate_parallel (void (tva::RandomGenerator::*func)(InputType &, T2 ...), InputType & input_1, const T2 & ... args2) {

            for (int thread_ind = 0; thread_ind < threads_num; ++thread_ind) {
                task_set_mutex[thread_ind]->lock();

                int start_ind, end_ind;
                getThreadBatchBoundary(input_1.size(), batch_size,
                                       thread_ind, threads_num,
                                       start_ind, end_ind);

                tasks[thread_ind].reset();

                tasks[thread_ind] = std::unique_ptr<Task>(
                        new Task_ARGS_VOID_1<InputType>(
                                input_1,
                                start_ind, end_ind, batch_size,
                                [=](InputType& _x, const int &_start, const int &_end) {
                                    _x.set_batch(_start, _end);
                                    (((tva::RandomGenerator*)(this->randomGenerators[thread_ind].get()))->*func)(_x, args2...);
                                    return true;
                                }
                        ));

                task_execute_mutex[thread_ind]->unlock();
            }

            for(int i = 0; i < threads_num; ++i){
                task_finished_mutex[i]->lock();
            }
        }


        template<typename InputType, typename...T2>
        void
        generate_parallel (void (tva::RandomGenerator::*func)(InputType, InputType &, T2 ...), InputType& input_1, InputType & input_2, const T2 & ... args2) {

            for (int thread_ind = 0; thread_ind < threads_num; ++thread_ind) {
                task_set_mutex[thread_ind]->lock();

                int start_ind, end_ind;
                getThreadBatchBoundary(input_1.size(), batch_size,
                                       thread_ind, threads_num,
                                       start_ind, end_ind);

                tasks[thread_ind].reset();

                tasks[thread_ind] = std::unique_ptr<Task>(
                        new Task_ARGS_VOID_2<InputType>(
                                input_1, input_2,
                                start_ind, end_ind, batch_size,
                                [=](InputType& _x, InputType& _y, const int &_start, const int &_end) {
                                    _x.set_batch(_start, _end);
                                    _y.set_batch(_start, _end);
                                    (((tva::RandomGenerator*)(this->randomGenerators[thread_ind].get()))->*func)(_x, _y, args2...);
                                    return true;
                                }
                        ));

                task_execute_mutex[thread_ind]->unlock();
            }

            for(int i = 0; i < threads_num; ++i){
                task_finished_mutex[i]->lock();
            }
        }





        int getPartyID() const {
            return protocols_32[0].get()->partyID;
        }

        int getReplicationNumber() const {
            return protocols_32[0].get()->replicationNumber;
        }


        // How to calculate start and end for each thread:
        // assume the vector have size n .. batch size m .. number of threads t
        // first calculate number of batches n/m
        // calculate number of batches per thread n/m/t
        // calculate remaining batches ... (n/m)%t
        // threads_1 = (n/m)%t
        // threads_2 = t - threads_1 - 1
        // size_1 = m *(n/m/t + 1)
        // size_2 = size_1 - m
        // for first threads_1 threads, put start = i * size_1  ... put end = (i+1) * size_1
        // for next threads_2



    };

    static RunTime runTime(50000, 1);

} }  // namespace tva::service


#endif // TVA_SERVICE_RUNTIME_H
