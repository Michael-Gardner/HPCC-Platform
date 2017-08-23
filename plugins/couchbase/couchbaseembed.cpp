/*##############################################################################

    HPCC SYSTEMS software Copyright (C) 2016 HPCC Systems®.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
############################################################################## */

#include "couchbaseembed.hpp"

#include "platform.h"
#include "jexcept.hpp"
#include "jlog.hpp"
#include "hqlplugins.hpp"
#include "deftype.hpp"
#include "eclhelper.hpp"
#include "eclrtl.hpp"
#include "eclrtl_imp.hpp"

static const char *g_moduleName = "couchbase";
static const char *g_moduleDescription = "Couchbase Embed Helper";
static const char *g_version = "Couchbase Embed Helper 1.0.0";
static const char *g_compatibleVersions[] = { g_version, nullptr };
static const NullFieldProcessor NULLFIELD(NULL);

extern "C" COUCHBASEEMBED_PLUGIN_API bool getECLPluginDefinition(ECLPluginDefinitionBlock *pb)
{
    if (pb->size == sizeof(ECLPluginDefinitionBlockEx))
    {
        ECLPluginDefinitionBlockEx * pbx = (ECLPluginDefinitionBlockEx *) pb;
        pbx->compatibleVersions = g_compatibleVersions;
    }
    else if (pb->size != sizeof(ECLPluginDefinitionBlock))
        return false;

    pb->magicVersion = PLUGIN_VERSION;
    pb->version = g_version;
    pb->moduleName = g_moduleName;
    pb->ECL = NULL;
    pb->flags = PLUGIN_IMPLICIT_MODULE;
    pb->description = g_moduleDescription;
    return true;
}

namespace couchbaseembed
{
    //--------------------------------------------------------------------------
    // Plugin Classes
    //--------------------------------------------------------------------------

    void reportIfQueryFailure(Couchbase::Query * query)
    {
        auto status = query->meta().status();
        if (status.errcode())
        {
            if (status.isNetworkError())
                failx("NetworkErr: %s", status.description());
            else if (status.isDataError())
                failx("DataErr: %s", status.description());
            else if (status.isInputError())
                failx("InputErr: %s", status.description());
            else if (status.isTemporary())
                failx("TempErr: %s", status.description());
            else
                failx("Couchbase err: %s (%d)", status.description(), status.errcode());
        }

        //consider parsing json result
        if (strstr(query->meta().body().to_string().c_str(), "\"status\": \"errors\""))
            failx("Err: %s", query->meta().body().to_string().c_str());
    }

    CouchbaseRowStream::CouchbaseRowStream(IEngineRowAllocator* resultAllocator, Couchbase::Query * cbaseQuery)
       :   m_resultAllocator(resultAllocator)
    {
        m_currentRow = 0;
        m_shouldRead = true;

        //iterating over result rows and copying them to stringarray
        //is there a way to independently step through original result rows?
        for (auto cbrow : *cbaseQuery)
            m_Rows.append(cbrow.json().to_string().c_str());

        reportIfQueryFailure(cbaseQuery);
    }

    CouchbaseRowStream::~CouchbaseRowStream() {}

    const void * CouchbaseRowStream::nextRow()
    {
        const void * result = NULL;
        if (m_shouldRead && m_currentRow < m_Rows.length())
        {
            auto json = m_Rows.item(m_currentRow++);
            Owned<IPropertyTree> contentTree = createPTreeFromJSONString(json,ipt_caseInsensitive);
            if (contentTree)
            {
                CouchbaseRowBuilder cbRowBuilder(contentTree);
                RtlDynamicRowBuilder rowBuilder(m_resultAllocator);
                const RtlTypeInfo *typeInfo = m_resultAllocator->queryOutputMeta()->queryTypeInfo();
                assertex(typeInfo);
                RtlFieldStrInfo dummyField("<row>", NULL, typeInfo);
                size32_t len = typeInfo->build(rowBuilder, 0, &dummyField, cbRowBuilder);
                return rowBuilder.finalizeRowClear(len);
            }
            else
                failx("Error processing result row");
        }
        return result;
    }

   void CouchbaseRowStream::stop()
   {
       m_resultAllocator.clear();
       m_shouldRead = false;
   }

   Couchbase::Query * CouchbaseConnection::query(Couchbase::QueryCommand * qcommand)
   {
       Couchbase::Status queryStatus;
       Couchbase::Query * pQuery = new Couchbase::Query(*m_pCouchbaseClient, *qcommand, queryStatus); // will be owned by method caller

       if (!queryStatus)
           failx("Couldn't issue query: %s", queryStatus.description());

       if (!pQuery->status())
           failx("Couldn't execute query, reason: %s\nBody is: ", pQuery->meta().body().data());

       if (pQuery->meta().status().errcode() != LCB_SUCCESS )//rows.length() == 0)
           failx("Query execution error: %s", pQuery->meta().body().data());

       return pQuery;
   }

    extern void UNSUPPORTED(const char *feature)
    {
        throw MakeStringException(-1, "UNSUPPORTED feature: %s not supported in %s", feature, g_version);
    }

