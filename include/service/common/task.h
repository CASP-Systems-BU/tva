#ifndef TVA_SERVICE_TASK_H
#define TVA_SERVICE_TASK_H

#include <functional>

namespace tva{ namespace service {

    class Task {
    protected:
        int start;
        int end;
        int batch_size;

    public:
        Task(const int &_start, const int &_end, const int &_batch_size) :
                start(_start), end(_end), batch_size(_batch_size) {
        }

        virtual ~Task() {}
        virtual void execute() = 0;
    };

    template<typename InputType, typename ReturnType, typename...T>
    class Task_ARGS_RTR_1 : public Task {
        InputType x;
        ReturnType res;

        std::function<bool(InputType &, ReturnType &, const int &, const int &)> func;
    public:
        Task_ARGS_RTR_1(const InputType &_x,
                        ReturnType &_res,
                        const int &_start, const int &_end, const int &_batch_size,
                        std::function<bool(InputType &, ReturnType &, const int &, const int &)> _func) :
                Task(_start, _end, _batch_size),
                x(_x), res(_res),
                func(_func) {}

        void execute() override {
            for (int i = start; i < end; i += batch_size) {
                int end_ = std::min(i + batch_size, end);
                func(x, res, i, end_);
            }
        }
    };

    template<typename InputType, typename ReturnType, typename...T>
    class Task_ARGS_RTR_2 : public Task {
        InputType x;
        InputType y;
        ReturnType res;

        std::function<bool(InputType &, InputType &, ReturnType &, const int &, const int &)> func;
    public:
        Task_ARGS_RTR_2(const InputType &_x,
                        const InputType &_y,
                        ReturnType &_res,
                        const int &_start, const int &_end, const int &_batch_size,
                        std::function<bool(InputType &, InputType &, ReturnType &, const int &, const int &)> _func) :
                Task(_start, _end, _batch_size),
                x(_x), y(_y), res(_res),
                func(_func) {}

        void execute() override {
            for (int i = start; i < end; i += batch_size) {
                int end_ = std::min(i + batch_size, end);
                func(x, y, res, i, end_);
            }
        }
    };

    template<typename InputType>
    class Task_ARGS_VOID_1 : public Task {
        InputType x;

        std::function<bool(InputType &, const int &, const int &)> func;
    public:
        Task_ARGS_VOID_1(const InputType &_x,
                         const int &_start, const int &_end, const int &_batch_size,
                         std::function<bool(InputType &, const int &, const int &)> _func) :
                Task(_start, _end, _batch_size),
                x(_x),
                func(_func) {}

        void execute() override {
            for (int i = start; i < end; i += batch_size) {
                int end_ = std::min(i + batch_size, end);
                func(x, i, end_);
            }
        }
    };

    template<typename InputType>
    class Task_ARGS_VOID_2 : public Task {
        InputType x;
        InputType y;

        std::function<bool(InputType &, InputType &, const int &, const int &)> func;
    public:
        Task_ARGS_VOID_2(const InputType &_x, const InputType &_y,
                         const int &_start, const int &_end, const int &_batch_size,
                         std::function<bool(InputType &, InputType &, const int &, const int &)> _func) :
                Task(_start, _end, _batch_size),
                x(_x),
                y(_y),
                func(_func) {}

        void execute() override {
            for (int i = start; i < end; i += batch_size) {
                int end_ = std::min(i + batch_size, end);
                func(x, y, i, end_);
            }
        }
    };


    template<typename InputType, typename ReturnType, typename ObjectType>
    class Task_1 : public Task {
        InputType x;
        ReturnType res;
        ObjectType *object;

        ReturnType (ObjectType::*func)(const InputType &x);

    public:
        Task_1(InputType &_x,
               ReturnType &_res,
               const int &_start, const int &_end, const int &_batch_size,
               ObjectType *_object,
               ReturnType (ObjectType::*_func)(const InputType &x)) :
                Task(_start, _end, _batch_size),
                x(_x), res(_res),
                object(_object),
                func(_func) {}

        void execute() override {
            for (int i = start; i < end; i += batch_size) {
                int end_ = std::min(i + batch_size, end);
                x.set_batch(i, end_);
                res.set_batch(i, end_);
                res = (object->*func)(x);
            }
        }
    };


    template<typename InputType, typename ReturnType, typename ObjectType>
    class Task_2 : public Task {
        InputType x;
        InputType y;
        ReturnType res;
        ObjectType *object;

        ReturnType (ObjectType::*func)(const InputType &x,
                                         const InputType &y);

    public:
        Task_2(InputType &_x,
               InputType &_y,
               ReturnType &_res,
               const int &_start, const int &_end, const int &_batch_size,
               ObjectType *_object,
               ReturnType (ObjectType::*_func)(
                       const InputType &x,
                       const InputType &y)) :
                Task(_start, _end, _batch_size),
                x(_x), y(_y), res(_res),
                object(_object),
                func(_func) {}

        void execute() override {
            for (int i = start; i < end; i += batch_size) {
                int end_ = std::min(i + batch_size, end);
                x.set_batch(i, end_);
                y.set_batch(i, end_);
                res.set_batch(i, end_);
                res = (object->*func)(x, y);
            }
        }
    };
} } // namespace tva::service

#endif // TVA_SERVICE_TASK_H
