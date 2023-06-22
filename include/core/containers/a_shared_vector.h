#ifndef TVA_A_SHARED_VECTOR_H
#define TVA_A_SHARED_VECTOR_H

#include "shared_vector.h"


namespace tva {

    /**
     * A SharedVector that contains arithmetic shares and supports secure arithmetic operations.
     * @tparam Share - Share data type.
     * @tparam EVector - Share container type.
     */
    template<typename Share, typename EVector>
    class ASharedVector : public SharedVector<Share, EVector> {
    public:

        /**
         * Creates an ASharedVector of size `_size` and initializes it with zeros.
         * @param _size - The size of the ASharedVector.
         */
        explicit ASharedVector(const int& _size) : SharedVector<Share, EVector>(_size, Encoding::AShared) {}

        /**
         * This is a shallow copy constructor from EVector contents.
         * @param _shares - The EVector whose contents will be pointed by the ASharedVector.
         */
        explicit ASharedVector(EVector& _shares ) : SharedVector<Share, EVector>(_shares, Encoding::AShared) {}

        /**
         * This is a move constructor from EVector contents.
         * @param _shares - The EVector whose contents will be moved to the new ASharedVector.
         */
        ASharedVector(EVector&& _shares ) : SharedVector<Share, EVector>(_shares, Encoding::AShared) {}

        /**
         * This is a move constructor from another ASharedVector.
         * @param other - The ASharedVector whose contents will be moved to the new ASharedVector.
         */
        ASharedVector(ASharedVector&& other) noexcept : SharedVector<Share, EVector>(other.vector, other.encoding) {}

        /**
         * This is a copy constructor from another ASharedVector.
         * @param other - The ASharedVector whose contents will be moved to the new ASharedVector.
         */
        ASharedVector(const ASharedVector& other) : SharedVector<Share, EVector>(other.vector, other.encoding) {}

        /**
         * Copy constructor from SharedVector contents.
         * @param _shares - The SharedVector object whose contents will be copied to the new ASharedVector.
         */
        explicit ASharedVector(SharedVector<Share, EVector> &_shares) : EncodedVector(_shares.encoding) {
            assert(_shares.encoding==Encoding::AShared);
            auto secretShares_ = reinterpret_cast<ASharedVector *>(&_shares);
            this->vector = secretShares_->vector;
        }

        /**
         * Move constructor that creates an ASharedVector from a unique pointer to an EncodedVector object.
         * @param base - The pointer to the SharedVector object whose contents will be moved to the new ASharedVector.
         */
        ASharedVector(std::unique_ptr<ASharedVector>&& base) : ASharedVector((ASharedVector*) base.get()){}

        /**
         * Shallow copy constructor that creates an ASharedVector from a unique pointer to an EncodedVector object.
         * @param base - The SharedVector object whose contents will be pointed by the new ASharedVector.
         */
        ASharedVector(std::unique_ptr<ASharedVector>& base) : ASharedVector((ASharedVector*) base.get()){}

        /**
         * Move constructor that creates an ASharedVector from a pointer to another ASharedVector object.
         * @param _base - The ASharedVector that will be moved as a whole (contents + state) to the new ASharedVector.
         *
         * NOTE: This constructor is implicitly called by the two constructors above.
         */
        explicit ASharedVector(ASharedVector *_base) : ASharedVector(std::move(*_base)) {}

        /**
         * This is a deep move assignment. Applies the move assignment operator to EVector.
         * @param aSecretShares - The ASharedVector whose contents will be moved to the current ASharedVector.
         * @return
         */
        ASharedVector& operator = (const ASharedVector&& aSecretShares ) {
            this->encoding = aSecretShares.encoding;
            this->vector = aSecretShares.vector;
            return *this;
        }

        /**
         * This is a deep copy assignment. Applies the move assignment operator to EVector.
         * @param aSecretShares - The ASharedVector whose contents will be moved to the current ASharedVector.
         * @return
         */
        ASharedVector& operator = (const ASharedVector& aSecretShares ) {
            this->encoding = aSecretShares.encoding;
            this->vector = aSecretShares.vector;
            return *this;
        }

        /**
         * This is a deep move assignment from a unique pointer to a EncodedVector object.
         * Applies the move assignment operator to EVector.
         * @param base - The pointer to the SharedVector object whose contents will be moved to the new ASharedVector.
         * @return A reference to this ASharedVector after modification.
         */
        ASharedVector& operator = (std::unique_ptr<ASharedVector>&& aSecretShares ) {
            this->encoding = aSecretShares.get()->encoding;
            this->vector = aSecretShares.get()->vector;
            return *this;
        }

