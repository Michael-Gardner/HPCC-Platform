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

#include "ElasticStackLogAccess.hpp"

#include "platform.h"

#include <string>
#include <vector>
#include <iostream>
#include <json/json.h>

ElasticStackLogAccess::ElasticStackLogAccess(const std::vector<std::string> &hostUrlList, IPropertyTree & logAccessPluginConfig) : m_esClient(hostUrlList)
{
    if (!hostUrlList.at(0).empty())
        m_esConnectionStr.set(hostUrlList.at(0).c_str());

    m_pluginCfg.set(&logAccessPluginConfig);

#ifdef _DEBUG
    StringBuffer xml;
    toXML(m_pluginCfg, xml);
    fprintf(stdout, "RRRRR%s", xml.str());
#endif

    m_globalIndexTimestampField.set(DEFAULT_TS_NAME);
    m_globalIndexSearchPattern.set(DEFAULT_INDEX_PATTERN);

    m_classIndexSearchPattern.set(DEFAULT_HPCC_LOG_TYPE_COL);
    m_workunitSearchColName.set(DEFAULT_HPCC_LOG_JOBID_COL);
    m_componentsSearchColName.set(DEFAULT_HPCC_LOG_COMPONENT_COL);

    Owned<IPropertyTreeIterator> logmapiter = m_pluginCfg->getElements("logmaps");
    ForEach(*logmapiter)
    {
        IPropertyTree & logmap = logmapiter->query();
        const char * logmaptype = logmap.queryProp("@type");
        if (stricmp(logmaptype, "global")==0)
        {
            if (logmap.hasProp("@storename"))
                m_globalIndexSearchPattern = logmap.queryProp("@storename");
            if (logmap.hasProp("@searchcolumn"))
                m_globalSearchColName = logmap.queryProp("@searchcolumn");
            if (logmap.hasProp("@timestampcolumn"))
            m_globalIndexTimestampField = logmap.queryProp("@timestampcolumn");
        }
        else if (stricmp(logmaptype, "workunits")==0)
        {
            if (logmap.hasProp("@storename"))
                m_workunitIndexSearchPattern = logmap.queryProp("@storename");
            if (logmap.hasProp("@searchcolumn"))
                m_workunitSearchColName = logmap.queryProp("@searchcolumn");
        }
        else if (stricmp(logmaptype, "components")==0)
        {
            if (logmap.hasProp("@storename"))
                m_componentsIndexSearchPattern = logmap.queryProp("@storename");
            if (logmap.hasProp("@searchcolumn"))
                m_componentsSearchColName = logmap.queryProp("@searchcolumn");
        }
        else if (stricmp(logmaptype, "class")==0)
        {
            if (logmap.hasProp("@storename"))
                m_classIndexSearchPattern = logmap.queryProp("@storename");
            if (logmap.hasProp("@searchcolumn"))
                m_classSearchColName = logmap.queryProp("@searchcolumn");
        }
    }

#ifdef _DEBUG
    std::string document {"{\"kubernetes.container.name\": \"eclwatch\", \"container.image.name\": \"hpccsystems\\\\core\", \"hpcc.log.message\": \"00000777 USR 2021-06-18 17:55:03.316 975333 975333 W20210513-220848 \\\"CSmartSocketFactory::CSmartSocketFactory(192.168.1.140:9876)\\\"\"}"};

    std::string testindex = "filebeat-7.9.3-2021.06.18-000001";

    try
    {
        // Index the document, index "testindex" must be created before
        cpr::Response indexResponse = m_esClient.index(testindex, DEFAULT_ES_DOC_TYPE, "", document);
        // 200
        std::cout << indexResponse.status_code << std::endl;
        // application/json; charset=UTF-8
        std::cout << "Search results: "<< std::endl;
        // Elasticsearch response (JSON text string)
        std::cout << indexResponse.text << std::endl;
    }
    catch (std::runtime_error &e)
    {
        const char * wha = e.what();
        fprintf(stdout, "Error creating index %s", wha);
    }
    catch (IException * e)
    {
        StringBuffer mess;
        const char * message = e->errorMessage(mess);
        fprintf(stdout, "Error creating index %s", mess.str());
        e->Release();
    }

    try
    {
        // Retrieve the document
        cpr::Response retrievedDocument = m_esClient.get(testindex, DEFAULT_ES_DOC_TYPE, "");
        // 200
        std::cout << retrievedDocument.status_code << std::endl;
        // application/json; charset=UTF-8
        std::cout << retrievedDocument.header["content-type"] << std::endl;
        // Elasticsearch response (JSON text string) where key "_source" contain:
        // {"message": "Hello world!"}
        Owned<IPropertyTree> resptree = createPTreeFromJSONString(retrievedDocument.text.c_str());
        std::cout << retrievedDocument.text << std::endl;

        Json::Value root;
        Json::CharReaderBuilder builder;
        Json::CharReader * reader = builder.newCharReader();

        std::string errors;

        bool parsingSuccessful = reader->parse(retrievedDocument.text.c_str(), retrievedDocument.text.c_str() + retrievedDocument.text.size(), &root, &errors);
        delete reader;

        if (!parsingSuccessful)
            return;

        const Json::Value &items = root["items"];
    }
    catch (std::runtime_error &e)
    {
        const char * wha = e.what();
        fprintf(stdout, "Error retrieving doc: %s", wha);
    }
    catch (IException * e)
    {
        StringBuffer mess;
        const char * message = e->errorMessage(mess);
        fprintf(stdout, "Error retrieving doc: %s", mess.str());
    }
#endif
}

