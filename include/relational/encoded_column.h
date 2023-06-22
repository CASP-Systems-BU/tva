#ifndef TVA_ENCODED_COLUMN_H
#define TVA_ENCODED_COLUMN_H

#include "../debug/debug.h"

#define binary_op_downcast(_op_, eType, vType);                                         \
std::unique_ptr<EncodedColumn> operator  _op_ (const EncodedColumn &other) const {      \
    assert(encoding == Encoding::eType &&                                               \
           encoding == other.encoding);                                                 \
    auto v1 = (vType<Share, EVector>*) contents.get();                                  \
    auto v2 = (vType<Share, EVector>*)  other.contents.get();                           \
    return std::unique_ptr<EncodedColumn>(new SharedColumn(*v1 _op_ *v2));              \
}                                                                                       \

#define unary_op_downcast(_op_, eType, vType);                                          \
std::unique_ptr<EncodedColumn> operator _op_ () const {                                 \
    assert(encoding == Encoding::eType);                                                \
    auto v = (vType<Share, EVector>*) contents.get();                                   \
    return std::unique_ptr<EncodedColumn>(new SharedColumn(_op_ (*v)));                 \
}                                                                                       \


namespace tva { namespace relational {

    // TODO:
    //  - Add a way to convert EncodedColumn to ASharedVector/BSharedVector if possible

    /**
     * An encoded table column that supports vectorized secure operations.
     */
    class EncodedColumn {
    public:

        /**
         * The column's encoding (e.g., A-shared, B-shared, etc.)
         */
        Encoding encoding;

        /**
         * The column's contents (i.e., an encoded vector).
         */
        std::unique_ptr<EncodedVector> contents;

        /**
         * The column's name
         */
        std::string name;

        // Destructor
        virtual ~EncodedColumn() {};

        /**
         * @return The column's size in number of elements.
         */
        virtual size_t size() const = 0;

        /**
         * Elementwise secure arithmetic addition.
         * @param other - The second operand of addition.
         * @return A unique pointer to an EncodedColumn that contains encoded results of
         * the elementwise additions.
         */
        virtual std::unique_ptr<EncodedColumn> operator+(const EncodedColumn &other) const = 0 ;

        /**
         * Elementwise secure arithmetic subtraction.
         * @param other - The second operand of subtraction.
         * @return A unique pointer to an EncodedColumn that contains encoded results of
         * the elementwise subtractions.
         */
        virtual std::unique_ptr<EncodedColumn> operator-(const EncodedColumn &other) const = 0;

        /**
         * Elementwise secure arithmetic multiplication.
         * @param other - The second operand of multiplication.
         * @return A unique pointer to an EncodedColumn that contains encoded results of
         * the elementwise multiplications.
         */
        virtual std::unique_ptr<EncodedColumn> operator*(const EncodedColumn &other) const = 0;

        /**
         * Elementwise secure arithmetic negation.
         * @return A unique pointer to an EncodedColumn with all elements of `this` EncodedColumn negated.
         */
       virtual  std::unique_ptr<EncodedColumn> operator-() const = 0;

        // **************************************** //
        //             Boolean operators            //
        // **************************************** //

        /**
         * Elementwise secure bitwise XOR.
         * @param other - The second operand of XOR.
         * @return A unique pointer to an EncodedColumn that contains encoded results of
         * the elementwise XORs.
         */
        virtual std::unique_ptr<EncodedColumn> operator^(const EncodedColumn &other) const = 0;

        /**
         * Elementwise secure bitwise AND.
         * @param other - The second operand of AND.
         * @return A unique pointer to an EncodedColumn that contains encoded results of
         * the elementwise ANDs.
         */
        virtual std::unique_ptr<EncodedColumn> operator&(const EncodedColumn &other) const = 0;

        /**
         * Elementwise secure bitwise OR.
         * @param other - The second operand of OR.
         * @return A unique pointer to an EncodedColumn that contains encoded results of
         * the elementwise ORs.
         */
        virtual std::unique_ptr<EncodedColumn> operator|(const EncodedColumn &other) const = 0;

        /**
         * Elementwise secure boolean completion.
         * @return A unique pointer to an EncodedColumn with all elements of `this` EncodedColumn complemented.
         */
        virtual std::unique_ptr<EncodedColumn> operator~() const = 0;

        /**
         * Elementwise secure boolean negation.
         * @return A unique pointer to an EncodedColumn with all results of `this` EncodedColumn negated.
         */
        virtual std::unique_ptr<EncodedColumn> operator!() const = 0;

        // **************************************** //
        //           Comparison operators           //
        // **************************************** //

