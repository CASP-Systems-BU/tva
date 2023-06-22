#ifndef TVA_SHARED_VECTOR_H
#define TVA_SHARED_VECTOR_H

#include "e_vector.h"
#include "encoded_vector.h"

namespace tva {

    // Forward class declarations
    template<typename Share, typename EVector>
    class BSharedVector;
    template<typename Share, typename EVector>
    class ASharedVector;

    /**
     * A secret-shared vector with share and container types.
     * @tparam Share - Share data type.
     * @tparam EVector - Share container type.
     *
     * A SharedVector is an "encoded view" of a plaintext vector as seen from an untrusted party. Different parties in
     * a MPC protocol have different "views" of the same plaintext vector and views may vary significantly across
     * protocols. Currently, TVA supports two techniques to construct a SharedVector: *arithmetic and boolean secret
     * sharing*. Using these techniques, a secret value *s* is encoded using *n* > 1 random "shares" such that:
     *
     * - \f$s = s_1 + s_2 + ... + s_n~\texttt{mod}~2^\ell\f$, where \f$\ell\f$ is the length of *s* in bits (Arithmetic)
     *
     * - \f$s = s_1 \oplus s_2 \oplus ... \oplus s_n\f$, where \f$\oplus\f$ denotes the bitwise XOR operation (Boolean)
     *
     * Let \f(v = \{4, 12, 84\}\f) be a plaintext vector that is secret-shared by 2 parties using arithmetic
     * sharing. From the viewpoint of each party, vector \f(v\f) will look like this:
     *
     * \f(v = \{-1, 12, 100\}   (This is the encoded view of Party 1)\f)
     *
     * \f(v = \{5, 0, -16\}~~~  (This is the encoded view of Party 2)\f)
     *
     * These two vectors are in practice SharedVectors containing random numbers that add up to the numbers in the
     * original vector (which remains hidden from the parties). To reconstruct the original vector in this example,
     * the parties must "open" their SharedVector to a single entity (i.e., a learner) to apply the elementwise
     * addition. The methods to construct and open SharedVectors are defined in Protocol.
     */
    template<typename Share, typename EVector>
    class SharedVector : public EncodedVector {
    protected:
        // The contents of the shared vector
        EVector vector;
    public:

        /**
         * Creates a SharedVector of size `_size` and initializes it with zeros.
         * @param _size - The size of the SharedVector.
         * @param eType - The encoding of the SharedVector.
         */
        explicit SharedVector(const int &_size, const Encoding& eType) : EncodedVector(eType), vector(_size) {}

        /**
         * This is a shallow copy constructor from EVector.
         * @param _shares - The EVector whose contents will be pointed by the SharedVector.
         */
        explicit SharedVector(const EVector &_shares, const Encoding& eType) : EncodedVector(eType), vector(_shares) {}

        /**
         * This is a move constructor from SharedVector.
         * @param secretShares - The SharedVector whose contents will be moved to the new SharedVector.
         */
        SharedVector(SharedVector &&secretShares) noexcept : EncodedVector(secretShares.encoding),
                                                            vector(secretShares.vector) {}

        SharedVector(SharedVector &secretShares) : EncodedVector(secretShares.encoding),
                                                            vector(secretShares.vector) {}

        /**
         * Copy constructor from EncodedVector.
         * @param _shares - The EncodedVector object whose contents will be copied to the new SharedVector.
         */
        explicit SharedVector(EncodedVector &_shares) : EncodedVector(_shares.encoding) {
            auto secretShares_ = reinterpret_cast<SharedVector *>(&_shares);
            this->encoding = secretShares_.encoding;
            this->vector = secretShares_->vector;
        }

        // Destructor
        virtual ~SharedVector() {}

        /**
         * Opens the shared vector to all computing parties.
         * @return The opened (plaintext) vector.
         */
        virtual Vector<Share> open() const = 0;

        /**
         * Populates the shared vector with pseudo-randomly generated shares.
         */
        void populatePRandom(){
            // TODO: There is one gen and comm for each thread? How do I know that I access the right one?
            service::runTime.randomGenerators[0]->getMultipleRandom(this->vector(0), this->vector.size());
            if (service::runTime.protocols_32[0]->replicationNumber == 2) {
                service::runTime.communicators[0]->exchangeShares(this->vector(0),
                                                                  this->vector(1),
                                                                  1, 2,
                                                                  this->vector.size());
            }
        }

        /**
         * @return The size of the shared vector in number of elements.
         */
        size_t size() const {
            return vector.size();
        }

        /**
         * Transforms this vector into an EVector object.
         * @return An EVector object with the same contents as `this` shared vector.
         *
         * NOTE: This method is useful for developers who need access to the underlying shares of the SharedVector
         * that are only exposed through the EVector. Use it if you are certain about what you are doing.
         */
        EVector asEVector() {
            return EVector(this->vector);
        }

//        virtual inline tva::Vector<Share>& operator()(const int& index){
//            return vector(index);
//        }
    };
}

#endif //TVA_SHARED_VECTOR_H
