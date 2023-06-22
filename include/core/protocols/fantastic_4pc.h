#ifndef TVA_FANTASTIC_4PC_H
#define TVA_FANTASTIC_4PC_H

//      x2,x3,x4           x3,x4,x1           x4,x1,x2           x1,x2,x3

namespace tva {
    // Assumes: ShareVector has type of pair of gavel::vector
    // Communicator and random data generator still expects gavel::vector
    /**
    
     * Implements the secure primitives for the 4-party malicious protocol
     * by Dalskov et al. that uses replicated secret sharing.
     * @tparam Data - Plaintext data type.
     * @tparam Share - Replicated share type.
     * @tparam Vector - Data container type.
     * @tparam EVector - Share container type.
     */
    template<typename Data, typename Share, typename Vector, typename EVector>
    class Fantastic_4PC : public Protocol<Data, Share, Vector, EVector> {

    public:
        // Configuration Parameters
        static int parties_num;

        Fantastic_4PC(PartyID _partyID,
                      Communicator *_communicator,
                      RandomGenerator *_randomGenerator) :
                Protocol<Data, Share, Vector, EVector>(_communicator,
                                                       _randomGenerator,
                                                       _partyID, 4, 3) {

        }

        EVector add_a(const EVector &x, const EVector &y) {
            return std::vector<Vector>({x(0) + y(0), x(1) + y(1), x(2) + y(2)});
        }

        EVector sub_a(const EVector &x, const EVector &y) {
            return std::vector<Vector>({x(0) - y(0), x(1) - y(1), x(2) - y(2)});
        }


