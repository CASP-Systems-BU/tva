#ifndef TVA_OPERATORS_SORTING_H
#define TVA_OPERATORS_SORTING_H

#include "common.h"


namespace tva { namespace operators {

    /**
     * Fused elementwise equality and greater-than comparison between two B-shared vectors.
     * @tparam Share - Share data type.
     * @tparam EVector - Share container type.
     * @param x_vec - The first input vector.
     * @param y_vec - The second input vector.
     * @param eq_bits - The B-shared vector that will store the results of the elementwise equality checks.
     * @param gt_bits - The B-shared vector that will store the results of the elementwise greater-than comparisons.
     *
     * NOTE: This method is used when sorting on multiple attributes.
     */
    template <typename Share, typename EVector>
    void compare(const BSharedVector<Share, EVector>& x_vec,
                        const BSharedVector<Share, EVector>& y_vec,
                        BSharedVector<Share, EVector>& eq_bits,
                        BSharedVector<Share, EVector>& gt_bits) {
        // Number of bits in the `Share` representation
        const int MAX_BITS_NUMBER = std::numeric_limits<Share>::digits;
        // Compute same-bits prefix
        BSharedVector<Share, EVector> sameBit = x_vec.bit_same(y_vec);
        // Compute elementwise 'eq' and 'gt'
        BSharedVector<Share, EVector> shifted = sameBit.bit_right_shift(1);
        // Set the MSB of the same-bits prefix (needed for unsigned numbers)
        shifted.set_bits(((Share) 1) << (MAX_BITS_NUMBER - 1));
        BSharedVector<Share, EVector> xEdgeBit_eq(shifted & (~sameBit));
        BSharedVector<Share, EVector> xEdgeBit_gt(xEdgeBit_eq & (x_vec));
        // Store single-bit results
        eq_bits = !xEdgeBit_eq.bit_xor();
        gt_bits = xEdgeBit_gt.bit_xor();
        // If the shares are signed numbers, we need to treat the sign bits differently
        if (std::is_signed<Share>::value) {
            // Extract sign bits
            BSharedVector<Share, EVector> sign_1 = x_vec.bit_right_shift(MAX_BITS_NUMBER);
            BSharedVector<Share, EVector> sign_2 = y_vec.bit_right_shift(MAX_BITS_NUMBER);
            // Are signs different?
            BSharedVector<Share, EVector> diffs = sign_1 ^ sign_2;
            // Update greater bits: `this` is greater than `other` iff the signs are different and `other` is
            // negative, otherwise keep the existing greater bits
            gt_bits = (diffs & sign_2) ^ (!diffs & gt_bits);
        }
    }

    template <typename Share, typename EVector>
    static BSharedVector<Share, EVector> compare(BSharedVector<Share, EVector>& x_vec,
                                                 BSharedVector<Share, EVector>& y_vec,
                                                 const std::vector<bool>& inverse) {
        std::vector<BSharedVector<Share, EVector>*> x_vec_;
        std::vector<BSharedVector<Share, EVector>*> y_vec_;
        x_vec_.push_back(&x_vec);
        y_vec_.push_back(&y_vec);
        return compare_rows(x_vec_, y_vec_, inverse);
    }

    /**
     * Compares two `MxN` arrays row-wise by applying `M` greater-than comparisons on `N` keys.
     *
     * @tparam Share - Share data type.
     * @tparam EVector - Share container type.
     * @param x_vec - The left column-first array with `M` rows and `N` columns.
     * @param y_vec - The right column-first array with `M` rows and `N` columns.
     * @param inverse - A vector of `N` boolean values that denotes the order of comparison per key (if
     * `inverse[i]=True`, then rows from `x_vec` and `y_vec` are swapped in the comparison on the i-th column.
     * @return A new shared vector that contains the result bits of the `M` greater-than comparisons.
     *
     * NOTE: The i-th row, let l, from the left array is greater than the i-th row, let r, from the right array if l's
     * first key is greater than r's first key, or the first keys are the same and l's second key is greater than r's
     * second key, or the first two keys are the same and so forth, for all keys.
     */
    // TODO: use bit compression.
    template <typename Share, typename EVector>
    static BSharedVector<Share, EVector> compare_rows(const std::vector<BSharedVector<Share, EVector>*>& x_vec,
                                                      const std::vector<BSharedVector<Share, EVector>*>& y_vec,
                                                      const std::vector<bool>& inverse){
        assert((x_vec.size() > 0) && (x_vec.size() == y_vec.size()) && (inverse.size() == x_vec.size()));
        const int cols_num = x_vec.size();  // Number of keys
        // Compare elements on first key
        BSharedVector<Share, EVector>* t = inverse[0] ? y_vec[0] : x_vec[0];
        BSharedVector<Share, EVector>* o = inverse[0] ? x_vec[0] : y_vec[0];
        BSharedVector<Share, EVector> eq(t->size());
        BSharedVector<Share, EVector> gt(t->size());
        compare(*t, *o, eq, gt);
        // Compare elements on remaining keys
        for (int i = 1; i < cols_num; ++i) {
            t = inverse[i] ? y_vec[i] : x_vec[i];
            o = inverse[i] ? x_vec[i] : y_vec[i];
            BSharedVector<Share, EVector> new_eq(t->size());
            BSharedVector<Share, EVector> new_gt(t->size());
            compare(*t, *o, new_eq, new_gt);
            // Compose 'gt' and `eq` bits
            gt = gt ^ (new_gt & eq);
            eq = eq & new_eq;
        }
        return gt;
    }