    extern void failx(const char *message, ...)
    {
        va_list args;
        va_start(args,message);
        StringBuffer msg;
        msg.appendf("%s: ", g_moduleName).valist_appendf(message,args);
        va_end(args);
        rtlFail(0, msg.str());
    }

    extern void fail(const char *message)
    {
        StringBuffer msg;
        msg.appendf("%s: ", g_moduleName).append(message);
        rtlFail(0, msg.str());
    }

    void bindStringParam(unsigned len, const char *value, const RtlFieldInfo * field, Couchbase::QueryCommand * pQcmd)
    {
        VStringBuffer cbPlaceholder("$%s", field->name->queryStr());
        if (pQcmd)
        {
            size32_t utf8chars;
            char *utf8;
            rtlStrToUtf8X(utf8chars, utf8, len, value);
            auto status = pQcmd->named_param(cbPlaceholder.str(), utf8);
            if (!status.success())
                failx("Could not bind Param: %s val: %s", cbPlaceholder.str(), utf8);
        }
        else
            failx("Internal error: detected invalid CouchbaseQueryCommand while attempting to bind to field: %s", cbPlaceholder.str());
    }

    void bindBoolParam(bool value, const RtlFieldInfo * field, Couchbase::QueryCommand * pQcmd)
    {
        VStringBuffer cbPlaceholder("$%s", field->name->queryStr());
        if (pQcmd)
        {
            StringBuffer serialized;
            TokenSerializer tokenSerializer;
            tokenSerializer.serialize(value, serialized);

            auto status = pQcmd->named_param(cbPlaceholder.str(), serialized.str());
            if (!status.success())
                failx("Could not bind Param: %s val: %s", cbPlaceholder.str(), serialized.str());
        }
        else
            failx("Internal error: detected invalid CouchbaseQueryCommand while attempting to bind to field: %s", cbPlaceholder.str());
    }

    void bindDataParam(unsigned len, const void *value, const RtlFieldInfo * field, Couchbase::QueryCommand * pQcmd)
    {
        VStringBuffer cbPlaceholder("$%s", field->name->queryStr());
        if (pQcmd)
        {
            size32_t bytes;
            void *data;
            rtlStrToDataX(bytes, data, len, value);

            auto status = pQcmd->named_param(cbPlaceholder.str(), (char *)data);
            if (!status.success())
                failx("Could not bind Param: %s val: %s", cbPlaceholder.str(), (char *)data);
        }
        else
            failx("Internal error: detected invalid CouchbaseQueryCommand while attempting to bind to field: %s", cbPlaceholder.str());
    }

    void bindIntParam(__int64 value, const RtlFieldInfo * field, Couchbase::QueryCommand * pQcmd)
    {
        VStringBuffer cbPlaceholder("$%s", field->name->queryStr());
        if (pQcmd)
        {
            StringBuffer serialized;
            TokenSerializer tokenSerializer;
            tokenSerializer.serialize(value, serialized);

            auto status = pQcmd->named_param(cbPlaceholder.str(), serialized.str());
            if (!status.success())
                failx("Could not bind Param: %s val: %s", cbPlaceholder.str(), serialized.str());
        }
        else
            failx("Internal error: detected invalid CouchbaseQueryCommand while attempting to bind to field: %s", cbPlaceholder.str());
    }

    void bindUIntParam(unsigned __int64 value, const RtlFieldInfo * field, Couchbase::QueryCommand * pQcmd)
    {
        VStringBuffer cbPlaceholder("$%s", field->name->queryStr());
        if (pQcmd)
        {
            StringBuffer serialized;
            TokenSerializer tokenSerializer;
            tokenSerializer.serialize(value, serialized);

            auto status = pQcmd->named_param(cbPlaceholder.str(), serialized.str());
            if (!status.success())
                failx("Could not bind Param: %s val: %s", cbPlaceholder.str(), serialized.str());
        }
        else
            failx("Internal error: detected invalid CouchbaseQueryCommand while attempting to bind to field: %s", cbPlaceholder.str());
    }

    void bindRealParam(double value, const RtlFieldInfo * field, Couchbase::QueryCommand * pQcmd)
    {
        VStringBuffer cbPlaceholder("$%s", field->name->queryStr());
        if (pQcmd)
        {
            StringBuffer serialized;
            TokenSerializer tokenSerializer;
            tokenSerializer.serialize(value, serialized);
            auto status = pQcmd->named_param(cbPlaceholder.str(), serialized.str());
            if (!status.success())
                failx("Could not bind Param: %s val: %s", cbPlaceholder.str(), serialized.str());
        }
        else
            failx("Internal error: detected invalid CouchbaseQueryCommand while attempting to bind to field: %s", cbPlaceholder.str());
    }

