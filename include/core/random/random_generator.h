#ifndef TVA_RANDOM_GENERATOR_H
#define TVA_RANDOM_GENERATOR_H

#include "../containers/e_vector.h"
#include "../../debug/debug.h"

namespace tva {
    typedef int RGChannelID;
    typedef int RGSize;

    class RandomGenerator {

        RGChannelID currentId;      // Current Party's absolute index in the parties ring.

        // NOTE: inheriting classes should implement:
        //  - Functions to add/edit different PRG queues.

    public:
        /**
         * RandomGenerator - Initializes the RandomGenerator base with the current party index.
         * @param _currentId - The absolute index of the party in the parties ring.
         */
        RandomGenerator(RGChannelID _currentId) : currentId(_currentId) {}

        virtual ~RandomGenerator() {}

        /**
         * getNext - Generate the next element of some Pseudo Random Numbers Queue.
         * @param id - The ID of the Pseudo Random Numbers Queue to generate from.
         * @return The next Pseudo Random Number in the queue.
         */
        virtual void getNext(int8_t &num, RGChannelID id) = 0;
        virtual void getNext(int32_t &num, RGChannelID id) = 0;
        virtual void getNext(int64_t &num, RGChannelID id) = 0;

        /**
         * getRandom - Generates a secure not replicable random number.
         * @return The secure not replicable random number.
         */
        virtual void getRandom(int8_t &num) = 0;
        virtual void getRandom(int32_t &num) = 0;
        virtual void getRandom(int64_t &num) = 0;

        /**
         * getMultipleNext - Generate Many next elements of some Pseudo Random Numbers Queue.
         * @param id - The ID of the Pseudo Random Numbers Queue to generate from.
         * @param size - The Number of the Pseudo Random Numbers to be generated.
         * @return The pulled Pseudo Random Numbers.
         */
        virtual void getMultipleNext(Vector <int8_t> &nums, RGChannelID id, RGSize size) = 0;
        virtual void getMultipleNext(Vector <int32_t> &nums, RGChannelID id, RGSize size) = 0;
        virtual void getMultipleNext(Vector <int64_t> &nums, RGChannelID id, RGSize size) = 0;


        /**
         * getMultipleRandom - Generate many secure not replicable random numbers.
         * @param size - The Number of the secure not replicable random numbers to be generated.
         * @return The secure not replicable random numbers.
         */
        virtual void getMultipleRandom(Vector <int8_t> &nums, RGSize size) = 0;
        virtual void getMultipleRandom(Vector <int32_t> &nums, RGSize size) = 0;
        virtual void getMultipleRandom(Vector <int64_t> &nums, RGSize size) = 0;

        // TODO: get a number of elements of some type from channels in ids
        // virtual ShareVector getMultipleNext(std::vector<RGChannelID> ids, RGSize size) = 0;
    };

} // namespace tva

#endif // TVA_RANDOM_GENERATOR_H
