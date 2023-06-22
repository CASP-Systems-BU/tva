#ifndef TVA_E_VECTOR_H
#define TVA_E_VECTOR_H

#include <iostream>
#include "vector.h"  // TVA's wrapper for std::vector

#define define_apply_return_to_replicated(_func_);              \
template<typename...T>                                          \
EVector  _func_ (T... args) const {                             \
    std::vector<Vector<Share> > res;                            \
    for(int i = 0; i < ReplicationNumber; ++i){                 \
        res.push_back(contents[i]._func_(args...));             \
    }                                                           \
    return res;                                                 \
}

#define define_apply_to_replicated(_func_);                     \
template<typename...T>                                          \
void  _func_ (T... args) {                                      \
    for(int i = 0; i < ReplicationNumber; ++i){                 \
        contents[i]._func_(args...);                            \
    }                                                           \
}                                                               \


#define define_apply_input_to_replicated(_func_);               \
template<typename...T>                                          \
void  _func_ (const EVector& other, T... args) {                \
    for(int i = 0; i < ReplicationNumber; ++i){                 \
        contents[i]._func_(other.contents[i], args...);         \
    }                                                           \
}                                                               \


#define define_binary_evector_element_op(_op_);                 \
template<typename OtherType>                                    \
inline EVector operator _op_(const OtherType &other) const {    \
    std::vector<Vector < Share> > res;                          \
    for (int i = 0; i < ReplicationNumber; ++i) {               \
        res.push_back(contents[i] _op_ other);                  \
    }                                                           \
    return res;                                                 \
}                                                               \


#define define_binary_evector_evector_op(_op_);                 \
inline EVector operator _op_(const EVector &other) const {      \
    std::vector<Vector < Share> > res;                          \
    for (int i = 0; i < ReplicationNumber; ++i) {               \
        res.push_back(contents[i] _op_ other.contents[i]);      \
    }                                                           \
    return res;                                                 \
}                                                               \

namespace tva {

    // Forward declarations
    namespace service {
        class RunTime;
        template<typename InputType, typename ReturnType, typename ObjectType>
        class Task_1;
        template<typename InputType, typename ReturnType, typename ObjectType>
        class Task_2;
    }

    /**
     * @tparam Share - Share type.
     * @tparam ReplicationNumber - The number of shares that each party sees for each secret value.
     *
     * EVector is an abstraction similar to the EncodedVector, i.e., an "encoded view" of a plaintext vector as seen by
     * an untrusted party. In contrast to EncodedVector, EVector provides access to the underlying encodings of the
     * secret values.
     *
     * While EncodedVector targets end-users, EVector is the abstraction provided to Protocol developers who need
     * access to the underlying encodings in order to define new secure primitives, such as Protocol::add_a(),
     * Protocol::and_b(), etc.
     */
    template<typename Share, int ReplicationNumber>
    class EVector {
        // The EVector contents
        std::vector<Vector < Share> > contents;

        /**
         * Returns the size of EVector, i.e., the total number of secret values in the original (plaintext) vector.
         * @return The size of EVector.
         *
         * NOTE: The output of total_size() cannot be less than the output of size().
         */
        inline int total_size() const {
            return contents[0].total_size();
        }

    public:
        static const int replicationNumber = ReplicationNumber;

        /**
         * Applies Vector::simple_subset() to each vector in contents.
         */
        define_apply_return_to_replicated(simple_subset);

        /**
         * Applies Vector::bit_level_shift() to each vector in contents.
         */
        define_apply_return_to_replicated(bit_level_shift);
        
        /**
         * Applies Vector::reverse_bit_level_shift() to each vector in contents.
         */
        define_apply_return_to_replicated(reverse_bit_level_shift);

        /**
         * Applies Vector::bit_right_shift() to each vector in contents.
         */
        define_apply_return_to_replicated(bit_right_shift);

        /**
         * Applies Vector::bit_left_shift() to each vector in contents.
         */
        define_apply_return_to_replicated(bit_left_shift);

        /**
         * Applies Vector::bit_xor() to each vector in contents.
         */
        define_apply_return_to_replicated(bit_xor);

        /**
         * Applies Vector::set_batch() to each vector in contents.
         */
        define_apply_to_replicated(set_batch);

        /**
         * Applies Vector::ltz() to each vector in contents.
         */
        define_apply_return_to_replicated(ltz);

        /**
         * Applies Vector::extend_lsb() to each vector in contents.
         */
        define_apply_return_to_replicated(extend_lsb);