    /**
     * Same as above but accepts the `N` columns by reference.
     *
     * @tparam Share - Share data type.
     * @tparam EVector - Share container type.
     * @param x_vec - The left column-first array with `M` rows and `N` columns.
     * @param y_vec - The right column-first array with `M` rows and `N` columns.
     * @param inverse - A vector of `N` boolean values that denote the order of comparison per key (if
     * `inverse[i]=True`, then rows from `x_vec` and `y_vec` are swapped in the comparison on the i-th column.
     * @return A new shared vector that contains the result bits of the `M` greater-than comparisons.
     */
    // TODO: x_vec and y_vec should be passed as const
    template <typename Share, typename EVector>
    static BSharedVector<Share, EVector> compare_rows(std::vector<BSharedVector<Share, EVector>>& x_vec,
                                                      std::vector<BSharedVector<Share, EVector>>& y_vec,
                                                      const std::vector<bool>& inverse){
        std::vector<BSharedVector<Share, EVector>*> x_vec_;
        std::vector<BSharedVector<Share, EVector>*> y_vec_;
        for(int i = 0; i < x_vec.size(); ++i){
            x_vec_.push_back(&x_vec[i]);
            y_vec_.push_back(&y_vec[i]);
        }

        return compare_rows(x_vec_, y_vec_, inverse);
    }



    /**
     * Swaps rows of two `MxN` arrays in place using the provided `bits`.
     *
     * @tparam Share - Share data type.
     * @tparam EVector - Share container type.
     * @param x_vec - The left column-first array with `M` rows and `N` columns.
     * @param y_vec - The right column-first array with `M` rows and `N` columns.
     * @param bits - The B-shared vector that contains 'M' bits to use for oblivious swapping (if bits[i]=True, the
     * i-th rows will be swapped).
     */
    template <typename Share, typename EVector>
    static void swap(std::vector<BSharedVector<Share, EVector>*>& x_vec,
                     std::vector<BSharedVector<Share, EVector>*>& y_vec,
                     const BSharedVector<Share, EVector>& bits){
        // Make sure the input arrays have the same dimensions
        assert((x_vec.size() > 0) && (x_vec.size() == y_vec.size()));
        const int cols_num = x_vec.size();  // Number of columns
        for (int i = 0; i < cols_num; ++i) {
            assert((x_vec[i]->size() == y_vec[i]->size()) && (bits.size() == x_vec[i]->size()));
        }
        auto b = bits.extend_lsb();
        // Swap elements
        for (int i = 0; i < cols_num; ++i) {
            auto tmp = (b & *y_vec[i]) ^ (~b & *x_vec[i]);
            *y_vec[i] = (b & *x_vec[i]) ^ (~b & *y_vec[i]);
            *x_vec[i] = tmp;
        }
    }

    template <typename Share, typename EVector>
    static void swap(std::vector<ASharedVector<Share, EVector>*>& x_vec,
                     std::vector<ASharedVector<Share, EVector>*>& y_vec,
                     const ASharedVector<Share, EVector>& bits){
        // Make sure the input arrays have the same dimensions
        assert((x_vec.size() > 0) && (x_vec.size() == y_vec.size()));
        const int cols_num = x_vec.size();  // Number of columns
        for (int i = 0; i < cols_num; ++i) {
            assert((x_vec[i]->size() == y_vec[i]->size()) && (bits.size() == x_vec[i]->size()));
        }

        // Swap elements
        for (int i = 0; i < cols_num; ++i) {
            auto tmp = multiplex(bits, *x_vec[i], *y_vec[i]);
            *y_vec[i] = multiplex(bits, *y_vec[i], *x_vec[i]);
            *x_vec[i] = tmp;
        }
    }


