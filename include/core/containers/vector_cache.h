#ifndef TVA_VECTOR_CACHE_H
#define TVA_VECTOR_CACHE_H

#include <vector>
#include <queue>
#include <map>
#include <memory>

namespace tva{
    namespace {

        template<typename DataType>
        class VectorCache {
            std::map<int, std::queue<std::vector<DataType> *> > cache;

        public:
            VectorCache() {}

            ~VectorCache() {
                // iterate over cache which is a map.
                // The value inside cache is a queue of vector points.
                // Free the vector pointers.
                for (auto &kv : cache) {
                    while (!kv.second.empty()) {
                        delete kv.second.front();
                        kv.second.pop();
                    }
                }
            }

            std::vector<DataType> *AllocateVector(const int &size) {
                if (cache[size].size() > 0) {
                    auto v = cache[size].front();
                    cache[size].pop();
                    return v;
                } else {
                    return new std::vector<DataType>(size);
                }
            }

            void deallocateVector(std::vector<DataType> *vecPtr) {
                cache[vecPtr->size()].push(vecPtr);
            }
        };

    }
}

#endif //TVA_VECTOR_CACHE_H
