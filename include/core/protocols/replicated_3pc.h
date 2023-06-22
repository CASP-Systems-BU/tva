#ifndef TVA_REPLICATED_3PC_H
#define TVA_REPLICATED_3PC_H

namespace tva {
    // Assumes: ShareVector has type of pair of gavel::vector
    // Communicator and random data generator still expects gavel::vector
    /**
     * Implements the secure primitives for the 3-party semi-honest protocol
     * by Araki et al. that uses replicated secret sharing.
     * @tparam Data - Plaintext data type.
     * @tparam Share - Replicated share type.
     * @tparam Vector - Data container type.
     * @tparam EVector - Share container type.
     */
    template<typename Data, typename Share, typename Vector, typename EVector>
    class Replicated_3PC : public Protocol<Data, Share, Vector, EVector> {
    public:
        // Configuration Parameters
        static int parties_num;

        /**
         * Constructor for the semi-honest replicated 3-party protocol by Araki et al.
         * @param _partyID - The (globally) unique id of the party that calls this constructor.
         * @param _communicator - A pointer to the communicator.
         * @param _randomGenerator - A pointer to the random number generator.
         */
        Replicated_3PC(PartyID _partyID,
                       Communicator *_communicator,
                       RandomGenerator *_randomGenerator) :
                Protocol<Data, Share, Vector, EVector>(_communicator,
                                                       _randomGenerator,
                                                       _partyID, 3, 2) { }

        EVector add_a(const EVector &x, const EVector &y) {
            return std::vector<Vector>({x(0) + y(0), x(1) + y(1)});
        }

        EVector sub_a(const EVector &x, const EVector &y) {
            return std::vector<Vector>({x(0) - y(0), x(1) - y(1)});
        }

        EVector multiply_a(const EVector &x, const EVector &y) {
            // Number of elements
            long long size = x.size();
            // Generate 'size' random shares of zero
            // TODO (john): Change this so that the generator only gives us the required random numbers
            Vector r(size);
            this->randomGenerator->getMultipleNext(r, 2, size);
            // Local computation
            auto local = (x(0) * y(0)) + (x(0) * y(1)) + (x(1) * y(0)) + r;
            // Communication round
            Vector remote(size);
            this->communicator->exchangeShares(local, remote, 1, 2, size);
            // Return output shared vector
            return std::vector<Vector>({local, remote});
        }

        EVector neg_a(const EVector &x) {
            return std::vector<Vector>({-x(0), -x(1)});
        }

        std::vector<EVector> div_const_a(const EVector &x, const Data &c) {
            auto size = x.size();
            EVector res(size), err(size);

            if (this->partyID == 0) {
                auto x_sum = x(0) + x(1);

#ifdef USE_DIVISION_CORRECTION
                auto x_sum_neg = x_sum < 0;
                x_sum = x_sum - x_sum_neg * c;
                Vector r(size);
                this->randomGenerator->getMultipleNext(r, 4, size);
                res(0) = (x_sum) / c - r;

                this->randomGenerator->getMultipleNext(r, 4, size);
                err(0) = (x_sum) % c + x_sum_neg * c - r;
#else
                Vector r(size);
                this->randomGenerator->getMultipleNext(r, 4, size);
                res(0) = (x_sum) / c - r;
#endif
            } else if (this->partyID == 1) {
#ifdef USE_DIVISION_CORRECTION
                auto x_sum_neg = x(0) < 0;
                res(0) = (x(0) / c) - x_sum_neg;
                err(0) = x(0) % c + x_sum_neg * c - c;
#else
                res(0) = x(0) / c;
#endif
            } else {
                this->randomGenerator->getMultipleNext(res(0), 3, size);
#ifdef USE_DIVISION_CORRECTION
                this->randomGenerator->getMultipleNext(err(0), 3, size);
#endif

            }

            this->communicator->exchangeShares(res(0), res(1), 1, 2, size);
#ifdef USE_DIVISION_CORRECTION
            this->communicator->exchangeShares(err(0), err(1), 1, 2, size);
#endif

            return std::vector<EVector>({res, err});
        }

        int div_const_a_count() {
            return 2;
        }

        EVector xor_b(const EVector &x, const EVector &y) {
            return std::vector<Vector>({x(0) ^ y(0), x(1) ^ y(1)});
        }

