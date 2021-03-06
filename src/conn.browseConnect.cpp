#include "conn.hpp"

using namespace Eos;

namespace Eos {
    struct BrowseConnectOperation : Operation<Connection, BrowseConnectOperation> {
        BrowseConnectOperation(Handle<Value> connectionString)
            : connectionString_(connectionString)
        {
            EOS_DEBUG_METHOD();
            EOS_DEBUG(L"Connection string: %ls\n", *connectionString_);
        }

        static EOS_OPERATION_CONSTRUCTOR(New, Connection) {
            EOS_DEBUG_METHOD();

            if (args.Length() < 3)
                return NanError("Too few arguments");

            if (!args[1]->IsString())
                return NanTypeError("Connection string should be a string");

            (new BrowseConnectOperation(args[1]))->Wrap(args.Holder());

            EOS_OPERATION_CONSTRUCTOR_RETURN();
        }

        static const char* Name() { return "BrowseConnectOperation"; }

    protected:
        SQLRETURN CallOverride() {
            EOS_DEBUG_METHOD();

            return SQLBrowseConnectW(
                Owner()->GetHandle(), 
                *connectionString_, connectionString_.length(),
                outConnectionString_, outConnectionStringBufferLength, &outConnectionStringLength_);
        }

        void CallbackOverride(SQLRETURN ret) {
            EOS_DEBUG_METHOD();

            if (!SQL_SUCCEEDED(ret) && ret != SQL_NEED_DATA)
                return CallbackErrorOverride(ret);

            EOS_DEBUG(L"Final Result: %hi\n", ret);

            Handle<Value> argv[] = { 
                NanUndefined(),
                ret == SQL_NEED_DATA ? NanTrue() : NanFalse(),
                StringFromTChar(outConnectionString_, min(outConnectionStringLength_, (SQLSMALLINT)outConnectionStringBufferLength)) 
            };

            EOS_DEBUG(L"Result: %i, %ls\n", ret, outConnectionString_);
            
            MakeCallback(argv);
        }

    protected:
        WStringValue connectionString_;
        enum { outConnectionStringBufferLength = 4096 };
        SQLWCHAR outConnectionString_[outConnectionStringBufferLength + 1];
        SQLSMALLINT outConnectionStringLength_;
    };
}

NAN_METHOD(Connection::BrowseConnect) {
    EOS_DEBUG_METHOD();

    if (args.Length() < 2)
        return NanThrowError("Connection::BrowseConnect() requires 2 arguments");

    Handle<Value> argv[] = { NanObjectWrapHandle(this), args[0], args[1] };
    return Begin<BrowseConnectOperation>(argv);
}

template<> Persistent<FunctionTemplate> Operation<Connection, BrowseConnectOperation>::constructor_ = Persistent<FunctionTemplate>();
namespace { ClassInitializer<BrowseConnectOperation> ci; }