        EVector multiply_a(const EVector &x, const EVector &y) {

            long long size = x.size();

            Vector r_1_1(size), r_1_2(size), r_1_3(size);
            this->randomGenerator->getMultipleNext(r_1_1, size, 1);
            this->randomGenerator->getMultipleNext(r_1_2, size, 2);
            this->randomGenerator->getMultipleNext(r_1_3, size, 3);


            // Only parties: (i = id, j = id + 1, g = id + 2)
            // Inp Shares_1 (4*2): 0 = (0,1), 1 = (1,2), g = (+2, +3)
            auto shares_1_1 = x(0) * y(1) + x(1) * y(0) - r_1_1;
            auto shares_1_2 = x(1) * y(2) + x(2) * y(1) - r_1_2;

            Vector shares_1_2_hash = shares_1_2;
            // TODO: generate hash


            ///////////////////////////////////////////////
            // Inp1:
            //  1- shares_1_1 to g = +2
            //  2- hash(shares_1_2) to g = +3
            //  3- receive shares_1_1_ from g = +2
            //  4- receive hash(shares_1_1_) from g = +1
            ///////////////////////////////////////////////
            Vector shares_1_3(size), shares_1_3_hash(size);
            this->communicator->exchangeShares(shares_1_1, shares_1_3, +2, +2, size);
            this->communicator->exchangeShares(shares_1_2_hash, shares_1_3_hash, +3, +1, size);
            // TODO: generate hash
            // Todo: check Hash
            for (int i = 0; i < size; ++i) {
                if (shares_1_3[i] != shares_1_3_hash[i]) {
                    printf("party %d accuses (%d,%d)\n", this->partyID,
                           (this->partyID + 2) % 4,
                           (this->partyID + 1) % 4);
                    exit(-1);
                }
            }


            // shares_2_1: x2*y4	+	x4*y2
            // shares_2_2: x1*y3	+	x3*y1
            // Only parties: (i = 0, j = 2, g = 3), (i = 3, j = 1, g = 2)
            // Inp Shares_2 (2*2): 0 = (0,2), 1 = (0,2)

            // if i = 0: 
            // send shares_2_1 to +3

            // if i = 1:
            // send hash(shares_2_2) to +1


            // if i = 2:
            // send hash(shares_2_1) to +1
            // receive shares_2_2 from +1
            // receive hash(shares_2_2) from +3

            // if i = 3:
            // send shares_2_2 to +3
            // receive shares_2_1 from +1
            // receive hash(shares_2_1) from +3


            Vector shares_2_1(size), shares_2_1_hash(size),
                    shares_2_2(size), shares_2_2_hash(size),
                    r_2_1(size), r_2_2(size), r_2_3(size);


            if (this->partyID == 1 || this->partyID == 1) {
                this->randomGenerator->getMultipleNext(r_2_3, size, +2);
            }

            if (this->partyID == 0 || this->partyID == 3) {
                this->randomGenerator->getMultipleNext(r_2_1, size, +3);
                shares_2_1 = x(0) * y(2) + x(2) * y(0) - r_2_1;
                this->communicator->sendShares(shares_2_1, +3, size);
            }

            if (this->partyID == 2 || this->partyID == 1) {
                this->randomGenerator->getMultipleNext(r_2_2, size, +1);
                shares_2_1 = x(0) * y(2) + x(2) * y(0) - r_2_2;
                // TODO: generate hash
                this->communicator->sendShares(shares_2_1, +1, size);
            }

            if (this->partyID == 2 || this->partyID == 3) {
                this->communicator->receiveShares(shares_2_2, +1, size);
                this->communicator->receiveShares(shares_2_2_hash, +3, size);
                // TODO: generate hash
                // TODO: check Hash
                for (int i = 0; i < size; ++i) {
                    if (shares_2_2[i] != shares_2_2_hash[i]) {
                        printf("party %d accuses (%d,%d)\n", this->partyID,
                               (this->partyID + 1) % 4,
                               (this->partyID + 3) % 4);
                        exit(-1);
                    }
                }
            }


            // Local share (4*1)
            if (this->partyID == 0) {
                auto replicated_1 = x(0) * y(0) + shares_1_1 + r_1_1 + shares_2_1 + r_2_3;              // 5 of 12
                auto replicated_2 = x(1) * y(1) + shares_1_2 + r_1_2 + r_2_1;                           // 3 of 12
                auto replicated_3 = x(2) * y(2) + shares_1_3 + r_1_3;                                   // 3 of 12
                return std::vector<Vector>({replicated_1, replicated_2, replicated_3});
            } else if (this->partyID == 1) {
                auto replicated_1 = x(0) * y(0) + shares_1_1 + r_1_1 + r_2_3;                           // 3 of 12
                auto replicated_2 = x(1) * y(1) + shares_1_2 + r_1_2 + r_2_2;                           // 3 of 12
                auto replicated_3 = x(2) * y(2) + shares_1_3 + r_1_3 + shares_2_1;                      // 5 of 12
                return std::vector<Vector>({replicated_1, replicated_2, replicated_3});
            } else if (this->partyID == 2) {
                auto replicated_1 = x(0) * y(0) + shares_1_1 + r_1_1;                                   // 3 of 12
                auto replicated_2 = x(1) * y(1) + shares_1_2 + r_1_2 + shares_2_2 + r_2_2;              // 5 of 12
                auto replicated_3 = x(2) * y(2) + shares_1_3 + r_1_3 + shares_2_1;                      // 5 of 12
                return std::vector<Vector>({replicated_1, replicated_2, replicated_3});
            } else {
                auto replicated_1 = x(0) * y(0) + shares_1_1 + r_1_1 + shares_2_1;                      // 5 of 12
                auto replicated_2 = x(1) * y(1) + shares_1_2 + r_1_2 + shares_2_2 + r_2_1;              // 5 of 12
                auto replicated_3 = x(2) * y(2) + shares_1_3 + r_1_3;                                   // 3 of 12
                return std::vector<Vector>({replicated_1, replicated_2, replicated_3});
            }
        }

        EVector neg_a(const EVector &x) {
            return std::vector<Vector>({-x(0), -x(1), -x(2)});
        }


         EVector inp_a(const Vector& x, const int& i, const int& j, const int& g, const int& h){
             EVector res(x.size());

             if (this->partyID == i){
                 // First working as main computer (i in inp)
                 // res(0) = r_1, res(1) = res - r_1, res(2) = 0
                 this->randomGenerator->getMultipleNext(res(0), 1, x.size());
                 res(1) = x - res(0);
                 this->communicator->sendShares(res(1), +1, res(1).size());
             }else if(this->partyID == j){
                 // First working as main computer (j in inp)
                 // res(0) = 0, res(1) = r_2, res(2) = res - r_2
                 this->randomGenerator->getMultipleNext(res(1), 2, x.size());
                 res(2) = x - res(1);
                 this->communicator->sendShares(res(2), +2, res(2).size());
             }else if(this->partyID == g){
                 // working as receiver (g in inp)
                 // res(0) = [res - r_0]_, res(1) = 0, res(2) = 0
                 Vector other(x.size());
                 this->communicator->receiveShares(res(0), +3, x.size());
                 this->communicator->receiveShares(other, +2, x.size());
                 if (!other.same_as(res(0))) {
                     printf("party %d accuses (%d,%d)\n", this->partyID,
                            (this->partyID + 3) % 4,
                            (this->partyID + 2) % 4);
                     exit(-1);
                 }
             }else if (this->partyID == h){
                 // working as random generator (h in inp)
                 // res(0) = 0, res(1) = 0, res(2) = r_3
                 this->randomGenerator->getMultipleNext(res(2), 3, x.size());
             }

             return res;
         }