        // /**
        //  * Applies Vector::subset_reference() to each vector in contents.
        //  */
        // define_apply_return_to_replicated(subset_reference);

        /**
         * Applies Vector::simple_subset_reference() to each vector in contents.
         */
        define_apply_return_to_replicated(simple_subset_reference);

        /**
         * Applies Vector::alternating_subset_reference() to each vector in contents.
         */
        define_apply_return_to_replicated(alternating_subset_reference);

        /**
         * Applies Vector::reversed_alternating_subset_reference() to each vector in contents.
         */
        define_apply_return_to_replicated(reversed_alternating_subset_reference);

        /**
         * Applies Vector::repeated_subset_reference() to each vector in contents.
         */
        define_apply_return_to_replicated(repeated_subset_reference);

        /**
         * Applies Vector::cyclic_subset_reference() to each vector in contents.
         */
        define_apply_return_to_replicated(cyclic_subset_reference);

        /**
         * Applies Vector::directed_subset_reference() to each vector in contents.
         */
        define_apply_return_to_replicated(directed_subset_reference);

        /**
         * Applies Vector::simple_bit_compress() to each vector in contents.
         */
        define_apply_return_to_replicated(simple_bit_compress);

        /**
         * Applies Vector::alternating_bit_compress() to each vector in contents.
         */
        define_apply_return_to_replicated(alternating_bit_compress);


        /**
         * Applies Vector::simple_bit_decompress() to each vector in contents.
         */
        define_apply_input_to_replicated(simple_bit_decompress);

        /**
         * Applies Vector::alternating_bit_decompress() to each vector in contents.
         */
        define_apply_input_to_replicated(alternating_bit_decompress);

        /**
         * Provides an interface to initialize the data object.
         * @tparam T - A generic type to allow for different constructors.
         * @param args - The different parameters of the generic types T.
         *
         * NOTE: the copy constructor of Vector<T> is shallow; if this is initialized by another Vector<T> object,
         * both will point to data in the same memory location.
         */
        template<typename...T>
        EVector(T... args) :
                contents(args...) {}

        /**
         * EVector<T,N> constructor that allocates `ReplicationNumber` new Vectors, each one of size `size`.
         * @param size - The size of Vector<T> in this EVector<T,N>.
         */
        EVector(const int &size) {
            for (int i = 0; i < ReplicationNumber; ++i) {
                // NOTE: it is important to use the push_back and
                // not pass the size in the constructor. Otherwise,
                // std::vector uses the copy constructor and all
                // `Vector`s will point to the same memory location.
                contents.push_back(Vector<Share>(size));
            }
        }

        /**
         * This is a shallow copy constructor (i.e., only copies the vector pointer but not its contents).
         * @param other - The other EVector whose contents this vector will point to.
         *
         * WARNING: The new vector will point to the same memory location as used by `other`. To copy the data into a
         * separate memory location, create a new vector first then use the assignment operator.
         */
        EVector(const EVector &other) :
                contents(other.contents) {}

        /**
         * This is a deep move assignment.
         * Applies the move assignment operator to Vector<T>. Assigns the values of the current batch from the
         * `other` vector to the current batch of this vector. Assumes `other` has the same size and replication
         * factor as this vector.
         * @param other - The EVector containing the values that this vector must point to.
         * @return A reference to this EVector after modification.
         */
        EVector &operator=(const EVector &&other) {
            for (int i = 0; i < ReplicationNumber; ++i) {
                contents[i] = other.contents[i];
            }
            return *this;
        }

        /**
         * This is a deep copy assignment.
         * Applies the copy assignment operator to Vector<T>. Copies the values of the current batch from the
         * `other` vector to the current batch of this vector. Assumes `other` has the same size and replication
         * factor as this vector.
         * @param other - The EVector that contains the values to be copied.
         * @return A reference to this EVector after modification.
         */
        EVector &operator=(const EVector &other) {
            for (int i = 0; i < ReplicationNumber; ++i) {
                contents[i] = other.contents[i];
            }
            return *this;
        }

        /**
         * Masks each element in `this` vector by doing a bitwise logical AND with `n`
         * @param n - The mask.
         */
        void mask(const Share& n) {
            for (int i = 0; i < ReplicationNumber; ++i)
                contents[i].mask(n);
        }

        /**
         * Sets the bits of each element in `this` vector by doing a bitwise logical OR with `n`
         * @param n - The element that encodes the bits to set.
         */
        void set_bits(const Share& n) {
            for (int i = 0; i < ReplicationNumber; ++i)
                contents[i].set_bits(n);
        }