    void bindUnicodeParam(unsigned chars, const UChar *value, const RtlFieldInfo * field, Couchbase::QueryCommand * pQcmd)
    {
        VStringBuffer cbPlaceholder("$%s", field->name->queryStr());
        if (pQcmd)
        {
            size32_t utf8chars;
            char *utf8;
            rtlUnicodeToUtf8X(utf8chars, utf8, chars, value);
            auto status = pQcmd->named_param(cbPlaceholder.str(), utf8);
            if (!status.success())
                failx("Could not bind Param: %s val: %s", cbPlaceholder.str(), utf8);
        }
        else
            failx("Internal error: detected invalid CouchbaseQueryCommand while attempting to bind to field: %s", cbPlaceholder.str());
    }

    int CouchbaseRecordBinder::numFields()
    {
        int count = 0;
        const RtlFieldInfo * const *fields = typeInfo->queryFields();
        assertex(fields);
        while (*fields++)
           count++;
        return count;
    }
    void CouchbaseRecordBinder::processRow(const byte *row)
    {
        thisParam = firstParam;
        typeInfo->process(row, row, &dummyField, *this); // Bind the variables for the current row
    }

    void CouchbaseRecordBinder::processString(unsigned len, const char *value, const RtlFieldInfo * field)
    {
        checkNextParam(field);
        bindStringParam(len, value, field, m_pQcmd);
    }

    void CouchbaseRecordBinder::processBool(bool value, const RtlFieldInfo * field)
    {
        bindBoolParam(value, field, m_pQcmd);
    }

    void CouchbaseRecordBinder::processData(unsigned len, const void *value, const RtlFieldInfo * field)
    {
        bindDataParam(len, value, field, m_pQcmd);
    }

    void CouchbaseRecordBinder::processInt(__int64 value, const RtlFieldInfo * field)
    {
       bindIntParam(value, field, m_pQcmd);
    }

    void CouchbaseRecordBinder::processUInt(unsigned __int64 value, const RtlFieldInfo * field)
    {
       bindUIntParam(value, field,m_pQcmd);
    }

    void CouchbaseRecordBinder::processReal(double value, const RtlFieldInfo * field)
    {
       bindRealParam(value, field, m_pQcmd);
    }

    void CouchbaseRecordBinder::processDecimal(const void *value, unsigned digits, unsigned precision, const RtlFieldInfo * field)
    {
       Decimal val;
       size32_t bytes;
       rtlDataAttr decText;
       val.setDecimal(digits, precision, value);
       val.getStringX(bytes, decText.refstr());
       processUtf8(bytes, decText.getstr(), field);
    }

    void CouchbaseRecordBinder::processUnicode(unsigned chars, const UChar *value, const RtlFieldInfo * field)
    {
       bindUnicodeParam(chars, value, field, m_pQcmd);
    }

    void CouchbaseRecordBinder::processQString(unsigned len, const char *value, const RtlFieldInfo * field)
    {
       size32_t charCount;
       rtlDataAttr text;
       rtlQStrToStrX(charCount, text.refstr(), len, value);
       processUtf8(charCount, text.getstr(), field);
    }

    void CouchbaseRecordBinder::processUtf8(unsigned chars, const char *value, const RtlFieldInfo * field)
    {
       bindStringParam(strlen(value), value, field, m_pQcmd);
    }

    unsigned CouchbaseRecordBinder::checkNextParam(const RtlFieldInfo * field)
    {
       if (logctx.queryTraceLevel() > 4)
           logctx.CTXLOG("Binding %s to %d", str(field->name), thisParam);
       return thisParam++;
    }

    CouchbaseEmbedFunctionContext::CouchbaseEmbedFunctionContext(const IContextLogger &_logctx, const char *options, unsigned _flags)
    : logctx(_logctx), m_NextRow(), m_nextParam(0), m_numParams(0), m_scriptFlags(_flags)
    {
        m_pQuery = nullptr;
        m_pQcmd = nullptr;

        const char *server = "localhost";
        const char *user = "";
        const char *password = "";
        const char *bucketname = "default";
        unsigned port = 8091;
        bool useSSL = false;
        StringBuffer connectionOptions;

        StringArray inputOptions;
        inputOptions.appendList(options, ",");
        ForEachItemIn(idx, inputOptions)
        {
            const char *opt = inputOptions.item(idx);
            const char *val = strchr(opt, '=');
            if (val)
            {
                StringBuffer optName(val-opt, opt);
                val++;
                if (stricmp(optName, "server")==0)
                    server = val;   // Note that lifetime of val is adequate for this to be safe
                else if (stricmp(optName, "port")==0)
                    port = atoi(val);
                else if (stricmp(optName, "user")==0)
                    user = val;
                else if (stricmp(optName, "password")==0)
                    password = val;
                else if (stricmp(optName, "bucket")==0)
                    bucketname = val;
                else if (stricmp(optName, "useSSL")==0)
                    useSSL = clipStrToBool(val);

                //Connection String options
                else if (stricmp(optName,   "detailed_errcodes")==0
                        || stricmp(optName, "operation_timeout")==0
                        || stricmp(optName, "config_total_timeout")==0
                        || stricmp(optName, "http_poolsize")==0
                        || stricmp(optName, "detailed_errcodes")==0)
                    connectionOptions.appendf("%s%s=%s", connectionOptions.length() == 0 ? "?" : "&", optName.str(), val);
                else
                    failx("Unknown option %s", optName.str());
            }
        }

        m_oCBConnection.setown(new CouchbaseConnection(useSSL, server, port, bucketname, user, password, connectionOptions.str()));
        m_oCBConnection->connect();
    }

