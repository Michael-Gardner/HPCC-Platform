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
//#include <cstdio>
#include "platform.h"

#include <string>
#include <vector>
#include <iostream>

/* undef verify definitions to avoid collision in cpr submodule */
#ifdef verify 
    //#pragma message("UNDEFINING 'verify' - Will be redefined by cpr" )
    #undef verify
#endif

#include <cpr/response.h>
#include <elasticlient/client.h>



ElasticStackLogAccess::ElasticStackLogAccess(IPropertyTree & logAccessPluginConfig)
{

}

ElasticStackLogAccess::ElasticStackLogAccess(const char * elasticSearchConnString, const char * defaultIndex)
{

}

ElasticStackLogAccess::ElasticStackLogAccess(const char * protocol, const char * host, const char * port, const char * defaultIndex)
{

}

void ElasticStackLogAccess::init(const char * protocol, const char * host, const char * port, const char * defaultIndex, const char * defDocType)
{
    m_elasticSearchConnString.set((!protocol || !*protocol) ? "http" : protocol).append("://");
    m_elasticSearchConnString.append((!host || !*host) ? "localhost" : host);
    m_elasticSearchConnString.set((!port || !*port) ? "9200" : port);

//    if (!defaultIndex || !*defaultIndex)
//    	Log("ElasticStackLogAccess: no default index defined");

    m_defaultIndex = defaultIndex;
	m_defaultDocType = defDocType;
//	m_docId; ??

// Prepare Client for nodes of one Elasticsearch cluster
    elasticlient::Client client({"http://localhost:9200/"}); // last / is mandatory

    // Or if you'd like to use proxy
    //elasticlient::Client client({"http://elastic1.host:9200/"}, 6000,
    //        {{"http", "http://proxy.host:8080"},{"https", "https://proxy.host:8080"}});

    std::string document {"{\"message\": \"00000041 USR 2021-05-05 17:55:03.316 975333 975333 \"CSmartSocketFactory::CSmartSocketFactory(192.168.1.140:9876)\"\"}"};

    // Index the document, index "testindex" must be created before
    cpr::Response indexResponse = client.index("filebeat-xyz", "docType", "docId", document);
    // 200
    std::cout << indexResponse.status_code << std::endl;
    // application/json; charset=UTF-8
    std::cout << indexResponse.header["content-type"] << std::endl;
    // Elasticsearch response (JSON text string)
    std::cout << indexResponse.text << std::endl;

    // Retrieve the document
    cpr::Response retrievedDocument = client.get("testindex", "docType", "docId");
    // 200
    std::cout << retrievedDocument.status_code << std::endl;
    // application/json; charset=UTF-8
    std::cout << retrievedDocument.header["content-type"] << std::endl;
    // Elasticsearch response (JSON text string) where key "_source" contain:
    // {"message": "Hello world!"}
    std::cout << retrievedDocument.text << std::endl;

    // Remove the document
    cpr::Response removedDocument = client.remove("testindex", "docType", "docId");
    // 200
    std::cout << removedDocument.status_code << std::endl;
    // application/json; charset=UTF-8
    std::cout << removedDocument.header["content-type"] << std::endl;
}

bool ElasticStackLogAccess::fetchLog(LogAccessConditions options, StringBuffer & returnbuf)
{
	switch (options.filter.filterType)
	{
		case ALF_WorkUnit:
			returnbuf.set("Searching for WU Logs ").append(options.filter.workUnit.str());
			return true;
			break;
		default:
			break;
	}

	return false;
}

bool ElasticStackLogAccess::fetchWULog(StringBuffer & returnbuf, const char * wu, LogAccessRange range, StringArray * cols)
{
	LogAccessConditions logFetchoptions;
	logFetchoptions.range = range;
	logFetchoptions.filter.filterType = ALF_WorkUnit;
	logFetchoptions.filter.workUnit.set(wu);

	fetchLog(logFetchoptions, returnbuf);
	return true;
}

bool ElasticStackLogAccess::fetchComponentLog(const char * component, LogAccessRange range, StringBuffer & returnbuf)
{
	return false;
}
bool ElasticStackLogAccess::fetchLogByAudience(const char * audience, LogAccessRange range, StringBuffer & returnbuf)
{
	return false;
}

//extern "C" ELASTICSTACKLOGACCESS_API ILogAccess * createInstance(IPropertyTree & logAccessPluginConfig)
extern "C" ILogAccess * createInstance(IPropertyTree & logAccessPluginConfig)
{
	return new ElasticStackLogAccess(logAccessPluginConfig);
}