IPropertyTree * ElasticStackLogAccess::getTimestampTypeFormat(const char * indexpattern, const char * fieldname)
{
    if (!indexpattern || !*indexpattern)
        throw makeStringException(-1, "ElasticStackLogAccess::getTimestampTypeFormat: indexpattern must be provided");

    if (!fieldname || !*fieldname)
            throw makeStringException(-1, "ElasticStackLogAccess::getTimestampTypeFormat: fieldname must be provided");

    try
    {
        VStringBuffer indexsearch("%s/_mapping/field/created_ts?include_type_name=true&format=JSON", indexpattern);
        LOG(MCuserProgress,"ES Log Access: Fetching field type: %s", indexsearch.str());
        cpr::Response indexResponse = m_esClient.performRequest(Client::HTTPMethod::GET, indexsearch.str(), "");
        Owned<IPropertyTree> esStatus = createPTreeFromJSONString(indexResponse.text.c_str());
        LOG(MCuserProgress,"ES Log Access: Field type mapping: '%s'", indexResponse.text.c_str());

        return esStatus.getClear();
    }
    catch (ConnectionException & ce)//std::runtime_error
    {
        LOG(MCuserError, "ES Log Access: Encountered error searching available indexes: '%s'", ce.what());
    }
    catch (...)
    {
        LOG(MCuserError, "ES Log Access: Encountered error searching available indexes");
    }
    return nullptr;
}

IPropertyTree * ElasticStackLogAccess::getIndexSearchStatus(const char * indexpattern)
{
    if (!indexpattern || !*indexpattern)
        throw makeStringException(-1, "ElasticStackLogAccess::getIndexSearchStatus: indexpattern must be provided");

    try
    {
        VStringBuffer indexsearch("_cat/indices/%s?format=JSON", indexpattern);
        LOG(MCuserProgress,"ES Log Access: Seeking list of available indexes: %s", indexsearch.str());
        cpr::Response indexResponse = m_esClient.performRequest(Client::HTTPMethod::GET, indexsearch.str(), "");
        Owned<IPropertyTree> esStatus = createPTreeFromJSONString(indexResponse.text.c_str());
        LOG(MCuserProgress,"ES Log Access: List of available indexes: '%s'", indexResponse.text.c_str());

        return esStatus.getClear();
    }
    catch (ConnectionException & ce)//std::runtime_error
    {
        LOG(MCuserError, "ES Log Access: Encountered error searching available indexes: '%s'", ce.what());
    }
    catch (...)
    {
        LOG(MCuserError, "ES Log Access: Encountered error searching available indexes");
    }
    return nullptr;
}

IPropertyTree * ElasticStackLogAccess::getESStatus()
{
    try
    {
        LOG(MCuserProgress,"ES Log Access: Seeking target cluster health...");
        cpr::Response indexResponse = m_esClient.performRequest(Client::HTTPMethod::GET, "_cluster/health", "");
        Owned<IPropertyTree> esStatus = createPTreeFromJSONString(indexResponse.text.c_str());
        LOG(MCuserProgress,"ES Log Access: Cluster health response: '%s'", indexResponse.text.c_str());

        return esStatus.getClear();
    }
    catch (ConnectionException & ce)//std::runtime_error
    {
        LOG(MCuserError, "ES Log Access: Encountered error while seeking target cluster health: '%s'", ce.what());
    }
    catch (...)
    {
        LOG(MCuserError, "ES Log Access: Encountered unknow error while seeking target cluster health");
    }

    return nullptr;
}
/*
 * Transform ES query response to back-end agnostic response
 *
 */