    CouchbaseEmbedFunctionContext::~CouchbaseEmbedFunctionContext()
    {
        if (m_pQcmd)
        {
            delete m_pQcmd;
            m_pQcmd = nullptr;
        }

        if (m_pQuery)
        {
            delete m_pQuery;
            m_pQuery = nullptr;
        }
    }

    IPropertyTree * CouchbaseEmbedFunctionContext::nextResultRowTree()
    {
        for (auto cbrow : *m_pQuery)
        {
            auto json = cbrow.json().to_string();
            Owned<IPropertyTree> contentTree = createPTreeFromJSONString(json.c_str());
            return contentTree.getLink();
        }

        reportIfQueryFailure(m_pQuery);

        return nullptr;
    }

    IPropertyTreeIterator * CouchbaseEmbedFunctionContext::nextResultRowIterator()
    {
        for (auto cbrow : *m_pQuery)
        {
            auto json = cbrow.json().to_string();
            Owned<IPropertyTree> contentTree = createPTreeFromJSONString(json.c_str());
            if (contentTree)
                return contentTree->getElements("./*");
            failx("Could not fetch next result row.");
            break;
        }

        reportIfQueryFailure(m_pQuery);

        return nullptr;
    }

    const char * CouchbaseEmbedFunctionContext::nextResultScalar()
    {
        m_resultrow.setown(nextResultRowIterator());

        if (m_resultrow)
        {
            m_resultrow->first();
            if(m_resultrow->isValid() == true)
            {
                if (m_resultrow->query().hasChildren())
                    typeError("scalar", "");
                return m_resultrow->query().queryProp("");
            }
            else
                failx("Could not fetch next result column.");
        }
        else
            failx("Could not fetch next result row.");

        return nullptr;
    }

    bool CouchbaseEmbedFunctionContext::getBooleanResult()
    {
        bool mybool;
        auto scalar = nextResultScalar();
        handleDeserializeOutcome(m_tokenDeserializer.deserialize(scalar, mybool), "bool", scalar);

        return mybool;
    }

    void CouchbaseEmbedFunctionContext::getDataResult(size32_t &len, void * &result)
    {
        auto value = nextResultScalar();
        if (value && *value)
        {
            rtlStrToDataX(len, result, strlen(value), value);   // This feels like it may not work to me - will preallocate rather larger than we want
        }
        else
        {
            rtlStrToDataX(len, result, NULLFIELD.resultChars, NULLFIELD.stringResult);
        }
    }

    double CouchbaseEmbedFunctionContext::getRealResult()
    {
        double mydouble = 0.0;
        auto value = nextResultScalar();
        handleDeserializeOutcome(m_tokenDeserializer.deserialize(value, mydouble), "real", value);

        return mydouble;
    }

    __int64 CouchbaseEmbedFunctionContext::getSignedResult()
    {
        __int64 myint64 = 0;
        auto value = nextResultScalar();
        handleDeserializeOutcome(m_tokenDeserializer.deserialize(value, myint64), "signed", value);

        return myint64;
    }

    unsigned __int64 CouchbaseEmbedFunctionContext::getUnsignedResult()
    {
        unsigned __int64 myuint64 = 0;
        auto value = nextResultScalar();
        handleDeserializeOutcome(m_tokenDeserializer.deserialize(value, myuint64), "unsigned", value);

        return myuint64;
    }

    void CouchbaseEmbedFunctionContext::getStringResult(size32_t &chars, char * &result)
    {
        auto value = nextResultScalar();
        if (value && *value)
        {
            unsigned numchars = rtlUtf8Length(strlen(value), value);
            rtlUtf8ToStrX(chars, result, numchars, value);
        }
        else
        {
            rtlStrToStrX(chars, result, NULLFIELD.resultChars, NULLFIELD.stringResult);
        }
    }

    void CouchbaseEmbedFunctionContext::getUTF8Result(size32_t &chars, char * &result)
    {
        getStringResult(chars, result);
    }

    void CouchbaseEmbedFunctionContext::getUnicodeResult(size32_t &chars, UChar * &result)
    {
        auto value = nextResultScalar();
        if (value && *value)
        {
            unsigned numchars = rtlUtf8Length(strlen(value), value);
            rtlUtf8ToUnicodeX(chars, result, numchars, value);
        }
        else
        {
            rtlUnicodeToUnicodeX(chars, result, NULLFIELD.resultChars, NULLFIELD.unicodeResult);
        }
    }

    void CouchbaseEmbedFunctionContext::getDecimalResult(Decimal &value)
    {
        auto text = nextResultScalar();
        if (text && *text)
            value.setString(rtlUtf8Length(strlen(text), text), text);
        else
            value.set(NULLFIELD.decimalResult);
    }