         // TODO: modify inp to work any sequence of parties.
        EVector inp_b(const Vector& x, const int& i, const int& j, const int& g, const int& h){
            EVector res(x.size());

             // inp(i,j,g,h) algorithm
             // res(0) = r_1, res(1) = res - r_1, res(2) = 0, res(3) = 0

            if (this->partyID == i){
                // printf("%d: sending %d\n", this->partyID, x.size());
                // First working as main computer (i in inp)
                // res(0) = r_1, res(1) = res ^ r_1, res(2) = 0
                this->randomGenerator->getMultipleNext(res(0), 1, x.size());
                res(1) = x ^ res(0);
                this->communicator->sendShares(res(1), +1, res(1).size());
            }else if(this->partyID == j){
                // First working as main computer (j in inp)
                // res(0) = 0, res(1) = r_2, res(2) = res ^ r_2
                this->randomGenerator->getMultipleNext(res(1), 2, x.size());
                res(2) = x ^ res(1);
                this->communicator->sendShares(res(2), +2, res(2).size());
            }else if(this->partyID == g){
                // printf("%d: receiving %d\n", this->partyID, x.size());
                // working as receiver (g in inp)
                // res(0) = [res ^ r_0]_, res(1) = 0, res(2) = 0
                Vector other(x.size());
                this->communicator->receiveShares(res(0), +3, x.size());
                this->communicator->receiveShares(other, +2, x.size());
                if (!other.same_as(res(0))) {
                    printf("party %d accuses (%d,%d)\n", this->partyID,
                           (this->partyID + 3) % 4,
                           (this->partyID + 2) % 4);
                    exit(-1);
                }
            }else if (this->partyID == h){
                // working as random generator (h in inp)
                // res(0) = 0, res(1) = 0, res(2) = r_3
                this->randomGenerator->getMultipleNext(res(2), 3, x.size());
            }

            return res;
        }


        std::vector<EVector> div_const_a(const EVector &x, const Data &c) {
            auto size = x.size();
            EVector res(size), err(size);

            // inp(i,j,g,h) algorithm
            // res(0) = r_1, res(1) = res ^ r_1, res(2) = 0, res(3) = 0
            // inp(0, 3, 1, 2) -- inp(2, 1, 3, 0)
            // shares in 3rd index and random number in 4th index

            // Computation
            if(this->partyID == 0 || this->partyID == 2){
                res(1) = x(0) + x(1);
#ifdef USE_DIVISION_CORRECTION
                auto x_sum_neg = res(1) < 0;

                auto res_ = res(1) / c - x_sum_neg;
                auto err_ = res(1) % c + x_sum_neg * c;
                if(this->partyID == 0){
                    err_ = err_ - c;
                }

                res = inp_a(res_, 0, 3, 1, 2)
                        + inp_a(res_, 2, 1, 3, 0);

                err = inp_a(err_, 0, 3, 1, 2)
                      + inp_a(err_, 2, 1, 3, 0);
#else
                auto res_ = res(1) / c;
                res = inp_a(res_, 0, 3, 1, 2)
                      + inp_a(res_, 2, 1, 3, 0);
#endif

            }else if (this->partyID == 3 || this->partyID == 1){
                res(2) = x(1) + x(2);
#ifdef USE_DIVISION_CORRECTION
                auto x_sum_neg = res(2) < 0;

                auto res_ = res(2) / c - x_sum_neg;
                auto err_ = res(2) % c + x_sum_neg * c;
                if(this->partyID == 3){
                    err_ = err_ - c;
                }

                res = inp_a(res_, 0, 3, 1, 2)
                      + inp_a(res_, 2, 1, 3, 0);

                err = inp_a(err_, 0, 3, 1, 2)
                      + inp_a(err_, 2, 1, 3, 0);
#else
                auto res_ = res(2) / c;
                res = inp_a(res_, 0, 3, 1, 2)
                      + inp_a(res_, 2, 1, 3, 0);
#endif
            }


            return std::vector<EVector>({res, err});
        }