        EVector and_b(const EVector &x, const EVector &y) {
            // Number of elements
            long long size = x.size();
            // Generate 'size' random shares of zero
            // TODO (john): Change this so that the generator only gives us the required random numbers using each seed
            Vector r(size);
            this->randomGenerator->getMultipleNext(r, 5, size);
            // Local computation
            auto local = (x(0) & y(0)) ^ (x(0) & y(1)) ^ (x(1) & y(0)) ^ r;
            // Communication round
            Vector remote(size);
            this->communicator->exchangeShares(local, remote, 1, 2, size);
            // Return output shared vector
            return std::vector<Vector>({local, remote});
        }

        EVector not_b(const EVector &x) {
            return std::vector<Vector>({~x(0), ~x(1)});
        }

        EVector not_b_1(const EVector &x) {
            return std::vector<Vector>({!x(0), !x(1)});
        }

        EVector ltz(const EVector &x) {
            return std::vector<Vector>({x(0).ltz(), x(1).ltz()});
        }

        EVector b2a_bit(const EVector &x) {
            // The number of bits per element
            static const int bits_per_element = 1;

            EVector res(x.size());
            Vector r(x.size(), 0);

            // TODO: Add a new channel for the random generator
            //  that have the shares for A/B Shares of random numbers
            // Compression (1)
            EVector res_compressed = x.simple_bit_compress(0, 1, bits_per_element - 1, 1);
            Vector r_compressed(res_compressed.size(), 0);
            // this->randomGenerator->getMultipleRandom(r_compressed, x.size());

            // TODO: is this communication useless?
            res_compressed(0) = res_compressed(0) ^ r_compressed;
            this->communicator->exchangeShares(res_compressed(0), res_compressed(1), 1, 2, res_compressed.size());
            auto res_open_compressed = this->open_shares_b(res_compressed);

            // Decompression (1)
            res(0).simple_bit_decompress(res_open_compressed, 0, 1, bits_per_element - 1, 1);
            r.simple_bit_decompress(r_compressed, 0, 1, bits_per_element - 1, 1);

            // Computation
            Data pid_mod = this->partyID % 2;
            res(0) = ((((r) * ((res(0) * (-2)) + 1)) + (res(0) * pid_mod))) & 1;

            // Compression (2)
            res_compressed(0) = res(0).simple_bit_compress(0, 1, bits_per_element - 1, 1);

            // Communication
            this->communicator->exchangeShares(res_compressed(0), res_compressed(1), 1, 2, res_compressed.size());

            // Decompression (2)
            res.simple_bit_decompress(res_compressed, 0, 1, bits_per_element - 1, 1);

            return res;
        }


        std::vector<EVector> redistribute_shares_b(const EVector &x) {
            std::vector<EVector> res;
            Vector vec = x(0) + x(1);
            res.push_back(secret_share_b(vec, 0));
            res.push_back(secret_share_b(x(0), 1));
            return res;
        }

        int redistribute_shares_b_count() {
            return 2;
        }

        Data reconstruct_from_a(const std::vector<Share> &shares) {
            return shares[0][0] + shares[1][0] + shares[2][0];
        }

        Vector reconstruct_from_a(const std::vector<EVector> &shares) {
            return shares[0](0) + shares[1](0) + shares[2](0);
        }

        Data reconstruct_from_b(const std::vector<Share> &shares) {
            return shares[0][0] ^ shares[1][0] ^ shares[2][0];
        }

        Vector reconstruct_from_b(const std::vector<EVector> &shares) {
            return shares[0](0) ^ shares[1](0) ^ shares[2](0);
        }

        Data open_share_a(const Share &share) {
            // TODO: implement this function
            std::cerr << "Method 'open_share_a()' is not supported by Replicated_3PC." << std::endl;
            exit(-1);
        }

        Vector open_shares_a(const EVector &shares) {
            // Parties open their local shares to other parties
            int size = shares.size();
            Vector shares_3(size);
            this->communicator->exchangeShares(shares(1), shares_3, 1, 2, size);
            return shares(0) + shares(1) + shares_3;
        }

        Data open_share_b(const Share &share) {
            // TODO: implement this function
            std::cerr << "Method 'open_share_b()' is not supported by Replicated_3PC." << std::endl;
            exit(-1);
        }