void processESJsonResp(const cpr::Response & retrievedDocument, StringBuffer & returnbuf, LogAccessLogFormat format)
{
    if (retrievedDocument.status_code == 200)
    {
        DBGLOG("Retrieved ES JSON DOC: %s", retrievedDocument.text.c_str());

        IPropertyTree * tree = createPTreeFromJSONString(retrievedDocument.text.c_str());
        if (!tree)
            throw makeStringExceptionV(-1, "%s: Could not parse ElasticSearch query response", COMPONENT_NAME);

        if (tree->hasProp("timed_out") && tree->getPropBool("timed_out") == true)
            LOG(MCuserProgress,"ES Log Access: timeout reported");
        if (tree->hasProp("_shards/failed") && tree->getPropInt("_shards/failed") > 0)
            LOG(MCuserProgress,"ES Log Access: failed _shards reported");

        DBGLOG("ES Log Access: hit count: '%s'", tree->queryProp("hits/total/value"));

        Owned<IPropertyTreeIterator> iter = tree->getElements("hits/hits/fields");
        switch (format)
        {
            case LOGACCESS_LOGFORMAT_xml:
            {
                returnbuf.set("<lines>");
                StringBuffer hitchildxml;

                ForEach(*iter)
                {
                    IPropertyTree & cur = iter->query();
                    toXML(&cur,hitchildxml.clear());
                    returnbuf.appendf("<line>%s</line>", hitchildxml.str());
                }
                returnbuf.append("</lines>");
                break;
            }
            case LOGACCESS_LOGFORMAT_json:
            {
                returnbuf.set("{\"lines\": [");
                StringBuffer hitchildjson;
                bool first = true;
                ForEach(*iter)
                {
                    IPropertyTree & cur = iter->query();
                    toJSON(&cur,hitchildjson.clear());
                    if (!first)
                        returnbuf.append(", ");

                    first = false;
                    returnbuf.appendf("{\"fields\": [ %s ]}", hitchildjson.str());
                }
                returnbuf.append("]}");
                break;
            }
            case LOGACCESS_LOGFORMAT_csv:
            {
                ForEach(*iter)
                {
                    IPropertyTree & cur = iter->query();
                    bool first = true;
                    Owned<IPropertyTreeIterator> fieldelementsitr = cur.getElements("*");
                    ForEach(*fieldelementsitr)
                    {
                        if (!first)
                            returnbuf.append(", ");
                        else
                            first = false;

                        returnbuf.append(fieldelementsitr->query().queryProp(".")); // commas in data should be escaped
                    }
                    returnbuf.append("\n");
                }
                break;
            }
            default:
                break;
        }
    }
    else
    {
        throw makeStringExceptionV(-1, "ElasticSearch request failed: %s", retrievedDocument.status_line.c_str());
    }
}

void esTimestampQueryRangeString(std::string & range, const char * timestampfield, std::time_t from, std::time_t to)
{
    if (timestampfield && *timestampfield)
    {
        //Elastic Search Date formats can be customized, but if no format is specified then it uses the default:
        //"strict_date_optional_time||epoch_millis"
        // "%Y-%m-%d"'T'"%H:%M:%S"

        //We'll report the timestamps as epoch_millis
        range = "\"range\": { \"";
            range += timestampfield;
                range += "\": {";
                    range += "\"gte\": \"";
                    range += std::to_string(from*1000);
                    range += "\"";

                    if (to != -1) //aka 'to' has been initialized
                    {
                        range += ",\"lte\": \"";
                        range += std::to_string(to*1000);
                        range += "\"";
                    }
        range += "} }";
    }
    else
        throw makeStringException(-1, "ES Log Access: TimeStamp Field must be provided");
}

/*
 * Constructs ElasticSearch match clause
 * Use for exact term matches such as a price, a product ID, or a username.
 */
void esTermQueryString(std::string & search, const char *searchval, const char *searchfield)
{
    //https://www.elastic.co/guide/en/elasticsearch/reference/current/query-dsl-term-query.html
    //You can use the term query to find documents based on a precise value such as a price, a product ID, or a username.

    //Avoid using the term query for text fields.
    //By default, Elasticsearch changes the values of text fields as part of analysis. This can make finding exact matches for text field values difficult.
    if (searchval && *searchval && searchfield && *searchfield)
    {
        search += "\"term\": { \"";
        search += searchfield;
        search += "\" : { \"value\": \"";
        search += searchval;
        search += "\" } }";
    }
}