        int div_const_a_count(){
            return 2;
        }


        EVector xor_b(const EVector &x, const EVector &y) {
            return std::vector<Vector>({x(0) ^ y(0), x(1) ^ y(1), x(2) ^ y(2)});
        }


        EVector and_b(const EVector &x, const EVector &y) {

            long long size = x.size();

            Vector r_1_1(size), r_1_2(size), r_1_3(size);
            this->randomGenerator->getMultipleNext(r_1_1, size, 1);
            this->randomGenerator->getMultipleNext(r_1_2, size, 2);
            this->randomGenerator->getMultipleNext(r_1_3, size, 3);

            auto shares_1_1 = (x(0) & y(1)) ^ (x(1) & y(0)) ^ r_1_1;
            auto shares_1_2 = (x(1) & y(2)) ^ (x(2) & y(1)) ^ r_1_2;

            Vector shares_1_2_hash = shares_1_2;
            // TODO: generate hash

            Vector shares_1_3(size), shares_1_3_hash(size);
            this->communicator->exchangeShares(shares_1_1, shares_1_3, +2, +2, size);
            this->communicator->exchangeShares(shares_1_2_hash, shares_1_3_hash, +3, +1, size);
            // TODO: generate hash
            // Todo: check Hash
            for (int i = 0; i < size; ++i) {
                if (shares_1_3[i] != shares_1_3_hash[i]) {
                    printf("party %d accuses (%d,%d)\n", this->partyID,
                           (this->partyID + 2) % 4,
                           (this->partyID + 1) % 4);
                    exit(-1);
                }
            }

            Vector shares_2_1(size), shares_2_1_hash(size),
                    shares_2_2(size), shares_2_2_hash(size),
                    r_2_1(size), r_2_2(size), r_2_3(size);

            if (this->partyID == 1 || this->partyID == 1) {
                this->randomGenerator->getMultipleNext(r_2_3, size, +2);
            }

            // Generating Data part
            if (this->partyID == 0 || this->partyID == 3) {
                this->randomGenerator->getMultipleNext(r_2_1, size, +3);
                shares_2_1 = (x(0) & y(2)) ^ (x(2) & y(0)) ^ r_2_1;
                this->communicator->sendShares(shares_2_1, +3, size);
            }

            // Generating Hash
            if (this->partyID == 2 || this->partyID == 1) {
                this->randomGenerator->getMultipleNext(r_2_2, size, +1);
                shares_2_1 = (x(0) & y(2)) ^ (x(2) & y(0)) ^ r_2_2;
                // TODO: generate hash
                this->communicator->sendShares(shares_2_1, +1, size);
            }

            if (this->partyID == 2 || this->partyID == 3) {
                this->communicator->receiveShares(shares_2_2, +1, size);
                this->communicator->receiveShares(shares_2_2_hash, +3, size);
                // TODO: generate hash
                // TODO: check Hash
                for (int i = 0; i < size; ++i) {
                    if (shares_2_2[i] != shares_2_2_hash[i]) {
                        printf("party %d accuses (%d,%d)\n", this->partyID,
                               (this->partyID + 1) % 4,
                               (this->partyID + 3) % 4);
                        exit(-1);
                    }
                }
            }


            // Local share (4*1)
            if (this->partyID == 0) {
                auto replicated_1 = (x(0) & y(0)) ^ shares_1_1 ^ r_1_1 ^ shares_2_1 ^ r_2_3;        // 3 of 12
                auto replicated_2 = (x(1) & y(1)) ^ shares_1_2 ^ r_1_2 ^ r_2_1;                     // 3 of 12
                auto replicated_3 = (x(2) & y(2)) ^ shares_1_3 ^ r_1_3;                            // 4 of 12
                return std::vector<Vector>({replicated_1, replicated_2, replicated_3});
            } else if (this->partyID == 1) {
                auto replicated_1 = (x(0) & y(0)) ^ shares_1_1 ^ r_1_1 ^ r_2_3;                     // 3 of 12
                auto replicated_2 = (x(1) & y(1)) ^ shares_1_2 ^ r_1_2 ^ r_2_2;                     // 4 of 12
                auto replicated_3 = (x(2) & y(2)) ^ shares_1_3 ^ r_1_3 ^ shares_2_1;                // 4 of 12
                return std::vector<Vector>({replicated_1, replicated_2, replicated_3});
            } else if (this->partyID == 2) {
                auto replicated_1 = (x(0) & y(0)) ^ shares_1_1 ^ r_1_1;                            // 4 of 12
                auto replicated_2 = (x(1) & y(1)) ^ shares_1_2 ^ r_1_2 ^ shares_2_2 ^ r_2_2;        // 4 of 12
                auto replicated_3 = (x(2) & y(2)) ^ shares_1_3 ^ r_1_3 ^ shares_2_1;                // 3 of 12
                return std::vector<Vector>({replicated_1, replicated_2, replicated_3});
            } else {
                auto replicated_1 = (x(0) & y(0)) ^ shares_1_1 ^ r_1_1 ^ shares_2_1;                // 4 of 12
                auto replicated_2 = (x(1) & y(1)) ^ shares_1_2 ^ r_1_2 ^ shares_2_2 ^ r_2_1;        // 3 of 12
                auto replicated_3 = (x(2) & y(2)) ^ shares_1_3 ^ r_1_3;                            // 3 of 12
                return std::vector<Vector>({replicated_1, replicated_2, replicated_3});
            }
        }

