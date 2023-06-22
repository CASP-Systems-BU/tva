#ifndef TVA_VECTOR_DATA_H
#define TVA_VECTOR_DATA_H

#include <vector>
#include "../../debug/debug.h"
#include "vector_cache.h"

namespace tva{

    typedef int VectorDataSize;

    template<typename DataType> class VectorDataBase;
    template<typename DataType> class VectorData;
    template<typename DataType> class SimpleVectorData;
    template<typename DataType> class AlternatingVectorData;
    template<typename DataType> class ReversedAlternatingVectorData;
    template<typename DataType> class ReversedVectorData;
    template<typename DataType> class RepeatedVectorData;
    template<typename DataType> class CyclicVectorData;



    template<typename DataType>
    class VectorDataBase {
    private:
        VectorDataSize current_size;

        inline virtual std::shared_ptr<VectorDataBase<DataType>> copy_shared_ptr() = 0;
    public:

        VectorDataBase(const VectorDataSize& _current_size):
                       current_size(_current_size){}

        VectorDataBase():
        current_size(0){}

        virtual ~VectorDataBase(){}

        /**
         * @return An iterator pointing to the first element.
         *
         * NOTE: This method is used by the communicator.
         */
        inline virtual typename std::vector<DataType>::iterator begin() = 0;

        /**
         * @return An iterator pointing to the last element.
         *
         * NOTE: This method is used by the communicator.
         */
        inline virtual typename std::vector<DataType>::iterator end() = 0;

        /**
         * Returns a mutable reference to the element at the given `index`.
         * @param index - The index of the target element.
         * @return A mutable reference to the element at the given `index`.
         */
        inline virtual DataType &operator[](const int &index) = 0;

        /**
         * Returns an immutable reference of the element at the given `index`.
         * @param index - The index of the target element.
         * @return Returns a read-only reference of the element at the given `index`.
         */
        inline virtual const DataType &operator[](const int &index) const = 0;

        SimpleVectorData<DataType> simple_subset_reference(const VectorDataSize &_start_index,
                                                                   const VectorDataSize &_end_index,
                                                                   const VectorDataSize &_step){
            return SimpleVectorData<DataType>(this->copy_shared_ptr(), _start_index, _end_index, _step);
        }

        AlternatingVectorData<DataType> alternating_subset_reference(const VectorDataSize &_included_size,
                                                                             const VectorDataSize &_excluded_size){
            return AlternatingVectorData<DataType>(this->copy_shared_ptr(), _included_size, _excluded_size);
        }

        ReversedAlternatingVectorData<DataType> reversed_alternating_subset_reference(const VectorDataSize &_included_size,
                                              const VectorDataSize &_excluded_size) {
            return ReversedAlternatingVectorData<DataType>(this->copy_shared_ptr(), _included_size, _excluded_size);
        }

        ReversedVectorData<DataType> directed_subset_reference(){
            return ReversedVectorData<DataType>(this->copy_shared_ptr());
        }

        RepeatedVectorData<DataType> repeated_subset_reference(const VectorDataSize &_repetition_number){
            return RepeatedVectorData<DataType>(this->copy_shared_ptr(), _repetition_number);
        }

        CyclicVectorData<DataType> cyclic_subset_reference(const VectorDataSize &_cycles_number){
            return CyclicVectorData<DataType>(this->copy_shared_ptr(), _cycles_number);
        }


        /**
         * @return The total number of elements in the vector.
         */
        inline VectorDataSize size() const {
            return current_size;
        }
    };


    template<typename DataType>
    class VectorData : public VectorDataBase<DataType>{
    private:
        std::vector<DataType>* data;

        inline virtual std::shared_ptr<VectorDataBase<DataType>> copy_shared_ptr(){
            return std::shared_ptr<VectorDataBase<DataType>>(new VectorData(*this));
        }
    public:

#if defined(RECYCLE_THREAD_MEMORY)
        static thread_local std::shared_ptr<VectorCache<DataType>> vectorCache;
        std::shared_ptr<VectorCache<DataType>> sourceVectorCachePtr;

        VectorData(const VectorDataSize &_size, DataType _init_val = 0) :
                data(vectorCache.get()->AllocateVector(_size)),
                VectorDataBase<DataType>(_size) {
            auto &ptr = *data;
            for (int i = 0; i < _size; ++i) {
                ptr[i] = _init_val;
            }
            sourceVectorCachePtr = vectorCache;
        }

        ~VectorData() {
            vectorCache.get()->deallocateVector(data);
        }
#else
        VectorData(const VectorDataSize &_size, DataType _init_val = 0) :
                data(new std::vector<DataType>(_size, _init_val)),
                VectorDataBase<DataType>(_size) {}

        ~VectorData() {
            delete data;
        }
#endif

        VectorData(std::initializer_list<DataType> &&elements) :
                VectorData(elements.size(), 0) {
            auto &ptr = *data;
            for (int i = 0; i < elements.size(); ++i) {
                ptr[i] = elements[i];
            }
        }

