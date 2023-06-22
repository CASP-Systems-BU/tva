#ifndef TVA_OPERATORS_AGGREGATION_H
#define TVA_OPERATORS_AGGREGATION_H

#include "../containers/a_shared_vector.h"
#include "../containers/b_shared_vector.h"

#include "common.h"

namespace tva { namespace operators {
    typedef enum {
        SUM,
        MAX,
        MIN,
        COUNT,
        EXTRA
    } AggregationType;

    template<typename Share, typename EVector>
    static void sum_aggregation(const int& d,
                                const ASharedVector<Share, EVector>& group_bits_a,
                                std::vector<ASharedVector<Share, EVector>*> results,
                                const bool& reverse = false){

        auto total_size = results[0]->size();
        for(int j = 0; j < results.size(); ++j){
            ASharedVector<Share, EVector> first_vector = results[j]->simple_subset_reference(0, 1, total_size - d - 1);
            ASharedVector<Share, EVector> second_vector = results[j]->simple_subset_reference(d);

            if(reverse){
                second_vector = second_vector + group_bits_a * first_vector;
            } else{
                first_vector = first_vector + group_bits_a * second_vector;
            }

        }
    }

    template<typename Share, typename EVector>
    static void max_aggregation( const int& d,
                                 const BSharedVector<Share, EVector>& group_bits,
                                 std::vector<BSharedVector<Share, EVector>*> results,
                                 const bool& reverse = false,
                                 const bool& min_aggr = false){

        auto total_size = results[0]->size();
        for(int j = 0; j < results.size(); ++j){
            BSharedVector<Share, EVector> first_vector = results[j]->simple_subset_reference(0, 1, total_size - d - 1);
            BSharedVector<Share, EVector> second_vector = results[j]->simple_subset_reference(d);
            BSharedVector<Share, EVector> second_greater = second_vector > first_vector;

            if(min_aggr){
                second_greater = !second_greater;
            }

            if(reverse){
                second_vector = multiplex(group_bits, second_vector, multiplex(second_greater, first_vector, second_vector));
            } else{
                first_vector = multiplex(group_bits, first_vector, multiplex(second_greater, first_vector, second_vector));
            }
        }
    }

    template<typename Share, typename EVector>
    static void max_aggregation( const int& d,
                                 const BSharedVector<Share, EVector>& group_bits,
                                 std::vector<BSharedVector<Share, EVector>*> results,
                                 const bool& reverse = false){
        max_aggregation(d, group_bits, results, reverse, false);
    }

    template<typename Share, typename EVector>
    static void min_aggregation( const int& d,
                                 const BSharedVector<Share, EVector>& group_bits,
                                 std::vector<BSharedVector<Share, EVector>*> results,
                                 const bool& reverse = false){
        max_aggregation(d, group_bits, results, reverse, true);
    }

    template<typename Share, typename EVector, typename SVector, typename...T>
    static void odd_even_aggregation(std::vector<BSharedVector<Share, EVector>*>& keys,
                                     std::vector<SVector*>& values,
                                     std::vector<SVector*>& results,
                                     void(*func)(const int&,
                                             const SVector&, std::vector<SVector*>,
                                             const bool&),
                                     const bool& reverse = false,
                                     BSharedVector<Share, EVector>* sel_b = NULL){
        assert(values.size() > 0);
        assert(results.size() > 0);

        // If values and results point to two different objects
        // then do deep assignment
        for(int i = 0; i < values.size(); ++i){
            if(values[i] != results[i]){
                (*results[i]) = (*values[i]);
            }
        }

        auto total_size = values[0]->size();

        // TODO: check that in multiplex and just avoid the step
        //  - Normally, multiplexing using secret shares uses a secure and.
        //  - However, if we are multiplexing using public one, we can use plaintext.
        Vector<Share> one(1, 1);
        BSharedVector<Share, EVector> shared_one =
                tva::service::runTime.secret_share_b<EVector::replicationNumber>(one, 0);


        for(int i = 1; i <= (log2(total_size)); ++i){
            int d = total_size /(std::pow(2,i));
            if(reverse){
                d = total_size /(std::pow(2, (log2(total_size)) - i + 1));
            }

            BSharedVector<Share, EVector> group_bits(total_size - d);
            if (keys.size() == 0){
                group_bits = shared_one.repeated_subset_reference(group_bits.size());
            }else{
                BSharedVector<Share, EVector> first_vector = keys[0]->simple_subset_reference(0, 1, total_size - d - 1);
                BSharedVector<Share, EVector> second_vector = keys[0]->simple_subset_reference(d);
                group_bits = first_vector == second_vector;
            }

            // for remaining columns
            for(int j = 1; j < keys.size(); ++j){
                BSharedVector<Share, EVector> first_vector_ = keys[j]->simple_subset_reference(0, 1, total_size - d - 1);
                BSharedVector<Share, EVector> second_vector_ = keys[j]->simple_subset_reference(d);
                BSharedVector<Share, EVector> group_bits_ = first_vector_ == second_vector_;
                group_bits = group_bits & group_bits_;
            }

            if(sel_b != NULL){
                group_bits = group_bits
                        & sel_b->simple_subset_reference(0, 1, total_size - d - 1)
                        & sel_b->simple_subset_reference(d);
            }

            SVector group_bits_a (group_bits.size());
            auto_conversion(group_bits, group_bits_a);
            func(d, group_bits_a, results, reverse);
        }
    }

    template<typename Share, typename EVector, typename SVector>
    static void odd_even_aggregation(std::vector<BSharedVector<Share, EVector>> keys,
                                        std::vector<SVector> values,
                                        std::vector<SVector> results,
                                        void(*func)(const int&,
                                                const SVector&, std::vector<SVector*>,
                                                const bool&),
                                        const bool& reverse = false){

        std::vector<BSharedVector<Share, EVector>*> keys_ptr;
        std::vector<SVector*> values_ptr;
        std::vector<SVector*> results_ptr;

        for(int i =0; i < keys.size(); ++i){
            keys_ptr.push_back(&keys[i]);
        }

        for(int i =0; i < values.size(); ++i){
            values_ptr.push_back(&values[i]);
        }

        for(int i =0; i < results.size(); ++i){
            results_ptr.push_back(&results[i]);
        }

        odd_even_aggregation<Share, EVector, SVector>(keys_ptr, values_ptr, results_ptr, func, reverse);
    }

} }



#endif //TVA_OPERATORS_AGGREGATION_H
