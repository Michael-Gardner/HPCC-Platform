/*##############################################################################
    HPCC SYSTEMS software Copyright (C) 2021 HPCC Systems®.
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

#pragma once

#include "jlog.hpp"
#include "jptree.hpp"
#include "jstring.hpp"
#include "jfile.ipp"


#ifndef ELASTICSTACKLOGACCESS_EXPORTS
#define ELASTICSTACKLOGACCESS_API DECL_IMPORT
#else
#define ELASTICSTACKLOGACCESS_API DECL_EXPORT
#endif

#define COMPONENT_NAME "ES Log Access"

/* undef verify definitions to avoid collision in cpr submodule */
#ifdef verify
    //#pragma message("UNDEFINING 'verify' - Will be redefined by cpr" )
    #undef verify
#endif

#include <cpr/response.h>
#include <elasticlient/client.h>

using namespace elasticlient;

class ELASTICSTACKLOGACCESS_API ElasticStackLogAccess : public CInterface, implements IRemoteLogAccess
{
private:
    const char * type = "elasticstack";

    Owned<IPropertyTree> m_pluginCfg;

    StringBuffer m_globalIndexSearchPattern;
    StringBuffer m_globalSearchColName;
    StringBuffer m_globalIndexTimestampField;

    StringBuffer m_workunitSearchColName;
    StringBuffer m_workunitIndexSearchPattern;

    StringBuffer m_componentsSearchColName;
    StringBuffer m_componentsIndexSearchPattern;

    StringBuffer m_audienceSearchColName;
    StringBuffer m_audienceIndexSearchPattern;

    StringBuffer m_classSearchColName;
    StringBuffer m_classIndexSearchPattern;

    StringBuffer m_defaultDocType; //default doc type to query

    elasticlient::Client m_esClient;
    StringBuffer m_esConnectionStr;

public:
    static constexpr const char * DEFAULT_ES_DOC_TYPE = "_doc";
    static constexpr const char * DEFAULT_ES_PORT = "9200";

    static constexpr int DEFAULT_ES_DOC_LIMIT = 100;
    static constexpr int DEFAULT_ES_DOC_START = 0;

    static constexpr const char * DEFAULT_TS_NAME = "@timestamp";
    static constexpr const char * DEFAULT_INDEX_PATTERN = "filebeat*";
    static constexpr const char * DEFAULT_LOG_COLUMN_NAME = "message";

    static constexpr const char * DEFAULT_HPCC_LOG_SEQ_COL = "hpcc.log.sequence";
    static constexpr const char * DEFAULT_HPCC_LOG_TIMESTAMP_COL = "hpcc.log.timestamp";
    static constexpr const char * DEFAULT_HPCC_LOG_MESS_COL = "hpcc.log.message";
    static constexpr const char * DEFAULT_HPCC_LOG_AUD_COL = "hpcc.log.audience";
    static constexpr const char * DEFAULT_HPCC_LOG_PROCID_COL = "hpcc.log.procid";
    static constexpr const char * DEFAULT_HPCC_LOG_THREADID_COL = "hpcc.log.threadid";
    static constexpr const char * DEFAULT_HPCC_LOG_TYPE_COL = "hpcc.log.class";
    static constexpr const char * DEFAULT_HPCC_LOG_MESSAGE_COL = "hpcc.log.message";
    static constexpr const char * DEFAULT_HPCC_LOG_JOBID_COL = "hpcc.log.jobid";
    static constexpr const char * DEFAULT_HPCC_LOG_COMPONENT_COL = "kubernetes.container.name";

    static constexpr const char * LOGMAP_INDEXPATTERN_PATH = "logmap/global/@storename";
    static constexpr const char * LOGMAP_COLUMNNAME_PATH = "logmap/global/@searchcolumn";
    static constexpr const char * LOGMAP_TIMESTAMPCOL_PATH = "logmap/global/@timestampcolumn";
    IMPLEMENT_IINTERFACE

    ElasticStackLogAccess(const std::vector<std::string> &hostUrlList, IPropertyTree & logAccessPluginConfig);
    virtual ~ElasticStackLogAccess() override = default;

    bool fetchLog(LogAccessConditions & options, StringBuffer & returnbuf, LogAccessLogFormat format);

    cpr::Response performESQuery(LogAccessConditions & options);

    IPropertyTree * getESStatus();
    IPropertyTree * getIndexSearchStatus(const char * indexpattern);
    IPropertyTree * getTimestampTypeFormat(const char * indexpattern, const char * fieldname);

    IPropertyTree * getRemoteLogStoreStatus() { return getESStatus();};
    const char * getRemoteLogAccessType() { return type; }
    IPropertyTree * fetchLogMap() { return m_pluginCfg->queryPropTree("logmap");}
    const char * fetchConnectionStr() { return m_esConnectionStr.str();}
};