        /**
         * This is a copy assignment from a unique pointer to a EncodedVector object.
         * Applies the copy assignment operator to EVector.
         * @param base - The SharedVector object whose contents will be copied to the new ASharedVector.
         * @return A reference to this ASharedVector after modification.
         */
        ASharedVector& operator = ( const std::unique_ptr<ASharedVector>& aSecretShares ) {
            auto& _aSecretShares = *aSecretShares.get();
            this->encoding = _aSecretShares.encoding;
            this->vector = _aSecretShares.vector;
            return *this;
        }

        // Destructor
        virtual ~ASharedVector() {}

        // // TODO: Refactor this functions
        // template<typename...T>
        // ASharedVector subset_reference(T... args) const {
        //     return ASharedVector(this->vector.subset_reference(args...));
        // }

        template<typename...T>
        ASharedVector simple_subset_reference(T... args) const {
            return ASharedVector(this->vector.simple_subset_reference(args...));
        }

        template<typename...T>
        ASharedVector alternating_subset_reference(T... args) const {
            return ASharedVector(this->vector.alternating_subset_reference(args...));
        }

        template<typename...T>
        ASharedVector reversed_alternating_subset_reference(T... args) const {
            return ASharedVector(this->vector.reversed_alternating_subset_reference(args...));
        }

        template<typename...T>
        ASharedVector repeated_subset_reference(T... args) const {
            return ASharedVector(this->vector.repeated_subset_reference(args...));
        }

        template<typename...T>
        ASharedVector cyclic_subset_reference(T... args) const {
            return ASharedVector(this->vector.cyclic_subset_reference(args...));
        }

        template<typename...T>
        ASharedVector directed_subset_reference(T... args) const {
            return ASharedVector(this->vector.directed_subset_reference(args...));
        }


        /**
         * This is a conversion from ASharedVector to BSharedVector.
         */
        std::unique_ptr<BSharedVector<Share, EVector>> a2b() {
            auto b_vecs = service::runTime.redistribute_shares_b(this->vector);
            BSharedVector<Share, EVector> vec1 = std::move(b_vecs[0]);
            BSharedVector<Share, EVector> vec2 = std::move(b_vecs[1]);
            return vec1 + vec2;
        }

        std::unique_ptr<ASharedVector> div(const Share &c) {
            // compute division and error correction shares
            auto out = service::runTime.div_const_a(this->vector, c);

#ifdef USE_DIVISION_CORRECTION
            ASharedVector out_res = std::move(out[0]);
            ASharedVector out_err = std::move(out[1]);

            // convert a to b 
            BSharedVector<Share, EVector> berr = out_err.a2b();

            // compute ltz
            BSharedVector<Share, EVector> ltzerr = !berr.ltz();

            // convert b back to a
            ASharedVector correction = ltzerr.b2a_bit();

            return out_res + correction;
#else
            return std::unique_ptr<ASharedVector>(new ASharedVector(std::move(out[0])));
#endif
        }
        
        
        // **************************************** //
        //           Arithmetic operators           //
        // **************************************** //

        /**
         * Elementwise secure arithmetic addition.
         * @param other - The second operand of addition.
         * @return A unique pointer to a new shared vector that contains arithmetic shares of
         * the elementwise additions.
         */
        binary_op(+, ASharedVector, add_a, this, other);

        /**
         * Elementwise secure arithmetic subtraction.
         * This operator expects both input vectors (`this` and `other`) to have the same size.
         * @param other - The second operand of subtraction.
         * @return A unique pointer to a new shared vector that contains arithmetic shares of
         * the elementwise subtractions.
         */
        binary_op(-, ASharedVector, sub_a, this, other);

        /**
         * Elementwise secure arithmetic multiplication.
         * This operator expects both input vectors (`this` and `other`) to have the same size.
         * @param other - The second operand of multiplication.
         * @return A unique pointer to a new shared vector that contains arithmetic shares of
         * the elementwise multiplications.
         */
        binary_op(*, ASharedVector, multiply_a, this, other);

        /**
         * Elementwise secure arithmetic negation.
         * This operator expects the input vector (`this`) to contain arithmetic shares.
         * @return A unique pointer to a new shared vector with all arithmetic shares of
         * `this` vector negated.
         */
        unary_op(-, ASharedVector, neg_a, this);

        /**
         * Opens this vector to all computing parties.
         * @return The opened (plaintext) vector.
         */
        Vector<Share> open() const {
            return service::runTime.open_shares_a(this->vector);
        }
    };

}

#endif //TVA_A_SHARED_VECTOR_H