/*
 * Constructs ElasticSearch match clause
 * Use for full-text search
 */
void esMatchQueryString(std::string & search, const char *searchval, const char *searchfield)
{
    //https://www.elastic.co/guide/en/elasticsearch/reference/current/query-dsl-match-query.html
    //Returns documents that match a provided text, number, date or boolean value. The provided text is analyzed before matching.
    //The match query is the standard query for performing a full-text search, including options for fuzzy matching.
    if (searchval && *searchval && searchfield && *searchfield)
    {
        search += "\"match\": { \"";
        search += searchfield;
        search += "\" : \"";
        search += searchval;
        search += "\" }";
    }
    else
        throw makeStringException(-1, "Could not create ES match query string: Either search value or search field is empty");
}

/*
 * Construct Elasticsearch query directives string
 */
void esSearchMetaData(std::string & search, StringArray & selectcols, int size = 10, int from = 0)
{
        //Query parameters:
        //https://www.elastic.co/guide/en/elasticsearch/reference/6.8/search-request-body.html

        //_source: https://www.elastic.co/guide/en/elasticsearch/reference/6.8/search-request-source-filtering.html
        search += "\"_source\": false, \"fields\": [" ;

        if (selectcols.length() > 0)
        {
            StringBuffer sourcecols;
            ForEachItemIn(idx, selectcols)
            {
                sourcecols.appendf("\"%s\"", selectcols.item(idx));
                if (idx < selectcols.length() -1)
                    sourcecols.append(",");
            }

            if (!sourcecols.isEmpty())
                search += sourcecols.str() ;
            else
                search += "*";
                //search += "!*.keyword"; //all fields ignoring the .keyword postfixed fields
        }
        else
            //search += "!*.keyword"; //all fields ignoring the .keyword postfixed fields
            search += "*";

        search += "],";
        search += "\"from\": ";
        search += std::to_string(from);
        search += ", \"size\": ";
        search += std::to_string(size);
        search += ", ";
}

/*
 * Construct ES query string, execute query
 */
