#ifndef TVA_OPERATORS_STREAMING_H
#define TVA_OPERATORS_STREAMING_H


#include "../containers/a_shared_vector.h"
#include "../containers/b_shared_vector.h"

#include "common.h"
#include "aggregation.h"


namespace tva { namespace operators {

    template<typename Share, typename EVector>
    static void tumbling_window(ASharedVector<Share, EVector>* key,
                         const Share& window_size,
                         ASharedVector<Share, EVector>* res){
        (*res) = key->div(window_size);
    }


    template<typename Share, typename EVector>
    static void tumbling_window(ASharedVector<Share, EVector> key,
                         const Share& window_size,
                         ASharedVector<Share, EVector> res){
        tumbling_window(&key, window_size, &res);
    }

    template<typename Share, typename EVector>
    void mark_gap_session(ASharedVector<Share, EVector>* timestamp,
                          BSharedVector<Share, EVector>* session_start,
                            const Share& gap){

        Vector<Share> one(1, 1);
        session_start->simple_subset_reference(0, 1, 1) =
                tva::service::runTime.secret_share_b<EVector::replicationNumber>(one, 0) & 1;


        Vector<Share> gap_vec(1, gap);
        ASharedVector<Share, EVector> shared_gap_vec = tva::service::runTime.secret_share_a<EVector::replicationNumber>(gap_vec, 0);
        auto shared_gap_vec_extended = shared_gap_vec.repeated_subset_reference(timestamp->size() - 1);


        ASharedVector<Share, EVector> pair_wise_gap = timestamp->simple_subset_reference(0, 1, timestamp->size() - 2)
                                                        + shared_gap_vec_extended
                                                        - timestamp->simple_subset_reference(1, 1);

        auto pair_wise_gap_b = pair_wise_gap.a2b();
        session_start->simple_subset_reference(1, 1) = pair_wise_gap_b->ltz();
    }


    template<typename Share, typename EVector>
    void gap_session_window(std::vector<BSharedVector<Share, EVector>*>& keys,
                            ASharedVector<Share, EVector>* timestamp_a,
                            BSharedVector<Share, EVector>* timestamp_b,
                            BSharedVector<Share, EVector>* window_id,
                            const Share& gap){

        mark_gap_session(timestamp_a, window_id, gap);


        Vector<Share> _one(1, -1);
        BSharedVector<Share, EVector> shared__one =
                tva::service::runTime.secret_share_b<EVector::replicationNumber>(_one, 0);

        (*window_id) = multiplex(*window_id, shared__one.repeated_subset_reference(window_id->size()), *timestamp_b);

        std::vector<BSharedVector<Share, EVector>*> r_vec({window_id});
        tva::operators::odd_even_aggregation(keys, r_vec, r_vec, &tva::operators::max_aggregation, true);
    }


    template<typename Share, typename EVector>
    void mark_threshold_session(BSharedVector<Share, EVector>* function_res,
                                BSharedVector<Share, EVector>* session_start,
                                BSharedVector<Share, EVector>* potential_window,
                                const Share& threshold){

        Vector<Share> threshold_vec(1, threshold);
        BSharedVector<Share, EVector> shared_threshold_vec =
                tva::service::runTime.secret_share_b<EVector::replicationNumber>(threshold_vec, 0);

        (*potential_window) = (*function_res) > shared_threshold_vec.repeated_subset_reference(function_res->size());

        session_start->simple_subset_reference(1, 1) =
                potential_window->simple_subset_reference(1, 1)
                & (potential_window->simple_subset_reference(1, 1)
                ^ potential_window->simple_subset_reference(0, 1, potential_window->size() - 2));
    }

    template<typename Share, typename EVector>
    void threshold_session_window(std::vector<BSharedVector<Share, EVector>*>& keys,
                                  BSharedVector <Share, EVector> *function_res,
                                  BSharedVector <Share, EVector> *timestamp_b,
                                  BSharedVector <Share, EVector> *window_id,
                                  const Share &gap) {

        BSharedVector<Share, EVector> potential_window(window_id->size());

        mark_threshold_session(function_res, window_id, &potential_window, gap);

        Vector<Share> _one(1, -1);
        BSharedVector<Share, EVector> shared__one =
                tva::service::runTime.secret_share_b<EVector::replicationNumber>(_one, 0);
        BSharedVector<Share, EVector> extended_shared__one = shared__one.repeated_subset_reference(window_id->size());

        (*window_id) = multiplex(*window_id, extended_shared__one, *timestamp_b);

        std::vector<BSharedVector<Share, EVector>*> r_vec({window_id});
        tva::operators::odd_even_aggregation(keys, r_vec, r_vec, &tva::operators::max_aggregation, true, &potential_window);
    }




} }


#endif //TVA_OPERATORS_STREAMING_H