        VectorData(const std::vector<DataType> &_data) :
                VectorData(_data.size(), 0) {
            auto &ptr = *data;
            for (int i = 0; i < _data.size(); ++i) {
                ptr[i] = _data[i];
            }
        }

        VectorData(const std::vector<DataType> &&_data) :
                VectorData(_data.size(), 0) {
            auto &ptr = *data;
            for (int i = 0; i < _data.size(); ++i) {
                ptr[i] = _data[i];
            }
        }

        // VectorData(const std::shared_ptr<std::vector<DataType>> &_data) :
        //         data(_data), VectorDataBase<DataType>(_data.get()->size()) {}

        // VectorData(const std::shared_ptr<std::vector<DataType>> &&_data) :
        //         data(_data), VectorDataBase<DataType>(_data.get()->size()) {}

        inline typename std::vector<DataType>::iterator begin(){
            return data->begin();
        }

        inline typename std::vector<DataType>::iterator end(){
            return data->end();
        }

        inline DataType &operator[](const int &index){
            return (*data)[index];
        }

        inline const DataType &operator[](const int &index) const{
            return (*data)[index];
        }
    };

#if defined(RECYCLE_THREAD_MEMORY)
    template<typename DataType>
    thread_local std::shared_ptr<VectorCache<DataType>>
            VectorData<DataType>::vectorCache =
            std::shared_ptr<VectorCache<DataType>>(new VectorCache<DataType>());
#endif

    template<typename DataType>
    class SimpleVectorData : public VectorDataBase<DataType>{
    private:
        std::shared_ptr<VectorDataBase<DataType>> data;

        VectorDataSize start_index;
        VectorDataSize end_index;
        VectorDataSize step;

        inline virtual std::shared_ptr<VectorDataBase<DataType>> copy_shared_ptr(){
            return std::shared_ptr<VectorDataBase<DataType>>(new SimpleVectorData(*this));
        }
    public:
        SimpleVectorData(const std::shared_ptr<VectorDataBase<DataType>> &_data,
                         const VectorDataSize &_start_index,
                         const VectorDataSize &_end_index,
                         const VectorDataSize &_step) :
                data(_data), start_index(_start_index), end_index(_end_index), step(_step),
                VectorDataBase<DataType>(std::min(_data.get()->size(), (_end_index - _start_index) / _step + 1)) {}

        inline typename std::vector<DataType>::iterator begin(){
            return data.get()->begin() + start_index;
        }

        inline typename std::vector<DataType>::iterator end(){
            return data.get()->begin() + end_index;
        }

        inline DataType &operator[](const int &index){
            return (*data.get())[start_index + index * step];
        }

        inline const DataType &operator[](const int &index) const{
            return (*data.get())[start_index + index * step];
        }
    };


    template<typename DataType>
    class AlternatingVectorData : public VectorDataBase<DataType>{
    private:
        std::shared_ptr<VectorDataBase<DataType>> data;

        VectorDataSize included_size;
        VectorDataSize excluded_size;

        VectorDataSize chunk_size;          // (included_size + excluded_size)

        inline virtual std::shared_ptr<VectorDataBase<DataType>> copy_shared_ptr(){
            return std::shared_ptr<VectorDataBase<DataType>>(new AlternatingVectorData(*this));
        }
    public:
        AlternatingVectorData(const std::shared_ptr<VectorDataBase<DataType>> &_data,
                              const VectorDataSize &_included_size,
                              const VectorDataSize &_excluded_size) :
                data(_data), included_size(_included_size), excluded_size(_excluded_size),
                chunk_size((_included_size + _excluded_size)),
                VectorDataBase<DataType>(_data.get()->size() / (_included_size + _excluded_size)
                                         * (_included_size)
                                         + std::min(_included_size,
                                                    _data.get()->size() % (_included_size + _excluded_size))) {}

        inline typename std::vector<DataType>::iterator begin(){
            return data.get()->begin();
        }

        // TODO: optimize this?
        inline typename std::vector<DataType>::iterator end(){
            return data.get()->begin() + (((this->size() - 1) / included_size) * chunk_size + ((this->size() - 1) % included_size));
        }

        inline DataType &operator[](const int &index){
            return (*data.get())[(index / included_size) * chunk_size + (index % included_size)];
        }

        inline const DataType &operator[](const int &index) const{
            return (*data.get())[(index / included_size) * chunk_size + (index % included_size)];
        }
    };

    template<typename DataType>
    class ReversedAlternatingVectorData : public VectorDataBase<DataType>{
    private:
        std::shared_ptr<VectorDataBase<DataType>> data;

        VectorDataSize included_size;
        VectorDataSize excluded_size;

        VectorDataSize chunk_size;          // (included_size + excluded_size)