    IRowStream * CouchbaseEmbedFunctionContext::getDatasetResult(IEngineRowAllocator * _resultAllocator)
    {
        Owned<CouchbaseRowStream> cbaseRowStream;
        cbaseRowStream.setown(new CouchbaseRowStream(_resultAllocator, m_pQuery));
        return cbaseRowStream.getLink();
    }

    byte * CouchbaseEmbedFunctionContext::getRowResult(IEngineRowAllocator * _resultAllocator)
    {
        Owned<CouchbaseRowStream> cbaseRowStream;
        cbaseRowStream.setown(new CouchbaseRowStream(_resultAllocator, m_pQuery));
        return (byte *)cbaseRowStream->nextRow();
    }

    size32_t CouchbaseEmbedFunctionContext::getTransformResult(ARowBuilder & rowBuilder)
    {
        execute();

        auto resultrow = nextResultRowTree();
        if (!resultrow)
            fail("Failed to read row");
        if (resultrow->getCount("./*") != 1)
            typeError("row", "");

        CouchbaseRowBuilder couchbaseRowBuilder(resultrow);
        const RtlTypeInfo *typeInfo = rowBuilder.queryAllocator()->queryOutputMeta()->queryTypeInfo();
        assertex(typeInfo);
        RtlFieldStrInfo dummyField("<row>", NULL, typeInfo);
        return typeInfo->build(rowBuilder, 0, &dummyField, couchbaseRowBuilder);
    }

    void CouchbaseEmbedFunctionContext::bindRowParam(const char *name, IOutputMetaData & metaVal, byte *val)
    {
        CouchbaseRecordBinder binder(logctx, metaVal.queryTypeInfo(), m_pQcmd, m_nextParam);
        binder.processRow(val);
        m_nextParam += binder.numFields();
    }

    void CouchbaseEmbedFunctionContext::bindDatasetParam(const char *name, IOutputMetaData & metaVal, IRowStream * val)
    {
        // We only support a single dataset parameter...
        // MORE - look into batch?
        if (m_oInputStream)
        {
            fail("At most one dataset parameter supported");
        }
        m_oInputStream.setown(new CouchbaseDatasetBinder(logctx, LINK(val), metaVal.queryTypeInfo(), m_pQcmd, m_nextParam));
        m_nextParam += m_oInputStream->numFields();
    }

    void CouchbaseEmbedFunctionContext::bindBooleanParam(const char *name, bool val)
    {
        checkNextParam(name);
        StringBuffer serialized;
        m_tokenSerializer.serialize(val, serialized);

        VStringBuffer cbPlaceholder("$%s", name);
        auto status = m_pQcmd->named_param(cbPlaceholder.str(), serialized.str());
        if (!status.success())
            failx("Could not bind Param: %s val: %s", cbPlaceholder.str(), serialized.str());
    }

    void CouchbaseEmbedFunctionContext::bindDataParam(const char *name, size32_t len, const void *val)
    {
        checkNextParam(name);
        VStringBuffer cbPlaceholder("$%s", name);
        size32_t bytes;
        void *data;
        rtlStrToDataX(bytes, data, len, val);
        auto status = m_pQcmd->named_param(cbPlaceholder.str(), (char *)data);
        if (!status.success())
            failx("Could not bind Param: %s val: %s", cbPlaceholder.str(), (char *)data);
    }

    void CouchbaseEmbedFunctionContext::bindFloatParam(const char *name, float val)
    {
        checkNextParam(name);
        StringBuffer serialized;
        m_tokenSerializer.serialize(val, serialized);
        VStringBuffer cbPlaceholder("$%s", name);

        auto status = m_pQcmd->named_param(cbPlaceholder.str(), serialized.str());
        if (!status.success())
            failx("Could not bind Param: %s val: %s", cbPlaceholder.str(), serialized.str());
    }

    void CouchbaseEmbedFunctionContext::bindRealParam(const char *name, double val)
    {
        checkNextParam(name);
        StringBuffer serialized;
        m_tokenSerializer.serialize(val, serialized);
        VStringBuffer cbPlaceholder("$%s", name);

        auto status = m_pQcmd->named_param(cbPlaceholder.str(), serialized.str());
        if (!status.success())
            failx("Could not bind Param: %s val: %s", cbPlaceholder.str(), serialized.str());
    }

    void CouchbaseEmbedFunctionContext::bindSignedSizeParam(const char *name, int size, __int64 val)
    {
        bindSignedParam(name, val);
    }

    void CouchbaseEmbedFunctionContext::bindSignedParam(const char *name, __int64 val)
    {
        checkNextParam(name);
        StringBuffer serialized;
        m_tokenSerializer.serialize(val, serialized);

        VStringBuffer cbPlaceholder("$%s", name);

        auto status = m_pQcmd->named_param(cbPlaceholder.str(), serialized.str());
        if (!status.success())
            failx("Could not bind Param: %s val: %s", cbPlaceholder.str(), serialized.str());
    }

    void CouchbaseEmbedFunctionContext::bindUnsignedSizeParam(const char *name, int size, unsigned __int64 val)
    {
        bindUnsignedParam(name, val);
    }

