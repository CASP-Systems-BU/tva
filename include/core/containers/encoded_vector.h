#ifndef TVA_ENCODED_VECTOR_H
#define TVA_ENCODED_VECTOR_H

#include "../../debug/debug.h"
#include "../../service/common/runtime.h"

#include <memory>

#define shares_define_pointer_reference_operator(op);                   \
template<typename T>                                                    \
std::unique_ptr<T> operator op (std::unique_ptr<T>&& x, const T & y) {  \
    auto& x_ = x.get()[0];                                              \
    assert(x_.size()==y.size());                                        \
    return x_ op y;                                                     \
}                                                                       \

#define shares_define_reference_pointer_operator(op);                   \
template<typename T>                                                    \
std::unique_ptr<T> operator op (const T &x, std::unique_ptr<T>&& y) {   \
    auto& y_ = y.get()[0];                                              \
    assert(x.size()==y_.size());                                        \
    return x op y_;                                                     \
}                                                                       \

#define shares_define_pointers_operator(op);                                        \
template<typename T>                                                                \
std::unique_ptr<T> operator op (std::unique_ptr<T>&& x, std::unique_ptr<T>&& y) {   \
    auto& x_ = x.get()[0];                                                          \
    auto& y_ = y.get()[0];                                                          \
    assert(x_.size()==y_.size());                                                   \
    return x_ op y_;                                                                \
}                                                                                   \

#define shares_define_pointer_operator(op);                                         \
template<typename T>                                                                \
std::unique_ptr<T> operator op (std::unique_ptr<T>&& x) {                           \
    auto& x_ = x.get()[0];                                                          \
    return op x_;                                                                   \
}                                                                                   \

#define shares_define_pointer_reference_operator_no_move(op);           \
template<typename T>                                                    \
std::unique_ptr<T> operator op (std::unique_ptr<T>& x, const T & y) {   \
    auto& x_ = x.get()[0];                                              \
    assert(x_.size()==y.size());                                        \
    return x_ op y;                                                     \
}                                                                       \

#define shares_define_reference_pointer_operator_no_move(op);           \
template<typename T>                                                    \
std::unique_ptr<T> operator op (const T &x, std::unique_ptr<T>& y) {    \
    auto& y_ = y.get()[0];                                              \
    assert(x.size()==y_.size());                                        \
    return x op y_;                                                     \
}                                                                       \

#define shares_define_pointers_operator_no_move(op);                                \
template<typename T>                                                                \
std::unique_ptr<T> operator op (std::unique_ptr<T>& x, std::unique_ptr<T>& y) {     \
    auto& x_ = x.get()[0];                                                          \
    auto& y_ = y.get()[0];                                                          \
    assert(x_.size()==y_.size());                                                   \
    return x_ op y_;                                                                \
}                                                                                   \

#define shares_define_pointer_operator_no_move(op);                                 \
template<typename T>                                                                \
std::unique_ptr<T> operator op (std::unique_ptr<T>& x) {                            \
    auto& x_ = x.get()[0];                                                          \
    return op x_;                                                                   \
}                                                                                   \

#define binary_op(_op_, type, func, this, y);                                       \
std::unique_ptr<type> operator _op_ (const type &y) const {                         \
    auto& x_vector = this->vector;                                                  \
    auto& y_vector = reinterpret_cast<const type *>(&y)->vector;                    \
    assert(x_vector.size()==y_vector.size());                                       \
    auto res = service::runTime.func(x_vector, y_vector);                           \
    return std::unique_ptr<type>(new type(res));                                    \
}                                                                                   \

#define unary_op(_op_, type, func, this);                                           \
std::unique_ptr<type> operator _op_ () const {                                      \
    auto res = service::runTime.func(this->vector);                                 \
    return std::unique_ptr<type>(new type(res));                                    \
}                                                                                   \

#define fn_no_input(func, type, this);                                              \
std::unique_ptr<type> func () const {                                               \
    auto res = service::runTime.func(this->vector);                                 \
    return std::unique_ptr<type>(new type(res));                                    \
}                                                                                   \

#define fn_input_no_return(func, arg, this);                                        \
template<typename T>                                                                \
std::unique_ptr<type> func (T arg) {                                                \
    auto res = service::runTime.func(this->vector, arg);                            \
}                                                                                   \

namespace tva {

    /**
     * Vector encoding types.
     */
    typedef enum
    {
        AShared,
        BShared
    } Encoding;

    /**
     * A EncodedVector is the main programming abstraction in TVA. All secure operators are applied to encoded
     * vectors, i.e., they take one or more encoded vectors as input and generate one or more encoded vectors as output.
     *
     * TVA computing parties perform secure operations on their encoded vectors as if they were performing plaintext
     * operations on the original (secret) vectors. For example, given two secret vectors v1 and v2, TVA parties
     * can execute the elementwise addition of the secret vectors by simply running v1 + v2 using their local encoded
     * vectors. This way, the TVA program looks identical to the respective plaintext program, as shown below:
     *
     * **Plaintext program** (not secure)
     \verbatim
       // Executed by a trusted party
       auto w = v1 + v2;   // These are C++ vectors and '+' is the elementwise plaintext addition
     \endverbatim
     * **TVA program** (secure)
     \verbatim
       // Executed by an untrusted party
       auto w = v1 + v2;   // These are EncodedVectors and '+' is the elementwise secure addition
     \endverbatim
     */
    class EncodedVector {
    public:
        /**
         * The vector's encoding (e.g., A-shared, B-shared, etc.)
         */
        Encoding encoding;

        /**
         * Constructor
         * @param eType - The vector's encoding (e.g., A-shared, B-shared, etc.)
         */
        EncodedVector(const Encoding& eType) : encoding(eType) {};

        // Destructor
        virtual ~EncodedVector() {};

        virtual size_t size() const = 0;

        // TODO: how to generalize this "int"?
        // virtual inline TVA::Vector<int>& operator()(const int& index) = 0;

        // Friend class
        friend class EncodedColumn;
    };

    // **************************************** //
    //           Overloaded operators           //
    //         (for std::unique_pointer)        //
    // **************************************** //

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

    // Binary operators where both operands are std::unique_pointer<T>
    shares_define_pointers_operator(+);
    shares_define_pointers_operator(-);
    shares_define_pointers_operator(*);
    shares_define_pointers_operator(&);
    shares_define_pointers_operator(|);
    shares_define_pointers_operator(^);

    // Unary operators on std::unique_pointer<T>
    shares_define_pointer_operator(-);
    shares_define_pointer_operator(~);
    shares_define_pointer_operator(!);
}

#endif //TVA_ENCODED_VECTOR_H
