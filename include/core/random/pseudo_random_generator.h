#ifndef TVA_PSEUDO_RANDOM_GENERATOR_H
#define TVA_PSEUDO_RANDOM_GENERATOR_H

#include "random_generator.h"
#include <stdlib.h>


namespace tva {


    // TODO:
    //  1 - int_8 is quarter nrand48.
    //  2 - int_64 is double nrand48.


    class PseudoRandomGenerator : public RandomGenerator {

        std::vector<std::vector<unsigned short>> seeds;

        template<typename... T>
        void addShares(std::vector<unsigned short>& seed, T... seeds_) {
            addShares(seed);
            addShares(seeds_...);
        }

        void addShares(std::vector<unsigned short>& seed) {
            seeds.push_back(seed);
        }

    public:
        PseudoRandomGenerator() : RandomGenerator(-1) {}

        template<typename... T>
        PseudoRandomGenerator(T... _shares) : RandomGenerator(-1) {
            addShares(_shares...);
        }

        void getNext(int8_t &num, RGChannelID id) {
            // TODO: implement actual generation
            num = 0;
        }

        void getNext(int32_t &num, RGChannelID id) {
            // TODO: implement actual generation
            num = 0;
        }

        void getNext(int64_t &num, RGChannelID id) {
            // TODO: implement actual generation
            num = 0;
        }

        void getRandom(int8_t &num) {
            num = 0;
        }

        void getRandom(int32_t &num) {
            num = 0;
        }

        void getRandom(int64_t &num) {
            num = 0;
        }

        void getMultipleNext(Vector <int8_t> &nums, RGChannelID id, RGSize size) {
            if (id == 2) {
                for (int i = 0; i < size; ++i) {
#if defined(MPC_USE_RANDOM_GENERATOR_TRIPLES)
                    nums[i] = nrand48(&seeds[1][0]) - nrand48(&seeds[2][0]);
#else
                    nums[i] = 0;
#endif
                }
            } else if (id == 5) {
                for (int i = 0; i < size; ++i) {
#if defined(MPC_USE_RANDOM_GENERATOR_TRIPLES)
                    nums[i] = nrand48(&seeds[1][0]) ^ nrand48(&seeds[2][0]);
#else
                    nums[i] = 0;
#endif
                }
            }else{
                for (int i = 0; i < size; ++i) {
#if defined(MPC_USE_RANDOM_GENERATOR_TRIPLES)
                    nums[i] = nrand48(&seeds[1][0]) ^ nrand48(&seeds[2][0]);
#else
                    nums[i] = 0;
#endif
                }
            }
        }

        void getMultipleNext(Vector <int32_t> &nums, RGChannelID id, RGSize size) {
            if (id == 2) {
                for (int i = 0; i < size; ++i) {
#if defined(MPC_USE_RANDOM_GENERATOR_TRIPLES)
                    nums[i] = nrand48(&seeds[1][0]) - nrand48(&seeds[2][0]);
#else
                    nums[i] = 0;
#endif
                }
            } else if (id == 5) {
                for (int i = 0; i < size; ++i) {
#if defined(MPC_USE_RANDOM_GENERATOR_TRIPLES)
                    nums[i] = nrand48(&seeds[1][0]) ^ nrand48(&seeds[2][0]);
#else
                    nums[i] = 0;
#endif
                }
            } else{
                for (int i = 0; i < size; ++i) {
#if defined(MPC_USE_RANDOM_GENERATOR_TRIPLES)
                    nums[i] = nrand48(&seeds[1][0]) ^ nrand48(&seeds[2][0]);
#else
                    nums[i] = 0;
#endif
                }
            }
        }

        void getMultipleNext(Vector <int64_t> &nums, RGChannelID id, RGSize size) {
            if (id == 2) {
                for (int i = 0; i < size; ++i) {
#if defined(MPC_USE_RANDOM_GENERATOR_TRIPLES)
                    nums[i] = nrand48(&seeds[1][0]) - nrand48(&seeds[2][0]);
#else
                    nums[i] = 0;
#endif
                }
            } else if (id == 5) {
                for (int i = 0; i < size; ++i) {
#if defined(MPC_USE_RANDOM_GENERATOR_TRIPLES)
                    nums[i] = nrand48(&seeds[1][0]) ^ nrand48(&seeds[2][0]);
#else
                    nums[i] = 0;
#endif
                }
            } else{
                for (int i = 0; i < size; ++i) {
#if defined(MPC_USE_RANDOM_GENERATOR_TRIPLES)
                    nums[i] = nrand48(&seeds[1][0]) ^ nrand48(&seeds[2][0]);
#else
                    nums[i] = 0;
#endif
                }
            }
        }

        void getMultipleRandom(Vector <int8_t> &nums, RGSize size) {
            for (int i = 0; i < size; ++i) {
#if defined(MPC_USE_RANDOM_GENERATOR_TRIPLES)
                nums[i] = (nrand48(&seeds[0][0]));
#else
                nums[i] = 0;
#endif
            }
        }

        void getMultipleRandom(Vector <int32_t> &nums, RGSize size) {
            for (int i = 0; i < size; ++i) {
#if defined(MPC_USE_RANDOM_GENERATOR_TRIPLES)
                nums[i] = (nrand48(&seeds[0][0]) << 31) | nrand48(&seeds[0][0]) ;
#else
                nums[i] = 0;
#endif
            }
        }

        void getMultipleRandom(Vector <int64_t> &nums, RGSize size) {
            for (int i = 0; i < size; ++i) {
#if defined(MPC_USE_RANDOM_GENERATOR_TRIPLES)
                nums[i] = (((nrand48(&seeds[0][0]) << 31) | nrand48(&seeds[0][0])) << 31) | nrand48(&seeds[0][0]);
#else
                nums[i] = 0;
#endif
            }
        }
    };
} // namespace tva


#endif //TVA_PSEUDO_RANDOM_GENERATOR_H