        /**
         * Returns the current batch size, i.e., the number of secret values that are being processed in the
         * current round. The default batch size is the actual vector size.
         * @return The current batch size.
         *
         * NOTE: TVA parties apply operations on EVectors in batches. Each batch corresponds to a range of
         * elements in the vector.
         */
        inline int size() const {
            return contents[0].size();
        }

        /**
         * Returns a mutable reference to a column of the EVector.
         * @param column - The column index.
         * @return A mutable reference to a vector of encodings at the given index.
         *
         * EVector is implemented as a column-first 2D vector, where:
         *
         * - row *i* corresponds to *k* >= 1 encodings of the *i*-th secret value, 0 <= *i* < *n*.
         * - column *j* stores the *j*-th encoding, 0 <= *j* < *k*, of each secret value in the vector.
         *
         * The above column-first representation aims to facilitate the implementation of vectorized secure primitives.
         * The number of secret values *n* in EVector is defined by the user but the number of encodings *k* per secret
         * value depends on the Protocol. For example, in Replicated_3PC, each secret value \f$s\f$ is encoded with
         * three shares \f$s_1\f$, \f$s_2\f$, and \f$s_3\f$ and each party receives *k*=2 of the 3 shares, namely:
         *
         * - Party 0 receives shares \f$s_1\f$ and \f$s_2\f$.
         * - Party 1 receives shares \f$s_2\f$ and \f$s_3\f$.
         * - Party 2 receives shares \f$s_3\f$ and \f$s_1\f$.
         *
         * Assuming a plaintext vector \f$v = \{s^1, s^2, ..., s^{n}\}\f$ with *n* values that
         * are secret-shared by three parties, the respective EVectors in Replicated_3PC look as shown below:
         *
         * - \f$v = \{~\{s^1_1, s^2_1,~...~s^{n}_1\},~\{s^1_2, s^2_2,~...,~s^{n}_2\}~\}\f$     (Party 1)
         * - \f$v = \{~\{s^1_2, s^2_2,~...~s^{n}_2\},~\{s^1_3, s^2_3,~...,~s^{n}_3\}~\}\f$     (Party 2)
         * - \f$v = \{~\{s^1_3, s^2_3,~...~s^{n}_3\},~\{s^1_1, s^2_1,~...,~s^{n}_1\}~\}\f$     (Party 3)
         *
         * In this case, calling \f$v(1)\f$ from Party 2 will return a reference to the internal vector at index *j*=1
         * that includes the third share \f$s_3\f$ of each secret value:
         *
         * - \f$\{s^1_3, s^2_3,~...,~s^{n}_3\}\f$
         */
        inline Vector <Share> &operator()(const int &column) {
            return contents[column];
        }

        /**
         * Returns a read-only reference to a column of the EVector.
         * @param column - The column index.
         * @return A read-only reference to a vector of encodings at the given index.
         *
         * For more information, see the version of the same operator that returns a mutable reference.
         */
        inline const Vector <Share> &operator()(const int &column) const {
            return contents[column];
        }

        define_binary_evector_element_op(+);
        define_binary_evector_element_op(-);
        define_binary_evector_element_op(*);
        define_binary_evector_element_op(/);

        define_binary_evector_element_op(&);
        define_binary_evector_element_op(|);
        define_binary_evector_element_op(^);

        define_binary_evector_element_op(>>);
        define_binary_evector_element_op(<<);

        define_binary_evector_element_op(<);
        define_binary_evector_element_op(>);
        define_binary_evector_element_op(==);
        define_binary_evector_element_op(!=);


        define_binary_evector_evector_op(+);
        define_binary_evector_evector_op(-);
        define_binary_evector_evector_op(*);
        define_binary_evector_evector_op(/);

        define_binary_evector_evector_op(&);
        define_binary_evector_evector_op(|);
        define_binary_evector_evector_op(^);

        define_binary_evector_evector_op(>>);
        define_binary_evector_evector_op(<<);

        define_binary_evector_evector_op(<);
        define_binary_evector_evector_op(>);
        define_binary_evector_evector_op(==);
        define_binary_evector_evector_op(!=);

        // Friend classes
        template<typename ShareType, typename EVector>
        friend class BSharedVector;
        template<typename ShareType, typename EVector>
        friend class ASharedVector;
        friend class service::RunTime;
        template<typename InputType, typename ReturnType, typename ObjectType>
        friend class tva::service::Task_1;
        template<typename InputType, typename ReturnType, typename ObjectType>
        friend class tva::service::Task_2;
};

} // namespace tva

#endif // TVA_E_VECTOR_H