        // TODO: does the copy constructor here makes problems?
        EVector not_b(const EVector &x) {
            if (this->partyID == 0) {
                return std::vector<Vector>({x(0), x(1), ~x(2)});
            } else if (this->partyID == 1) {
                return std::vector<Vector>({x(0), ~x(1), x(2)});
            } else if (this->partyID == 2) {
                return std::vector<Vector>({~x(0), x(1), x(2)});
            } else {
                return std::vector<Vector>({x(0), x(1), x(2)});
            }
        }

        EVector not_b_1(const EVector &x) {
            if (this->partyID == 0) {
                return std::vector<Vector>({x(0), x(1), !x(2)});
            } else if (this->partyID == 1) {
                return std::vector<Vector>({x(0), !x(1), x(2)});
            } else if (this->partyID == 2) {
                return std::vector<Vector>({!x(0), x(1), x(2)});
            } else {
                return std::vector<Vector>({x(0), x(1), x(2)});
            }
        }

        EVector ltz(const EVector &x) {
            return std::vector<Vector>({x(0).ltz(), x(1).ltz(), x(2).ltz()});
        }

        EVector b2a_bit(const EVector &x) {
            // The number of bits per element
            static const int bits_per_element = 1;

            // Create output
            EVector res(x.size());
            EVector r(x.size());

            // TODO: Add a new channel for the random generator
            //  that have the shares for A/B Shares of random numbers
            // get the bshares for the eda bit
            // this->randomGenerator->getMultipleNext(r(0), 1, x.size());
            // this->randomGenerator->getMultipleNext(r(1), 2, x.size());
            // this->randomGenerator->getMultipleNext(r(2), 3, x.size());

            // Computation (1)
            res = (x ^ r) & 1;

            // Compression (1)
            EVector res_compressed = res.simple_bit_compress(0, 1, bits_per_element - 1, 1);

            // Communication (1)
            auto res_compressed_open = this->open_shares_b(res_compressed);

            // Decompression (1)
            Vector res_open(x.size(), 0);
            res_open.simple_bit_decompress(res_compressed_open, 0, 1, bits_per_element - 1, 1);

            // get the ashares for the eda bit
            // this->randomGenerator->getMultipleNext(r(0), 1, x.size());
            // this->randomGenerator->getMultipleNext(r(1), 2, x.size());
            // this->randomGenerator->getMultipleNext(r(2), 3, x.size());

            // Computation (2)
            // c + r*(-2c+1)
            int pid_mod_1 = (this->partyID) == 0;
            int pid_mod_2 = ((this->partyID + 1) % 4) == 0;
            int pid_mod_3 = ((this->partyID + 2) % 4) == 0;
            res(0) = (res_open * pid_mod_1 - r(0) * (res_open * 2 - 1)) & 1;
            res(1) = (res_open * pid_mod_2 - r(1) * (res_open * 2 - 1)) & 1;
            res(2) = (res_open * pid_mod_3 - r(2) * (res_open * 2 - 1)) & 1;

            return res;
        }

