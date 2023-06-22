#ifndef TVA_PROTOCOL_H
#define TVA_PROTOCOL_H

#include "../communication/communicator.h"
#include "../../debug/debug.h"
#include "../random/random_generator.h"

#include <vector>

namespace tva {
    // The protocol base
    class ProtocolBase{
    public:
        ProtocolBase(PartyID pID, int partiesNum, int replicationNum) :
                    partyID(pID),
                    partiesNumber(partiesNum),
                    replicationNumber(replicationNum) { };

        virtual ~ProtocolBase() {}

        // The unique id of the party that created the Protocol instance
        PartyID partyID;
        // The total number of computing parties participating in the protocol execution
        const int partiesNumber;
        // The replication factor
        const int replicationNumber;
    };
    /**
     * This is the abstract class that defines the primitive methods each secure protocol must implement.
     * @tparam Data - Plaintext data type.
     * @tparam Share - Share type (e.g., a 32-bit integer, a pair of 64-bit integers, a 256-bit string, etc.).
     * @tparam Vector - Data container type.
     * @tparam EVector - Share container type.
     *
     * Primitive operations are grouped as follows:
      *  1. Arithmetic operations on arithmetic shares.
      *  2. Boolean operations on boolean shares.
      *  3. Primitives for sending and receiving shares.
      *  4. Primitives for constructing and opening shares to learners.
     */
    template<typename Data, typename Share, typename Vector, typename EVector>
    class Protocol : public ProtocolBase {
        public:
            // The communicator
            Communicator *communicator;
            // The random number generator
            RandomGenerator *randomGenerator;

            /**
             * Protocol constructor:
             * @param _communicator - A pointer to the communicator.
             * @param _randomGenerator - A pointer to the random number generator.
             * @param _partyID - The (globally) unique identifier of the party that calls this constructor.
             * @param _partiesNumber - The total number of computing parties participating in the protocol execution.
             * @param _replicationNumber - The protocol's replication factor.
             */
            Protocol(Communicator *_communicator,
                     RandomGenerator *_randomGenerator,
                     PartyID _partyID,
                     const int &_partiesNumber, const int &_replicationNumber) :
                        ProtocolBase(_partyID, _partiesNumber, _replicationNumber){
                this->communicator = _communicator;
                this->randomGenerator = _randomGenerator;
            }
            /// Destructor
            virtual ~Protocol() {}

            // **************************************** //
            //          Arithmetic operations           //
            // **************************************** //

            /**
             * Defines vectorized arithmetic addition.
             * This method must take two input vectors with arithmetic shares and return a new
             * vector that contains arithmetic shares of the elementwise additions.
             * @param first - The first shared vector of size S.
             * @param second - The second shared vector of size S.
             * @return A new shared vector v of size S such that v[i] = first[i] + second[i], 0 <= i < S.
             */
            virtual EVector add_a(const EVector &first, const EVector &second) = 0;
            /**
             * Defines vectorized arithmetic subtraction.
             * This method must take two input vectors with arithmetic shares and return a new
             * vector that contains arithmetic shares of the elementwise subtractions.
             * @param first - The first shared vector of size S.
             * @param second - The second shared vector of size S.
             * @return A new shared vector v of size S such that v[i] = first[i] - second[i], 0 <= i < S.
             */
            virtual EVector sub_a(const EVector &first, const EVector &second) = 0;
            /**
             * Defines vectorized arithmetic multiplication.
             * This method must take two input vectors with arithmetic shares and return a new
             * vector that contains arithmetic shares of the elementwise multiplications.
             * @param first - The first shared vector of size S.
             * @param second - The second shared vector of size S.
             * @return A new shared vector v of size S such that v[i] = first[i] * second[i], 0 <= i < S.
             */
            virtual EVector multiply_a(const EVector &first, const EVector &second) = 0;
            /**
             * Defines vectorized arithmetic negation
             * This method must take one input vector with arithmetic shares and return a new
             * vector with all arithmetic shares negated.
             * @param input - The input shared vector of size S.
             * @return A new shared vector v of size S such that v[i] = -input[i], 0<=i<S.
             */
            virtual EVector neg_a(const EVector &input) = 0;

            virtual std::vector<EVector> div_const_a(const EVector &input, const Data &c) = 0;
            virtual int div_const_a_count() = 0;


            // **************************************** //
            //            Boolean operations            //
            // **************************************** //