        /**
         * Elementwise secure equality.
         * @param other - The second operand of equality.
         * @return A unique pointer to an EncodedColumn that contains encoded results of
         * the elementwise equality comparisons.
         */
        virtual std::unique_ptr<EncodedColumn> operator==(const EncodedColumn &other) const = 0;

        /**
         * Elementwise secure inequality.
         * @param other - The second operand of inequality.
         * @return A unique pointer to an EncodedColumn that contains encoded results of
         * the elementwise inequality comparisons.
         */
        virtual std::unique_ptr<EncodedColumn> operator!=(const EncodedColumn &other) const = 0;

        /**
         * Elementwise secure greater-than.
         * @param other - The second operand of greater-than.
         * @return A unique pointer to an EncodedColumn that contains encoded results of
         * the elementwise greater-than comparisons.
         */
        virtual std::unique_ptr<EncodedColumn> operator>(const EncodedColumn &other) const = 0;

        /**
         * Elementwise secure greater-or-equal.
         * @param other - The second operand of greater-or-equal.
         * @return A unique pointer to an EncodedColumn that contains encoded results of
         * the elementwise greater-or-equal comparisons.
         */
        virtual std::unique_ptr<EncodedColumn> operator>=(const EncodedColumn &other) const = 0;

        /**
         * Elementwise secure less-than.
         * @param other - The second operand of less-than.
         * @return A unique pointer to an EncodedColumn that contains encoded results of
         * the elementwise less-than comparisons.
         */
        virtual std::unique_ptr<EncodedColumn> operator<(const EncodedColumn &other) const = 0;

        /**
         * Elementwise secure less-or-equal.
         * @param other - The second operand of less-or-equal.
         * @return A unique pointer to an EncodedColumn that contains encoded results of
         * the elementwise less-or-equal comparisons.
         */
        virtual std::unique_ptr<EncodedColumn> operator<=(const EncodedColumn &other) const = 0;

        /**
         * Move assignment operator.
         * @param other - The unique pointer to the column whose contents will be moved to `this` column.
         * @return A reference to `this` column after modification.
         */
        virtual EncodedColumn& operator = (std::unique_ptr<EncodedColumn>&& other) = 0;

        // TODO (john): Maybe we also need a copy assignment, e.g., in case we want to copy a column into another
    };

    // NOTE: Although these operator templates are defined in TVA, we must redefine them in tva::relational

    // Binary operators between T (left) and std::unique_pointer<T> (right)
    shares_define_reference_pointer_operator_no_move(+);
    shares_define_reference_pointer_operator_no_move(-);
    shares_define_reference_pointer_operator_no_move(*);
    shares_define_reference_pointer_operator_no_move(&);
    shares_define_reference_pointer_operator_no_move(|);
    shares_define_reference_pointer_operator_no_move(^);

    // Binary operators between std::unique_pointer<T> (left) and T (right)
    shares_define_pointer_reference_operator_no_move(+);
    shares_define_pointer_reference_operator_no_move(-);
    shares_define_pointer_reference_operator_no_move(*);
    shares_define_pointer_reference_operator_no_move(&);
    shares_define_pointer_reference_operator_no_move(|);
    shares_define_pointer_reference_operator_no_move(^);

    // Binary operators between T (left) and std::unique_pointer<T> (right)
    shares_define_reference_pointer_operator(+);
    shares_define_reference_pointer_operator(-);
    shares_define_reference_pointer_operator(*);
    shares_define_reference_pointer_operator(&);
    shares_define_reference_pointer_operator(|);
    shares_define_reference_pointer_operator(^);

    // Binary operators between std::unique_pointer<T> (left) and T (right)
    shares_define_pointer_reference_operator(+);
    shares_define_pointer_reference_operator(-);
    shares_define_pointer_reference_operator(*);
    shares_define_pointer_reference_operator(&);
    shares_define_pointer_reference_operator(|);
    shares_define_pointer_reference_operator(^);

    // Binary operators where both operands are std::unique_pointer<T>&&
    shares_define_pointers_operator(+);
    shares_define_pointers_operator(-);
    shares_define_pointers_operator(*);
    shares_define_pointers_operator(&);
    shares_define_pointers_operator(|);
    shares_define_pointers_operator(^);

    // Binary operators where both operands are std::unique_pointer<T>
    shares_define_pointers_operator_no_move(+);
    shares_define_pointers_operator_no_move(-);
    shares_define_pointers_operator_no_move(*);
    shares_define_pointers_operator_no_move(&);
    shares_define_pointers_operator_no_move(|);
    shares_define_pointers_operator_no_move(^);

    // Unary operators on std::unique_pointer<T>
    shares_define_pointer_operator_no_move(-);
    shares_define_pointer_operator_no_move(~);
    shares_define_pointer_operator_no_move(!);
} }



#endif //TVA_ENCODED_COLUMN_H