    void CouchbaseEmbedFunctionContext::bindUnsignedParam(const char *name, unsigned __int64 val)
    {
        checkNextParam(name);
        StringBuffer serialized;
        m_tokenSerializer.serialize(val, serialized);

        VStringBuffer cbPlaceholder("$%s", name);

        auto status = m_pQcmd->named_param(cbPlaceholder.str(), serialized.str());
        if (!status.success())
            failx("Could not bind Param: %s val: %s", cbPlaceholder.str(), serialized.str());
    }

    void CouchbaseEmbedFunctionContext::bindStringParam(const char *name, size32_t len, const char *val)
    {
        checkNextParam(name);
        VStringBuffer cbPlaceholder("$%s", name);
        size32_t utf8chars;
        char *utf8;
        rtlStrToUtf8X(utf8chars, utf8, len, val);
        auto status = m_pQcmd->named_param(cbPlaceholder.str(), utf8);
        if (!status.success())
            failx("Could not bind Param: %s val: %s", cbPlaceholder.str(), utf8);
    }

    void CouchbaseEmbedFunctionContext::bindVStringParam(const char *name, const char *val)
    {
        checkNextParam(name);
        bindStringParam(name, strlen(val), val);
    }

    void CouchbaseEmbedFunctionContext::bindUTF8Param(const char *name, size32_t chars, const char *val)
    {
        checkNextParam(name);
        bindStringParam(name, strlen(val), val);
    }

    void CouchbaseEmbedFunctionContext::bindUnicodeParam(const char *name, size32_t chars, const UChar *val)
    {
        checkNextParam(name);
        VStringBuffer cbPlaceholder("$%s", name);
        size32_t utf8chars;
        char *utf8;
        rtlUnicodeToUtf8X(utf8chars, utf8, chars, val);
        auto status = m_pQcmd->named_param(cbPlaceholder.str(), utf8);
        if (!status.success())
            failx("Could not bind Param: %s val: %s", cbPlaceholder.str(), utf8);
    }

    void CouchbaseEmbedFunctionContext::compileEmbeddedScript(size32_t chars, const char *script)
    {
        if (script && *script)
        {
            // Incoming script is not necessarily null terminated. Note that the chars refers to utf8 characters and not bytes.

            size32_t len = rtlUtf8Size(chars, script);

            if (len > 0)
            {
                StringAttr queryScript;
                queryScript.set(script, len);
                const char * terminatedScript = queryScript.get(); // Now null terminated

                if (m_pQcmd)
                    delete m_pQcmd;

                m_pQcmd = new Couchbase::QueryCommand(terminatedScript);

                if ((m_scriptFlags & EFnoparams) == 0)
                    m_numParams = countParameterPlaceholders(terminatedScript);
                else
                    m_numParams = 0;
            }
            else
                failx("Empty N1QL query detected");
        }
        else
            failx("Empty N1QL query detected");
    }

    void CouchbaseEmbedFunctionContext::callFunction()
    {
        execute();
    }

    void CouchbaseEmbedFunctionContext::execute()
    {
        if (m_oInputStream)
            m_oInputStream->executeAll(m_oCBConnection);
        else
        {
            if (m_pQuery)
                delete m_pQuery;

            m_pQuery = m_oCBConnection->query(m_pQcmd);

            reportIfQueryFailure(m_pQuery);
        }
    }

    unsigned CouchbaseEmbedFunctionContext::checkNextParam(const char *name)
    {
        if (m_nextParam == m_numParams)
            failx("Too many parameters supplied: No matching $<name> placeholder for parameter %s", name);
        return m_nextParam++;
    }

    bool CouchbaseRowBuilder::getBooleanResult(const RtlFieldInfo *field)
    {
        const char * value = nextField(field);

        if (!value || !*value)
        {
            NullFieldProcessor p(field);
            return p.boolResult;
        }

        bool mybool;
        couchbaseembed::handleDeserializeOutcome(m_tokenDeserializer.deserialize(value, mybool), "bool", value);
        return mybool;
    }

    void CouchbaseRowBuilder::getDataResult(const RtlFieldInfo *field, size32_t &len, void * &result)
    {
        const char * value = nextField(field);

        if (!value || !*value)
        {
            NullFieldProcessor p(field);
            rtlStrToDataX(len, result, p.resultChars, p.stringResult);
            return;
        }
        rtlStrToDataX(len, result, strlen(value), value);   // This feels like it may not work to me - will preallocate rather larger than we want
    }

    double CouchbaseRowBuilder::getRealResult(const RtlFieldInfo *field)
    {
        const char * value = nextField(field);

        if (!value || !*value)
        {
            NullFieldProcessor p(field);
            return p.doubleResult;
        }

        double mydouble = 0.0;
        couchbaseembed::handleDeserializeOutcome(m_tokenDeserializer.deserialize(value, mydouble), "real", value);
        return mydouble;
    }