        inline virtual std::shared_ptr<VectorDataBase<DataType>> copy_shared_ptr(){
            return std::shared_ptr<VectorDataBase<DataType>>(new ReversedAlternatingVectorData(*this));
        }
    public:
        ReversedAlternatingVectorData(const std::shared_ptr<VectorDataBase<DataType>> &_data,
                              const VectorDataSize &_included_size,
                              const VectorDataSize &_excluded_size) :
                data(_data), included_size(_included_size), excluded_size(_excluded_size),
                chunk_size((_included_size + _excluded_size)),
                VectorDataBase<DataType>(_data.get()->size() / (_included_size + _excluded_size)
                                         * (_included_size)
                                         + std::min(_included_size,
                                                    _data.get()->size() % (_included_size + _excluded_size))) {}

        inline typename std::vector<DataType>::iterator begin(){
            return data.get()->begin();
        }

        // TODO: optimize this?
        inline typename std::vector<DataType>::iterator end(){
            return data.get()->begin() + (((this->size() - 1) / included_size) * chunk_size + ((this->size() - 1) % included_size));
        }

        inline DataType &operator[](const int &index){
            const VectorDataSize chunk_number = (index / included_size);
            const VectorDataSize chunk_index = (index % included_size);
            const VectorDataSize chunk_offset = std::min(chunk_number * chunk_size + included_size -1, data.get()->size()-1);

            return (*data.get())[chunk_offset - chunk_index];
        }

        inline const DataType &operator[](const int &index) const{
            const VectorDataSize chunk_number = (index / included_size);
            const VectorDataSize chunk_index = (index % included_size);
            const VectorDataSize chunk_offset = std::min(chunk_number * chunk_size + included_size -1, data.get()->size()-1);
            return (*data.get())[chunk_offset - chunk_index];
        }
    };


    template<typename DataType>
    class ReversedVectorData : public VectorDataBase<DataType>{
    private:
        std::shared_ptr<VectorDataBase<DataType>> data;

        VectorDataSize start_index;         // (data_size - 1)

        inline virtual std::shared_ptr<VectorDataBase<DataType>> copy_shared_ptr(){
            return std::shared_ptr<VectorDataBase<DataType>>(new ReversedVectorData(*this));
        }
    public:
        ReversedVectorData(const std::shared_ptr<VectorDataBase<DataType>> &_data) :
                data(_data), start_index(std::max(_data.get()->size() - 1, 0)),
                VectorDataBase<DataType>(_data.get()->size()) {}

        inline typename std::vector<DataType>::iterator begin(){
            return data.get()->begin() + start_index;
        }

        inline typename std::vector<DataType>::iterator end(){
            return data.get()->begin();
        }

        inline DataType &operator[](const int &index){
            return (*data.get())[start_index - index];
        }

        inline const DataType &operator[](const int &index) const{
            return (*data.get())[start_index - index];
        }
    };

    template<typename DataType>
    class RepeatedVectorData : public VectorDataBase<DataType>{
    private:
        std::shared_ptr<VectorDataBase<DataType>> data;

        VectorDataSize data_size;
        VectorDataSize repetition_number;

        inline virtual std::shared_ptr<VectorDataBase<DataType>> copy_shared_ptr(){
            return std::shared_ptr<VectorDataBase<DataType>>(new RepeatedVectorData(*this));
        }
    public:
        RepeatedVectorData(const std::shared_ptr<VectorDataBase<DataType>> &_data,
                           const VectorDataSize& _repetition_number) :
                data(_data), data_size(_data.get()->size()), repetition_number(_repetition_number),
                VectorDataBase<DataType>(_data.get()->size() * _repetition_number) {}

        inline typename std::vector<DataType>::iterator begin(){
            return data.get()->begin();
        }

        inline typename std::vector<DataType>::iterator end(){
            return data.get()->begin() + (data_size - 1);
        }

        inline DataType &operator[](const int &index){
            return (*data.get())[index / repetition_number];
        }

        inline const DataType &operator[](const int &index) const{
            return (*data.get())[index / repetition_number];
        }
    };

    template<typename DataType>
    class CyclicVectorData : public VectorDataBase<DataType>{
    private:
        std::shared_ptr<VectorDataBase<DataType>> data;

        VectorDataSize data_size;
        VectorDataSize cycles_number;

        inline virtual std::shared_ptr<VectorDataBase<DataType>> copy_shared_ptr(){
            return std::shared_ptr<VectorDataBase<DataType>>(new CyclicVectorData(*this));
        }
    public:
        CyclicVectorData(const std::shared_ptr<VectorDataBase<DataType>> &_data,
                           const VectorDataSize& _cycles_number) :
                data(_data), data_size(_data.get()->size()), cycles_number(_cycles_number),
                VectorDataBase<DataType>(_data.get()->size() * _cycles_number) {}

        inline typename std::vector<DataType>::iterator begin(){
            return data.get()->begin();
        }

        inline typename std::vector<DataType>::iterator end(){
            return data.get()->begin() + (data_size - 1);
        }

        inline DataType &operator[](const int &index){
            return (*data.get())[index % data_size];
        }

        inline const DataType &operator[](const int &index) const{
            return (*data.get())[index % data_size];
        }
    };





}

#endif //TVA_VECTOR_DATA_H