        // TODO: Check this function correctness
        std::vector<EVector> redistribute_shares_b(const EVector &x) {
            auto size = x.size();
            EVector res_1(size), res_2(size);

            // inp(i,j,g,h) algorithm
            // res(0) = r_1, res(1) = res - r_1, res(2) = 0, res(3) = 0
            // inp(0, 3, 1, 2) -- inp(2, 1, 3, 0)
            // shares in 3rd index and random number in 4th index

            // Computation
            if(this->partyID == 0 || this->partyID == 2){
                auto res_ = x(0) + x(1);

                res_1 = inp_b(res_, 0, 3, 1, 2);
                res_2 = inp_b(res_, 2, 1, 3, 0);

            }else if (this->partyID == 3 || this->partyID == 1){
                auto res_ = x(1) + x(2);

                res_1 = inp_b(res_, 0, 3, 1, 2);
                res_2 = inp_b(res_, 2, 1, 3, 0);
            }
            
            return std::vector<EVector>({res_1, res_2});
        }

        // TODO: Check this function correctness
        int redistribute_shares_b_count(){
            return 2;
        }

        // Shares Opening without communication
        Data reconstruct_from_a(const std::vector<Share> &shares) {
            return shares[0][0] + shares[1][0] + shares[2][0] + shares[3][0];
        }

        Vector reconstruct_from_a(const std::vector<EVector> &shares) {
            return shares[0](0) + shares[1](0) + shares[2](0) + shares[3](0);
        }

        Data reconstruct_from_b(const std::vector<Share> &shares) {
            return shares[0][0] ^ shares[1][0] ^ shares[2][0] ^ shares[3][0];
        }

        Vector reconstruct_from_b(const std::vector<EVector> &shares) {
            return shares[0](0) ^ shares[1](0) ^ shares[2](0) ^ shares[3](0);
        }

        // Shares Opening with communication
        Data open_share_a(const Share &share) {
            // TODO: implement this function after separating share and data element
            return 0;
        }

        Vector open_shares_a(const EVector &shares) {
            // shares have 0 & 1 & 2 ... go fetch 3
            int size = shares.size();
            Vector shares_4(size);
            this->communicator->exchangeShares(shares(0), shares_4, 1, +3, size);
            return shares(0) + shares(1) + shares(2) + shares_4;
        }

        Data open_share_b(const Share &share) {
            // TODO: implement this function
            return 0;
        }

        Vector open_shares_b(const EVector &shares) {
            // shares have 0 & 1 & 2 ... go fetch 3
            int size = shares.size();
            Vector shares_4(size);
            this->communicator->exchangeShares(shares(0), shares_4, 1, +3, size);
            return shares(0) ^ shares(1) ^ shares(2) ^ shares_4;
        }

        // Shares Generation
        std::vector<Share> get_share_a(const Data &data) {
            Data share_1, share_2, share_3;
            this->randomGenerator->getRandom(share_1);
            this->randomGenerator->getRandom(share_2);
            this->randomGenerator->getRandom(share_3);
            Data share_4 = data - share_1 - share_2 - share_3;
            return {{share_2, share_3, share_4},
                    {share_3, share_4, share_1},
                    {share_4, share_1, share_2},
                    {share_1, share_2, share_3}};
        }

        std::vector<EVector> get_shares_a(const Vector &data) {
            Vector share_1(data.size()), share_2(data.size()), share_3(data.size());
            this->randomGenerator->getMultipleRandom(share_1, data.size());
            this->randomGenerator->getMultipleRandom(share_2, data.size());
            this->randomGenerator->getMultipleRandom(share_3, data.size());
            auto share_4 = data - share_1 - share_2 - share_3;
            return {std::vector<Vector>({share_2, share_3, share_4}),
                    std::vector<Vector>({share_3, share_4, share_1}),
                    std::vector<Vector>({share_4, share_1, share_2}),
                    std::vector<Vector>({share_1, share_2, share_3})};
        }

        std::vector<Share> get_share_b(const Data &data) {
            Data share_1, share_2, share_3;
            this->randomGenerator->getRandom(share_1);
            this->randomGenerator->getRandom(share_2);
            this->randomGenerator->getRandom(share_3);
            Data share_4 = data ^ share_1 ^ share_2 ^ share_3;
            return {{share_2, share_3, share_4},
                    {share_3, share_4, share_1},
                    {share_4, share_1, share_2},
                    {share_1, share_2, share_3}};
        }