    __int64 CouchbaseRowBuilder::getSignedResult(const RtlFieldInfo *field)
    {
        const char * value = nextField(field);
        if (!value || !*value)
        {
            NullFieldProcessor p(field);
            return p.uintResult;
        }

        __int64 myint64 = 0;
        couchbaseembed::handleDeserializeOutcome(m_tokenDeserializer.deserialize(value, myint64), "signed", value);
        return myint64;
    }

    unsigned __int64 CouchbaseRowBuilder::getUnsignedResult(const RtlFieldInfo *field)
    {
        const char * value = nextField(field);
        if (!value || !*value)
        {
            NullFieldProcessor p(field);
            return p.uintResult;
        }

        unsigned __int64 myuint64 = 0;
        couchbaseembed::handleDeserializeOutcome(m_tokenDeserializer.deserialize(value, myuint64), "unsigned", value);
        return myuint64;
    }

    void CouchbaseRowBuilder::getStringResult(const RtlFieldInfo *field, size32_t &chars, char * &result)
    {
        const char * value = nextField(field);

        if (!value || !*value)
        {
            NullFieldProcessor p(field);
            rtlStrToStrX(chars, result, p.resultChars, p.stringResult);
            return;
        }

        unsigned numchars = rtlUtf8Length(strlen(value), value);  // MORE - is it a good assumption that it is utf8 ? Depends how the database is configured I think
        rtlUtf8ToStrX(chars, result, numchars, value);
        return;
    }

    void CouchbaseRowBuilder::getUTF8Result(const RtlFieldInfo *field, size32_t &chars, char * &result)
    {
        getStringResult(field, chars, result);
        return;
    }

    void CouchbaseRowBuilder::getUnicodeResult(const RtlFieldInfo *field, size32_t &chars, UChar * &result)
    {
        const char * value = nextField(field);

        if (!value || !*value)
        {
            NullFieldProcessor p(field);
            rtlUnicodeToUnicodeX(chars, result, p.resultChars, p.unicodeResult);
            return;
        }

        unsigned numchars = rtlUtf8Length(strlen(value), value);  // MORE - is it a good assumption that it is utf8 ? Depends how the database is configured I think
        rtlUtf8ToUnicodeX(chars, result, numchars, value);
        return;
    }

    void CouchbaseRowBuilder::getDecimalResult(const RtlFieldInfo *field, Decimal &value)
    {
        const char * dvalue = nextField(field);
        if (!dvalue || !*dvalue)
        {
            NullFieldProcessor p(field);
            value.set(p.decimalResult);
            return;
        }

        size32_t chars;
        rtlDataAttr result;
        value.setString(strlen(dvalue), dvalue);
        if (field)
        {
            RtlDecimalTypeInfo *dtype = (RtlDecimalTypeInfo *) field->type;
            value.setPrecision(dtype->getDecimalDigits(), dtype->getDecimalPrecision());
        }
    }

    void CouchbaseRowBuilder::processBeginSet(const RtlFieldInfo * field, bool &isAll)
    {
        isAll = false; // ALL not supported

        StringBuffer    xpath;
        xpathOrName(xpath, field);

        if (!xpath.isEmpty())
        {
            PathTracker     newPathNode(xpath, CPNTSet);
            StringBuffer    newXPath;

            constructNewXPath(newXPath, xpath.str());

            newPathNode.childCount = m_oResultRow->getCount(newXPath);
            m_pathStack.push_back(newPathNode);
        }
        else
        {
            failx("processBeginSet: Field name or xpath missing");
        }
    }

    bool CouchbaseRowBuilder::processNextSet(const RtlFieldInfo * field)
    {
        return m_pathStack.back().childrenProcessed < m_pathStack.back().childCount;
    }

    void CouchbaseRowBuilder::processBeginDataset(const RtlFieldInfo * field)
    {
        StringBuffer    xpath;
        xpathOrName(xpath, field);

        if (!xpath.isEmpty())
        {
            PathTracker     newPathNode(xpath, CPNTDataset);
            StringBuffer    newXPath;

            constructNewXPath(newXPath, xpath.str());

            newPathNode.childCount = m_oResultRow->getCount(newXPath);
            m_pathStack.push_back(newPathNode);
        }
        else
        {
            failx("processBeginDataset: Field name or xpath missing");
        }
    }

    void CouchbaseRowBuilder::processBeginRow(const RtlFieldInfo * field)
    {
        StringBuffer    xpath;
        xpathOrName(xpath, field);

        if (!xpath.isEmpty())
        {
            if (strncmp(xpath.str(), "<nested row>", 12) == 0)
            {
                // Row within child dataset
                if (m_pathStack.back().nodeType == CPNTDataset)
                {
                    m_pathStack.back().currentChildIndex++;
                }
                else
                {
                    failx("<nested row> received with no outer dataset designated");
                }
            }
            else
            {
                m_pathStack.push_back(PathTracker(xpath, CPNTScalar));
            }
        }
        else
        {
            failx("processBeginRow: Field name or xpath missing");
        }
    }

    bool CouchbaseRowBuilder::processNextRow(const RtlFieldInfo * field)
    {
        return m_pathStack.back().childrenProcessed < m_pathStack.back().childCount;
    }

