#include "stmt.hpp"
#include "parameter.hpp"
#include "buffer.hpp"

using namespace Eos;
using namespace Buffers;

namespace Eos {
    struct PutDataOperation : Operation<Statement, PutDataOperation> {
        PutDataOperation::PutDataOperation(Parameter* param, Persistent<Object> bufferObject)
            : parameter_(param)
            , bufferObject_(bufferObject)
        {
            EOS_DEBUG_METHOD();
        }

        static EOS_OPERATION_CONSTRUCTOR(New, Statement) {
            EOS_DEBUG_METHOD();

            if (args.Length() < 2)
                return NanThrowError("Too few arguments");

            if (!Parameter::Constructor()->HasInstance(args[1]))
                return NanThrowTypeError("The first parameter should be a Parameter");

            auto param = Parameter::Unwrap(args[1].As<Object>());

            if (param->Indicator() != SQL_NULL_DATA && !param->Buffer())
                return NanThrowError("No parameter data supplied (not even null)");

            param->Ref();

            (new PutDataOperation(param, Persistent<Object>::New(param->BufferObject())))
                ->Wrap(args.Holder());
            NanReturnValue(args.Holder());
        }

        void CallbackOverride(SQLRETURN ret) {
            EOS_DEBUG_METHOD();

            parameter_->Unref();

            if (!SQL_SUCCEEDED(ret) && ret != SQL_NO_DATA && ret != SQL_PARAM_DATA_AVAILABLE && ret != SQL_NEED_DATA)
                return CallbackErrorOverride(ret);

            EOS_DEBUG(L"Final Result: %hi\n", ret);

            Handle<Value> argv[] = { 
                NanUndefined(),
                NanNew<Boolean>(ret == SQL_NEED_DATA),
                NanNew<Boolean>(ret == SQL_PARAM_DATA_AVAILABLE)
            };

            Callback(argv);
        }

        static const char* Name() { return "PutDataOperation"; }

    protected:
        SQLRETURN CallOverride() {
            EOS_DEBUG_METHOD();

            return SQLPutData(
                Owner()->GetHandle(),
                parameter_->Buffer(),
                parameter_->Indicator());
        }

    private:
        Parameter* parameter_;
        Persistent<Object> bufferObject_;
    };
}

NAN_METHOD(Statement::PutData) {
    EOS_DEBUG_METHOD();

    if (args.Length() < 2)
        return NanThrowError("Statement::PutData() requires a parameter and a callback");

    Handle<Value> argv[] = { NanObjectWrapHandle(this), args[0], args[1] };

    return Begin<PutDataOperation>(argv);
}

Persistent<FunctionTemplate> Operation<Statement, PutDataOperation>::constructor_;
namespace { ClassInitializer<PutDataOperation> ci; }