        std::vector<EVector> get_shares_b(const Vector &data) {
            Vector share_1(data.size()), share_2(data.size()), share_3(data.size());
            this->randomGenerator->getMultipleRandom(share_1, data.size());
            this->randomGenerator->getMultipleRandom(share_2, data.size());
            this->randomGenerator->getMultipleRandom(share_3, data.size());
            auto share_4 = data ^ share_1 ^ share_2 ^ share_3;
            return {std::vector<Vector>({share_2, share_3, share_4}),
                    std::vector<Vector>({share_3, share_4, share_1}),
                    std::vector<Vector>({share_4, share_1, share_2}),
                    std::vector<Vector>({share_1, share_2, share_3})};
        }

        void replicate_shares() {
            // TODO (john): implement this function
            std::cerr << "Method 'replicate_shares()' is not supported by Fantastic_4PC." << std::endl;
            exit(-1);
        }

        EVector secret_share_b(const Vector &data, const PartyID& data_party = 0) {
            auto size = data.size();
            if (this->partyID == data_party) {
                // Generate shares
                auto boolean_shares = get_shares_b(data);
                // Send first shared vector to the successor
                this->communicator->sendShares(boolean_shares[1](0), this->partyID + 1, size);
                this->communicator->sendShares(boolean_shares[1](1), this->partyID + 1, size);
                this->communicator->sendShares(boolean_shares[1](2), this->partyID + 1, size);
                // Send second shared vector to the successor + 1
                this->communicator->sendShares(boolean_shares[2](0), this->partyID + 2, size);
                this->communicator->sendShares(boolean_shares[2](1), this->partyID + 2, size);
                this->communicator->sendShares(boolean_shares[2](2), this->partyID + 2, size);
                // Send second shared vector to the successor + 2
                this->communicator->sendShares(boolean_shares[3](0), this->partyID + 3, size);
                this->communicator->sendShares(boolean_shares[3](1), this->partyID + 3, size);
                this->communicator->sendShares(boolean_shares[3](2), this->partyID + 3, size);
                return boolean_shares[0];
            } else {
                EVector s(size);
                // Receive second shared vector from the predecessor
                this->communicator->receiveShares(s(0), data_party-this->partyID, size);
                this->communicator->receiveShares(s(1), data_party-this->partyID, size);
                this->communicator->receiveShares(s(2), data_party-this->partyID, size);
                return s;
            }
        }

        EVector secret_share_a(const Vector &data, const PartyID& data_party = 0) {
            auto size = data.size();
            if (this->partyID == data_party) {
                // Generate shares
                auto boolean_shares = get_shares_a(data);
                // Send first shared vector to the successor
                this->communicator->sendShares(boolean_shares[1](0), this->partyID + 1, size);
                this->communicator->sendShares(boolean_shares[1](1), this->partyID + 1, size);
                this->communicator->sendShares(boolean_shares[1](2), this->partyID + 1, size);
                // Send second shared vector to the successor + 1
                this->communicator->sendShares(boolean_shares[2](0), this->partyID + 2, size);
                this->communicator->sendShares(boolean_shares[2](1), this->partyID + 2, size);
                this->communicator->sendShares(boolean_shares[2](2), this->partyID + 2, size);
                // Send second shared vector to the successor + 2
                this->communicator->sendShares(boolean_shares[3](0), this->partyID + 3, size);
                this->communicator->sendShares(boolean_shares[3](1), this->partyID + 3, size);
                this->communicator->sendShares(boolean_shares[3](2), this->partyID + 3, size);
                return boolean_shares[0];
            } else {
                EVector s(size);
                // Receive second shared vector from the predecessor
                this->communicator->receiveShares(s(0), data_party-this->partyID, size);
                this->communicator->receiveShares(s(1), data_party-this->partyID, size);
                this->communicator->receiveShares(s(2), data_party-this->partyID, size);
                return s;
            }
        }
    };

    template<typename Data, typename Share, typename Vector, typename EVector>
    int Fantastic_4PC<Data, Share, Vector, EVector>::parties_num = 4;
}


#endif // TVA_FANTASTIC_4PC_H