    /**
     * Same as above but accepts the `N` columns by reference.
     *
     * @tparam Share - Share data type.
     * @tparam EVector - Share container type.
     * @param x_vec - The left column-first array with `M` rows and `N` columns.
     * @param y_vec - The right column-first array with `M` rows and `N` columns.
     * @param bits - The B-shared vector that contains 'M' bits to use for oblivious swapping (if bits[i]=True, the
     * i-th rows will be swapped).
     */
    template <typename Share, typename EVector>
    static void swap(std::vector<BSharedVector<Share, EVector>> &x_vec,
                     std::vector<BSharedVector<Share, EVector>> &y_vec,
                     const BSharedVector<Share, EVector> &bits) {
        std::vector<BSharedVector<Share, EVector>*> x_vec_;
        std::vector<BSharedVector<Share, EVector>*> y_vec_;
        for(int i = 0; i < x_vec.size(); ++i){
            x_vec_.push_back(&x_vec[i]);
            y_vec_.push_back(&y_vec[i]);
        }
        swap(x_vec_, y_vec_, bits);
    }

    template <typename Share, typename EVector>
    static void swap(std::vector<ASharedVector<Share, EVector>> &x_vec,
                     std::vector<ASharedVector<Share, EVector>> &y_vec,
                     const ASharedVector<Share, EVector> &bits) {
        std::vector<ASharedVector<Share, EVector>*> x_vec_;
        std::vector<ASharedVector<Share, EVector>*> y_vec_;
        for(int i = 0; i < x_vec.size(); ++i){
            x_vec_.push_back(&x_vec[i]);
            y_vec_.push_back(&y_vec[i]);
        }
        swap(x_vec_, y_vec_, bits);
    }

    /**
     * Same as above but accepts the `N` columns by reference.
     *
     * @tparam Share - Share data type.
     * @tparam EVector - Share container type.
     * @param x_vec - The left column-first array with `M` rows and `N` columns.
     * @param y_vec - The right column-first array with `M` rows and `N` columns.
     * @param bits - The B-shared vector that contains 'M' bits to use for oblivious swapping (if bits[i]=True, the
     * i-th rows will be swapped).
     */
    template<typename Share, typename EVector>
    void swap(BSharedVector <Share, EVector> &x_vec,
              BSharedVector <Share, EVector> &y_vec,
              BSharedVector <Share, EVector> &bits) {
        bits.mask((Share) 1);  // Mask all bits but the LSB
        std::vector<BSharedVector < Share, EVector>*> x_vec_;
        std::vector<BSharedVector < Share, EVector>*> y_vec_;
        x_vec_.push_back(&x_vec);
        y_vec_.push_back(&y_vec);
        swap(x_vec_, y_vec_, bits);
    }

