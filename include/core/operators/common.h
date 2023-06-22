#ifndef TVA_OPERATORS_COMMON_H
#define TVA_OPERATORS_COMMON_H


#include "../containers/a_shared_vector.h"
#include "../containers/b_shared_vector.h"


namespace tva { namespace operators {

    template<typename Share, typename EVector>
    static BSharedVector <Share, EVector> multiplex(const BSharedVector <Share, EVector> &sel,
                                                    const BSharedVector <Share, EVector> &a,
                                                    const BSharedVector <Share, EVector> &b) {
        auto sel_extended = sel.extend_lsb();
        BSharedVector<Share, EVector> res = ((~sel_extended) & a) ^ (sel_extended & b);
        return res;
    }

    template<typename Share, typename EVector>
    static ASharedVector <Share, EVector> multiplex(const ASharedVector <Share, EVector> &sel,
                                                    const ASharedVector <Share, EVector> &a,
                                                    const ASharedVector <Share, EVector> &b) {
        ASharedVector<Share, EVector> res = a - (sel * a) + (sel *b);
        return res;
    }

    template <typename Share, typename EVector>
    static void
    auto_conversion (BSharedVector<Share, EVector>& x, BSharedVector<Share, EVector>& res){
        res = x;
    }

    template <typename Share, typename EVector>
    static void
    auto_conversion (ASharedVector<Share, EVector>& x, ASharedVector<Share, EVector>& res){
        res = x;
    }

    // TODO: differentiate between "b2a_bit" and just "b2a"
    template <typename Share, typename EVector>
    static void
    auto_conversion (BSharedVector<Share, EVector>& x, ASharedVector<Share, EVector>& res){
        res = x.b2a_bit();
    }

    template <typename Share, typename EVector>
    static void
    auto_conversion (ASharedVector<Share, EVector>& x, BSharedVector<Share, EVector>& res){
        res = x.a2b();
    }

} }

#endif //TVA_OPERATORS_COMMON_H
