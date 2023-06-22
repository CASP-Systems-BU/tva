#ifndef TVA_SHARED_COLUMN_H
#define TVA_SHARED_COLUMN_H

#include "../core/containers/a_shared_vector.h"
#include "../core/containers/b_shared_vector.h"
#include "../core/containers/e_vector.h"
#include "encoded_column.h"


namespace tva { namespace relational {

    /**
     * A secret-shared column with share and container types.
     *
     * @tparam Share - Share data type.
     * @tparam EVector - Container data type.
     */
    template<typename Share, typename EVector>
    class SharedColumn : public EncodedColumn {
    public:

        /**
         * Allocates a shared column with the given encoding and initializes it with zeros.
         * @param _size - The column's size in number of elements.
         * @param eType - The column's encoding (e.g., A-Shared, B-Shared, etc.).
         */
        explicit SharedColumn(const int &_size, const Encoding& eType) {
            switch (eType) {
                case Encoding::AShared:
                {
                    auto a = new ASharedVector<Share, EVector>(_size);
                    contents = std::unique_ptr<ASharedVector<Share, EVector>>(a);
                    break;
                }
                case Encoding::BShared:
                {
                    auto b = new BSharedVector<Share, EVector>(_size);
                    contents = std::unique_ptr<BSharedVector<Share, EVector>>(b);
                    break;
                }
            }
            encoding = eType;
        }

        /**
         * Move constructor from an encoded vector.
         * @param _shares - The encoded vector whose contents will be moved to the new column.
         */
        explicit SharedColumn(std::unique_ptr<EncodedVector>&& _shares, const std::string& col_name="") {
            auto v = _shares.release();
            encoding = v->encoding;
            contents = std::unique_ptr<EncodedVector>(v);
            name = col_name;
        }

//        /**
//         * Move constructor from an encoded vector.
//         * @param _shares - The encoded vector whose contents will be moved to the new column.
//         */
//        explicit SharedColumn(EncodedVector& _shares) {
//            encoding = _shares.encoding;
//            contents = std::unique_ptr<EncodedVector>(&_shares);
//        }

        /**
         * Move constructor from an encoded column.
         * @param other - The encoded column whose contents will be moved to the new column.
         */
        SharedColumn(std::unique_ptr<EncodedColumn>&& other) {
            auto& v = *other.get();
            encoding = v.encoding;
            contents = std::unique_ptr<EncodedColumn>(&v);
        }

        SharedColumn& operator = (std::unique_ptr<EncodedColumn>&& other) {
            auto& c = *other.get();
            this->encoding = c.encoding;
            this->contents.reset(c.contents.release());
            return *this;
        }

        // Destructor
        virtual ~SharedColumn() {};

        /**
         * @return The column's size in number of elements.
         */
        virtual size_t size() const {
            return this->contents.get()->size();
        }

        // **************************************** //
        //           Arithmetic operators           //
        // **************************************** //

        binary_op_downcast(+, AShared, ASharedVector);
        binary_op_downcast(-, AShared, ASharedVector);
        binary_op_downcast(*, AShared, ASharedVector);

        unary_op_downcast(-, AShared, ASharedVector);

        // **************************************** //
        //             Boolean operators            //
        // **************************************** //

        binary_op_downcast(&, BShared, BSharedVector);
        binary_op_downcast(^, BShared, BSharedVector);
        binary_op_downcast(|, BShared, BSharedVector);

        unary_op_downcast(~, BShared, BSharedVector);
        unary_op_downcast(!, BShared, BSharedVector);

        // **************************************** //
        //           Comparison operators           //
        // **************************************** //

        binary_op_downcast(==, BShared, BSharedVector);
        binary_op_downcast(!=, BShared, BSharedVector);
        binary_op_downcast(>, BShared, BSharedVector);
        binary_op_downcast(<, BShared, BSharedVector);
        binary_op_downcast(>=, BShared, BSharedVector);
        binary_op_downcast(<=, BShared, BSharedVector);
    };
} }

#endif //TVA_SHARED_COLUMN_H