    /**
     * Sorts rows in the given array on all columns. Updates array in place.
     *
     * @tparam Share - Share data type.
     * @tparam EVector - Share container data type.
     * @param _columns - The columns of the array.
     * @param desc - The sorting direction per column (True for DESC).
     */
    template<typename Share, typename EVector>
    static void bitonic_sort(std::vector<BSharedVector<Share, EVector>*> _columns,
                             std::vector<ASharedVector<Share, EVector>*> _data_a,
                             std::vector<BSharedVector<Share, EVector>*> _data_b,
                             const std::vector<bool> &desc) {
        assert(_columns.size() > 0);
        // Vector sizes must be a power of two
        // TODO (john): Modify sorter to support arbitrary vector sizes
        for(int i = 0; i < _columns.size(); ++i)
            assert(ceil(log2(_columns[i]->size())) == floor(log2(_columns[i]->size())));
        // Number of rounds of bitonic sort
        int rounds = (int) log2(_columns[0]->size());
        // For each round
        for (int i=0; i<rounds; i++) {
            // For each column within a round
            for (int j=0; j<=i; j++) {
                const int half_box_size = 1 << (i-j);
                const int box_direction_2 = (j==0) ? -1 : 1;
                // The left (x) and right (y) rows to compare
                std::vector<BSharedVector<Share, EVector>> x;
                std::vector<BSharedVector<Share, EVector>> y;
                for (int k = 0; k <_columns.size(); ++k){
                    x.push_back(_columns[k]->alternating_subset_reference(half_box_size, half_box_size));
                    if(box_direction_2 == -1){
                        y.push_back(_columns[k]->simple_subset_reference(half_box_size)
                                    .reversed_alternating_subset_reference(half_box_size, half_box_size));
                    }else{
                        y.push_back(_columns[k]->simple_subset_reference(half_box_size)
                                    .alternating_subset_reference(half_box_size, half_box_size));
                    }
                }
                // Compare rows on all columns
                BSharedVector<Share, EVector> bits = compare_rows(x,y, desc);
                // Swap rows in place using the comparison bits
                swap(x,y,bits);


                // Sorting Data as well
                if(_data_b.size() > 0){
                    std::vector<BSharedVector<Share, EVector>> _data_b_1;
                    std::vector<BSharedVector<Share, EVector>> _data_b_2;
                    for(int k = 0; k < _data_b.size(); ++k){
                        _data_b_1.push_back(_data_b[k]->alternating_subset_reference(half_box_size, half_box_size));
                        if(box_direction_2 == -1){
                            _data_b_2.push_back(_data_b[k]->simple_subset_reference(half_box_size)
                                        .reversed_alternating_subset_reference(half_box_size, half_box_size));
                        }else{
                            _data_b_2.push_back(_data_b[k]->simple_subset_reference(half_box_size)
                                        .alternating_subset_reference(half_box_size, half_box_size));
                        }
                    }
                    swap(_data_b_1, _data_b_2, bits);
                }

                if(_data_a.size() > 0){
                    ASharedVector<Share, EVector> bits_a = bits.b2a_bit();
                    std::vector<ASharedVector<Share, EVector>> _data_a_1;
                    std::vector<ASharedVector<Share, EVector>> _data_a_2;
                    for(int k = 0; k < _data_a.size(); ++k){
                        _data_a_1.push_back(_data_a[k]->alternating_subset_reference(half_box_size, half_box_size));
                        if(box_direction_2 == -1){
                            _data_a_2.push_back(_data_a[k]->simple_subset_reference(half_box_size)
                                        .reversed_alternating_subset_reference(half_box_size, half_box_size));
                        }else{
                            _data_a_2.push_back(_data_a[k]->simple_subset_reference(half_box_size)
                                        .alternating_subset_reference(half_box_size, half_box_size));
                        }
                    }
                    swap(_data_a_1, _data_a_2, bits_a);
                }
            }
        }
    }

    /**
     * Same as above but accepts the columns by reference.
     *
     * @tparam Share - Share data type.
     * @tparam EVector - Share container data type.
     * @param _columns - The columns of the array.
     * @param desc - The sorting direction per column (True for DESC).
     */
    template<typename Share, typename EVector>
    static void bitonic_sort(std::vector<BSharedVector<Share, EVector>> _columns,
                             std::vector<ASharedVector<Share, EVector>> _data_a,
                             std::vector<BSharedVector<Share, EVector>> _data_b,
                             const std::vector<bool> &desc) {
        std::vector<BSharedVector<Share, EVector>*> res;
        for(BSharedVector<Share, EVector>& c : _columns){
            res.push_back(&c);
        }

        std::vector<ASharedVector<Share, EVector>*> _data_a_;
        for(ASharedVector<Share, EVector>& c : _columns){
            res.push_back(&c);
        }

        std::vector<BSharedVector<Share, EVector>*> _data_b_;
        for(BSharedVector<Share, EVector>& c : _columns){
            res.push_back(&c);
        }


        bitonic_sort(res, _data_a_, _data_b_, desc);
    }

    /**
     * Sorts rows in the given array on all columns. Updates array in place.
     *
     * @tparam Share - Share data type.
     * @tparam EVector - Share container data type.
     * @param _columns - The columns of the array.
     * @param desc - The sorting direction per column (True for DESC).
     */
    template<typename Share, typename EVector>
    static void bitonic_sort(BSharedVector<Share, EVector>& vec,
                             bool desc=false) {
        std::vector<BSharedVector<Share, EVector>*> res;
        res.push_back(&vec);
        bitonic_sort(res, {}, {}, {desc});
    }


} }




#endif //TVA_OPERATORS_SORTING_H
