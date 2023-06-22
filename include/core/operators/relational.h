#ifndef TVA_OPERATORS_RELATIONAL_H
#define TVA_OPERATORS_RELATIONAL_H


#include "../containers/a_shared_vector.h"
#include "../containers/b_shared_vector.h"

#include "common.h"


namespace tva { namespace operators {

    template<typename Share, typename EVector>
    static void distinct(std::vector<BSharedVector<Share, EVector>*>& keys,
                            BSharedVector<Share, EVector>* res){
        assert(keys.size() > 0);

        BSharedVector<Share, EVector> zero_index = res->simple_subset_reference(0, 1, 0);
        Vector<Share> zero(1, 1);
        zero_index = tva::service::runTime.secret_share_b<EVector::replicationNumber>(zero, 0) & 1;


        BSharedVector<Share, EVector> not_zero_index = res->simple_subset_reference(1, 1);
        BSharedVector<Share, EVector> first_vector = keys[0]->simple_subset_reference(0, 1, keys[0]->size() - 2);
        BSharedVector<Share, EVector> second_vector = keys[0]->simple_subset_reference(1, 1);
        not_zero_index = first_vector != second_vector;



        for(int i=1; i < keys.size(); ++i){
            BSharedVector<Share, EVector> first_vector_ = keys[i]->simple_subset_reference(0, 1, keys[i]->size() - 2);
            BSharedVector<Share, EVector> second_vector_ = keys[i]->simple_subset_reference(1, 1);

            not_zero_index = not_zero_index & (first_vector_ != second_vector_);
        }
    }

    template<typename Share, typename EVector>
    static void distinct(std::vector<BSharedVector<Share, EVector>>& keys,
                            BSharedVector<Share, EVector>& res){
        std::vector<BSharedVector<Share, EVector>*> keys_ptr;

        for(int i = 0; i < keys.size(); ++i){
            keys_ptr.push_back(&keys[i]);
        }

        distinct(keys_ptr, & res);
    }
} }

#endif //TVA_OPERATORS_RELATIONAL_H