    void CouchbaseRowBuilder::processEndSet(const RtlFieldInfo * field)
    {
        StringBuffer    xpath;
        xpathOrName(xpath, field);

        if (!xpath.isEmpty() && !m_pathStack.empty() && strcmp(xpath.str(), m_pathStack.back().nodeName.str()) == 0)
        {
            m_pathStack.pop_back();
        }
    }

    void CouchbaseRowBuilder::processEndDataset(const RtlFieldInfo * field)
    {
        StringBuffer    xpath;
        xpathOrName(xpath, field);

        if (!xpath.isEmpty())
        {
            if (!m_pathStack.empty() && strcmp(xpath.str(), m_pathStack.back().nodeName.str()) == 0)
            {
                m_pathStack.pop_back();
            }
        }
        else
        {
            failx("processEndDataset: Field name or xpath missing");
        }
    }

    void CouchbaseRowBuilder::processEndRow(const RtlFieldInfo * field)
    {
        StringBuffer    xpath;
        xpathOrName(xpath, field);

        if (!xpath.isEmpty())
        {
            if (!m_pathStack.empty())
            {
                if (m_pathStack.back().nodeType == CPNTDataset)
                {
                    m_pathStack.back().childrenProcessed++;
                }
                else if (strcmp(xpath.str(), m_pathStack.back().nodeName.str()) == 0)
                {
                    m_pathStack.pop_back();
                }
            }
        }
        else
        {
            failx("processEndRow: Field name or xpath missing");
        }
    }

    const char * CouchbaseRowBuilder::nextField(const RtlFieldInfo * field)
    {
        StringBuffer    xpath;
        xpathOrName(xpath, field);

        if (xpath.isEmpty())
        {
            failx("nextField: Field name or xpath missing");
        }

        StringBuffer fullXPath;

        if (!m_pathStack.empty() && m_pathStack.back().nodeType == CPNTSet && strncmp(xpath.str(), "<set element>", 13) == 0)
        {
            m_pathStack.back().currentChildIndex++;
            constructNewXPath(fullXPath, NULL);
            m_pathStack.back().childrenProcessed++;
        }
        else
        {
            constructNewXPath(fullXPath, xpath.str());
        }

        return m_oResultRow->queryProp(fullXPath.str());
    }

    void CouchbaseRowBuilder::xpathOrName(StringBuffer & outXPath, const RtlFieldInfo * field) const
    {
        outXPath.clear();

        if (field->xpath)
        {
            if (field->xpath[0] == xpathCompoundSeparatorChar)
            {
                outXPath.append(field->xpath + 1);
            }
            else
            {
                const char * sep = strchr(field->xpath, xpathCompoundSeparatorChar);

                if (!sep)
                {
                    outXPath.append(field->xpath);
                }
                else
                {
                    outXPath.append(field->xpath, 0, static_cast<size32_t>(sep - field->xpath));
                }
            }
        }
        else
        {
            outXPath.append(str(field->name));
        }
    }

    void CouchbaseRowBuilder::constructNewXPath(StringBuffer& outXPath, const char * nextNode) const
    {
        bool nextNodeIsFromRoot = (nextNode && *nextNode == '/');

        outXPath.clear();

        if (!nextNodeIsFromRoot)
        {
            // Build up full parent xpath using our previous components
            for (std::vector<PathTracker>::const_iterator iter = m_pathStack.begin(); iter != m_pathStack.end(); iter++)
            {
                if (strncmp(iter->nodeName, "<row>", 5) != 0)
                {
                    if (!outXPath.isEmpty())
                    {
                        outXPath.append("/");
                    }
                    outXPath.append(iter->nodeName);
                    if (iter->nodeType == CPNTDataset || iter->nodeType == CPNTSet)
                    {
                        outXPath.appendf("[%d]", iter->currentChildIndex);
                    }
                }
            }
        }

        if (nextNode && *nextNode)
        {
            if (!outXPath.isEmpty())
            {
                outXPath.append("/");
            }
            outXPath.append(nextNode);
        }
    }

    class CouchbaseEmbedContext : public CInterfaceOf<IEmbedContext>
    {
    public:
        virtual IEmbedFunctionContext * createFunctionContext(unsigned flags, const char *options)
        {
            return createFunctionContextEx(NULL, flags, options);
        }

        virtual IEmbedFunctionContext * createFunctionContextEx(ICodeContext * ctx, unsigned flags, const char *options)
        {
            if (flags & EFimport)
            {
                UNSUPPORTED("IMPORT");
                return nullptr;
            }

            else
                return new CouchbaseEmbedFunctionContext(ctx ? ctx->queryContextLogger() : queryDummyContextLogger(), options, flags);
        }

        virtual IEmbedServiceContext * createServiceContext(const char *service, unsigned flags, const char *options)
        {
            throwUnexpected();
            return nullptr;
        }
    };

    extern DECL_EXPORT IEmbedContext* getEmbedContext()
    {
        return new CouchbaseEmbedContext();
    }

    extern DECL_EXPORT bool syntaxCheck(const char *script)
    {
        return true; // TO-DO
    }
} // namespace