cpr::Response ElasticStackLogAccess::performESQuery(LogAccessConditions & options)
{
    try
    {
        StringBuffer queryValue;
        std::string queryField = m_globalSearchColName.str();
        std::string queryIndex = m_globalIndexSearchPattern.str();

        bool fullTextSearch = true;

        options.filter->toString(queryValue);
        switch (options.filter->filterType())
        {
        case LOGACCESS_FILTER_jobid:
        {
            if (!m_workunitSearchColName.isEmpty())
            {
                queryField  = m_workunitSearchColName.str();
                fullTextSearch = false; //found dedicated components column
            }

            if (!m_workunitIndexSearchPattern.isEmpty())
            {
                queryIndex = m_workunitIndexSearchPattern.str();
            }

            DBGLOG("%s: Searching log entries by jobid: '%s'...", COMPONENT_NAME, queryValue.str() );
            break;
        }
        case LOGACCESS_FILTER_class:
        {
            if (!m_classSearchColName.isEmpty())
            {
                queryField  = m_classSearchColName.str();
                fullTextSearch = false; //found dedicated components column
            }

            if (!m_classIndexSearchPattern.isEmpty())
            {
                queryIndex = m_classIndexSearchPattern.str();
            }

            DBGLOG("%s: Searching log entries by class: '%s'...", COMPONENT_NAME, queryValue.str() );
            break;
        }
        case LOGACCESS_FILTER_audience:
        {
            if (!m_audienceSearchColName.isEmpty())
            {
                queryField  = m_audienceSearchColName.str();
                fullTextSearch = false; //found dedicated components column
            }

            if (!m_audienceIndexSearchPattern.isEmpty())
            {
                queryIndex = m_audienceIndexSearchPattern.str();
            }

            DBGLOG("%s: Searching log entries by target audience: '%s'...", COMPONENT_NAME, queryValue.str() );
            break;
        }
        case LOGACCESS_FILTER_component:
        {
            if (!m_componentsSearchColName.isEmpty())
            {
                queryField = m_componentsSearchColName.str();
                fullTextSearch = false; //found dedicated components column
            }

            if (!m_componentsIndexSearchPattern.isEmpty())
            {
                queryIndex = m_componentsIndexSearchPattern.str();
            }

            DBGLOG("%s: Searching '%s' component log entries...", COMPONENT_NAME, queryValue.str() );
            break;
        }
        case LOGACCESS_FILTER_or:
            throw makeStringExceptionV(-1, "%s: Compound query criteria not currently supported: '%s'", COMPONENT_NAME, queryValue.str());
            //"query":{"bool":{"must":[{"match":{"kubernetes.container.name.keyword":{"query":"eclwatch","operator":"or"}}},{"match":{"container.image.name.keyword":"hpccsystems\\core"}}]} }
        case LOGACCESS_FILTER_and:
            throw makeStringExceptionV(-1, "%s: Compound query criteria not currently supported: '%s'", COMPONENT_NAME, queryValue.str());
            //"query":{"bool":{"must":[{"match":{"kubernetes.container.name.keyword":{"query":"eclwatch","operator":"and"}}},{"match":{"created_ts":"2021-08-25T20:23:04.923Z"}}]} }
        default:
            throw makeStringExceptionV(-1, "%s: Unknown query criteria type encountered: '%s'", COMPONENT_NAME, queryValue.str());
        }

        std::string fullrequest = "{";
        esSearchMetaData(fullrequest, options.logFieldNames, options.limit == -1 ? DEFAULT_ES_DOC_LIMIT : options.limit, options.offset == -1 ? DEFAULT_ES_DOC_LIMIT : options.offset);

            fullrequest += "\"query\": { \"bool\": { \"must\": { ";

            std::string criteria;
            if (fullTextSearch) //are we performing a query on a blob, or exact term match?
                esMatchQueryString(criteria, queryValue.str(), queryField.c_str());
            else
                esTermQueryString(criteria, queryValue.str(), queryField.c_str());
            fullrequest += criteria.c_str();
        fullrequest += "}"; //end must

            std::string filter = ", \"filter\": {";
                std::string range;

                //Bail out earlier?
                if (options.timeRange.startt.isNull())
                    throw makeStringExceptionV(-1, "%s: start time must be provided!", COMPONENT_NAME);

                esTimestampQueryRangeString(range, m_globalIndexTimestampField.str(), options.timeRange.startt.getSimple(),options.timeRange.endt.isNull() ? -1 : options.timeRange.endt.getSimple());

                filter += range.c_str();
            filter += "}"; //end filter

        fullrequest += filter.c_str();
        fullrequest += "}}}"; //end bool and query

        DBGLOG("%s: Search string '%s'", COMPONENT_NAME, fullrequest.c_str());

        return m_esClient.search(queryIndex.c_str(), DEFAULT_ES_DOC_TYPE, fullrequest);
    }
    catch (std::runtime_error &e)
    {
        const char * wha = e.what();
        throw makeStringExceptionV(-1, "%s: fetchLog: Error searching doc: %s", COMPONENT_NAME, wha);
    }
    catch (IException * e)
    {
        StringBuffer mess;
        e->errorMessage(mess);
        e->Release();
        throw makeStringExceptionV(-1, "%s: fetchLog: Error searching doc: %s", COMPONENT_NAME, mess.str());
    }
}

bool ElasticStackLogAccess::fetchLog(LogAccessConditions & options, StringBuffer & returnbuf, LogAccessLogFormat format)
{
    cpr::Response esresp = performESQuery(options);
    processESJsonResp(esresp, returnbuf, format);

    return true;
}

extern "C" IRemoteLogAccess * createInstance(IPropertyTree & logAccessPluginConfig)
{
    //constructing ES Connection string(s) here b/c ES Client explicit ctr requires conn string array

    const char * protocol = logAccessPluginConfig.queryProp("connection/@protocol");
    const char * host = logAccessPluginConfig.queryProp("connection/@host");
    const char * port = logAccessPluginConfig.queryProp("connection/@port");

    std::string elasticSearchConnString;
    elasticSearchConnString = !protocol || !*protocol ? "http" : protocol;
    elasticSearchConnString.append("://");
    elasticSearchConnString.append((!host || !*host) ? "localhost" : host);
    elasticSearchConnString.append(":").append((!port || !*port) ? ElasticStackLogAccess::DEFAULT_ES_PORT : port);
    elasticSearchConnString.append("/"); // required!

    return new ElasticStackLogAccess({elasticSearchConnString}, logAccessPluginConfig);
}