            /**
             * Defines vectorized bitwise XOR (^).
             * This method must take two input vectors with boolean shares and return a new
             * vector that contains boolean shares of the elementwise XORs.
             * @param first - The first shared vector of size S.
             * @param second - The second shared vector of size S.
             * @return A new shared vector v of size S such that v[i] = first[i] ^ second[i], 0 <= i < S.
             */
            virtual EVector xor_b(const EVector &first, const EVector &second) = 0;
            /**
             * Defines vectorized bitwise AND (&).
             * This method must take two input vectors with boolean shares and return a new
             * vector that contains boolean shares of the elementwise ANDs.
             * @param first - The first shared vector of size S.
             * @param second - The second shared vector of size S.
             * @return A new shared vector v of size S such that v[i] = first[i] & second[i], 0 <= i < S.
             */
            virtual EVector and_b(const EVector &first, const EVector &second) = 0;
            /**
             * Defines vectorized boolean complement (~).
             * This method must take one input vector with boolean shares and return a new
             * vector with all boolean shares complemented.
             * @param input - The input shared vector of size S.
             * @return A new shared vector v of size S such that v[i] = ~input[i], 0 <= i < S.
             */
            virtual EVector not_b(const EVector &input) = 0;
            /**
             * Defines vectorized boolean NOT (!).
             * This method must take one input vector with boolean shares and return a new
             * vector with all boolean shares negated.
             * @param input - The input shared vector of size S.
             * @return A new shared vector v of size S such that v[i] = !input[i], 0 <= i < S.
             */
            virtual EVector not_b_1(const EVector &input) = 0;

            /**
             * Defines vectorized less-than-zero comparison.
             * This method must take one input vector with boolean shares and return a new
             * vector that contains boolean shares of the elementwise less-than-zero comparisons.
             * @param input - The input shared vector of size S.
             * @return A new shared vector v of size S such that v[i] = 1 if input[i]<0, otherwise v[i] = 0, 0 <= i < S.
             */
            virtual EVector ltz(const EVector &input) = 0;

            // **************************************** //
            //          Conversion operations           //
            // **************************************** //

            /**
             * Defines vectorized boolean-to-arithmetic single bit conversion.
             * @param x - A B-shared vector of S single-bit elements.
             * @return A new A-shared vector v of size S such that v[i] = x[i], 0 <= i < S.
             */
            virtual EVector b2a_bit(const EVector &x) = 0;

            /**
            * Defines a redistribution of arithmetic secret shares into boolean secret shares.
            **/
            virtual std::vector<EVector> redistribute_shares_b(const EVector &x) = 0;

            /**
            * Defines the number of boolean share vectors needed for a redistribution of arithmetic shares. 
            **/
            virtual int redistribute_shares_b_count() = 0;

            // **************************************** //
            //          Reconstruction operations       //
            // **************************************** //

            /**
             * Defines how to reconstruct a single data value by adding its arithmetic shares stored
             * in the input vector.
             * @param shares - The input vector containing arithmetic shares of the secret value.
             * @return The plaintext value of type Data.
             *
             * NOTE: This method is useful when a computing party also acts as learner that receives
             * arithmetic shares from other parties and needs to reconstruct a true value.
             */
            virtual Data reconstruct_from_a(const std::vector<Share> &shares) = 0;
            /**
             * Vectorized version of the reconstruct_from_a() method.
             * This method defines how to reconstruct a vector of *n* data values
             * by adding their respective arithmetic shares in the input shared vectors.
             * @param shares - A vector of shared vectors (each one of size *n*) that contain arithmetic shares.
             * @return A new vector that contains *n* plaintext values of type Data.
             *
             * NOTE: This method is useful when a computing party also acts as learner that receives
             * arithmetic shared vectors from other parties and needs to reconstruct the original vector.
             */
            virtual Vector reconstruct_from_a(const std::vector<EVector> &shares) = 0;
            /**
             * Defines how to reconstruct a single data value by XORing its boolean shares stored
             * in the input vector.
             * @param shares - The input vector containing boolean shares of the secret value.
             * @return The plaintext value of type Data.
             *
             * NOTE: This method is useful when a computing party also acts as learner that receives
             * boolean shares from other parties and needs to reconstruct a true value.
             */
            virtual Data reconstruct_from_b(const std::vector<Share> &shares) = 0;
            /**
             * Vectorized version of the reconstruct_from_b() method.
             * This method defines how to reconstruct a vector of *n* data values
             * by XORing their respective boolean shares in the input shared vectors.
             * @param shares - A vector of shared vectors (each one of size *n*) that contain boolean shares.
             * @return A new vector that contains *n* plaintext values of type Data.
             *
             * NOTE: This method is useful when a computing party also acts as learner that receives
             * boolean shared vectors from other parties and needs to reconstruct the original vector.
             */
            virtual Vector reconstruct_from_b(const std::vector<EVector> &shares) = 0;