        Vector open_shares_b(const EVector &shares) {
            // Parties open their local shares to other parties
            int size = shares.size();
            Vector shares_3(size);
            this->communicator->exchangeShares(shares(1), shares_3, 1, 2, size);
            return shares(0) ^ shares(1) ^ shares_3;
        }

        std::vector<Share> get_share_a(const Data &data) {
            Data share_1, share_2;
            this->randomGenerator->getRandom(share_1);
            this->randomGenerator->getRandom(share_2);
            Data share_3 = data - share_1 - share_2;
            // Return vector of replicated arithmetic shares
            return {{share_1, share_3},
                    {share_2, share_1},
                    {share_3, share_2}};
        }

        std::vector<EVector> get_shares_a(const Vector &data) {
            Vector share_1(data.size()), share_2(data.size());
            this->randomGenerator->getMultipleRandom(share_1, data.size());
            this->randomGenerator->getMultipleRandom(share_2, data.size());
            auto share_3 = data - share_1 - share_2;
            // Return vector of replicated a-shared vectors
            return {std::vector<Vector>({share_1, share_3}),
                    std::vector<Vector>({share_2, share_1}),
                    std::vector<Vector>({share_3, share_2})};
        }

        std::vector<Share> get_share_b(const Data &data) {
            Data share_1, share_2;
            this->randomGenerator->getRandom(share_1);
            this->randomGenerator->getRandom(share_2);
            Data share_3 = data ^ share_1 ^ share_2;
            // Return vector of replicated boolean shares
            return {{share_1, share_3},
                    {share_2, share_1},
                    {share_3, share_2}};
        }

        std::vector<EVector> get_shares_b(const Vector &data) {
            Vector share_1(data.size()), share_2(data.size());
            this->randomGenerator->getMultipleRandom(share_1, data.size());
            this->randomGenerator->getMultipleRandom(share_2, data.size());
            auto share_3 = data ^ share_1 ^ share_2;
            // Return vector of replicated b-shared vectors
            return {std::vector<Vector>({share_1, share_3}),
                    std::vector<Vector>({share_2, share_1}),
                    std::vector<Vector>({share_3, share_2})};
        }

        void replicate_shares() {
            // TODO (john): implement this function
            std::cerr << "Method 'replicate_shares()' is not supported by Replicated_3PC." << std::endl;
            exit(-1);
        }

        EVector secret_share_b(const Vector &data, const PartyID& data_party = 0) {
            auto size = data.size();
            if (this->partyID == data_party) {
                // Generate shares
                auto boolean_shares = get_shares_b(data);
                // Send first shared vector to the successor
                this->communicator->sendShares(boolean_shares[1](0), 1, size);
                this->communicator->sendShares(boolean_shares[1](1), 1, size);
                // Send second shared vector to the predecessor
                this->communicator->sendShares(boolean_shares[2](0), 2, size);
                this->communicator->sendShares(boolean_shares[2](1), 2, size);
                return boolean_shares[0];
            } else {
                EVector s(size);
                // Receive second shared vector from the predecessor
                this->communicator->receiveShares(s(0), data_party-this->partyID, size);
                this->communicator->receiveShares(s(1), data_party-this->partyID, size);
                return s;
            }
        }

        EVector secret_share_a(const Vector &data, const PartyID& data_party = 0) {
            auto size = data.size();
            if (this->partyID == data_party) {
                // Generate shares
                auto arithmetic_shares = get_shares_a(data);
                // Send first shared vector to the successor
                this->communicator->sendShares(arithmetic_shares[1](0), 1, size);
                this->communicator->sendShares(arithmetic_shares[1](1), 1, size);
                // Send second shared vector to the predecessor
                this->communicator->sendShares(arithmetic_shares[2](0), 2, size);
                this->communicator->sendShares(arithmetic_shares[2](1), 2, size);
                return arithmetic_shares[0];
            } else {
                EVector s(size);
                // Receive second shared vector from the predecessor
                this->communicator->receiveShares(s(0), data_party-this->partyID, size);
                this->communicator->receiveShares(s(1), data_party-this->partyID, size);
                return s;
            }
        }
    };
        // TODO (john): Why not initializing parties_num as static const int?
        template<typename Data, typename Share, typename Vector, typename EVector>
        int Replicated_3PC<Data, Share, Vector, EVector>::parties_num = 3;
}


#endif // TVA_REPLICATED_3PC_H