            // **************************************** //
            //            Opening operations            //
            // **************************************** //

            /**
             * Defines how to open an a-shared secret value to all parties. After executing this method, all parties
             * will end up having all arithmetic shares of the secret value.
             * @param share - An arithmetic share of the secret value.
             * @return The plaintext value of type Data.
             *
             * NOTE: This method is useful when computing parties need to reveal
             * a secret-shared value to each other.
             */
            virtual Data open_share_a(const Share &share) = 0;
            /**
             * Vectorized version of open_share_a().
             * @param shares - A shared vector that contains arithmetic shares of the secret values.
             * @return A new vector that contains the plaintext values of type Data.
             *
             * NOTE: This method is useful when computing parties need to reveal
             * a secret-shared vector to each other.
             */
            virtual Vector open_shares_a(const EVector &shares) = 0;
            /**
             * Defines how to open a b-shared secret value to all parties. After executing this method, all parties
             * will end up having all boolean shares of the secret value.
             * @param share - A boolean share of the secret value.
             * @return The plaintext value of type Data.
             *
             * NOTE: This method is useful when computing parties need to reveal
             * a secret-shared value to each other.
             */
            virtual Data open_share_b(const Share &share) = 0;
            /**
             * Vectorized version of open_share_b().
             * @param shares - A shared vector that contains boolean shares of the secret values.
             * @return A new vector that contains the plaintext values of type Data.
             *
             * NOTE: This method is useful when computing parties need to reveal
             * a secret-shared vector to each other.
             */
            virtual Vector open_shares_b(const EVector &shares) = 0;

            // **************************************** //
            //        Share generation operations       //
            // **************************************** //

            /**
             * Defines how to construct arithmetic shares of the input data value according to a protocol.
             * @param data - The input value of type Data.
             * @return A vector containing arithmetic shares of the input value.
             *
             * NOTE: This method is useful when a computing party is also a data owner
             * that needs to split its secret data into arithmetic shares, which can then be
             * distributed across computing parties.
             */
            virtual std::vector<Share> get_share_a(const Data &data) = 0;
            /**
             * Vectorized version of the get_share_a() method.
             * @param data A vector of input values of type Data.
             * @return A vector of shared vectors containing arithmetic shares.
             */
            virtual std::vector<EVector> get_shares_a(const Vector &data) = 0;
            /**
             * Defines how to construct boolean shares of the input data value according to a protocol.
             * @param data - The input value of type Data.
             * @return A vector containing boolean shares of the input value.
             *
             * NOTE: This method is useful when a computing party is also a data owner
             * that needs to split its secret data into boolean shares, which can then be
             * distributed across computing parties.
             */
            virtual std::vector<Share> get_share_b(const Data &data) = 0;
            /**
             * Vectorized version of the get_share_b() method.
             * @param data A vector of input values of type Data.
             * @return A vector of shared vectors containing boolean shares.
             */
            virtual std::vector<EVector> get_shares_b(const Vector &data) = 0;
            /**
             * Defines how to replicate local shares across parties according to a protocol.
             *
             * NOTE: Computing parties must have received or generated the local shares before calling this method.
             */
            virtual void replicate_shares() = 0;

            /**
             * Defines how to B-share a plaintext vector according to a protocol.
             * @param data - The plaintext vector that must be secret-shared among computing parties.
             * @return The boolean shared vector of the party that calls this method.
             *
             * NOTE: This method is useful for secret-sharing plaintext data in TVA programs.
             */
            virtual EVector secret_share_b(const Vector &data, const PartyID& data_party) = 0;


            /**
             * Defines how to A-share a plaintext vector according to a protocol.
             * @param data - The plaintext vector that must be secret-shared among computing parties.
             * @return The arithmetic shared vector of the party that calls this method.
             *
             * NOTE: This method is useful for secret-sharing plaintext data in TVA programs.
             */
             virtual EVector secret_share_a(const Vector &data, const PartyID& data_party) = 0;

    };
}

#endif // TVA_PROTOCOL_H
