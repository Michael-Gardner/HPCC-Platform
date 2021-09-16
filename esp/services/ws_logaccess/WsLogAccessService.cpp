#include <ws_logaccess/WsLogAccessService.hpp>

#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>


Cws_logaccessEx::Cws_logaccessEx()
{
}

Cws_logaccessEx::~Cws_logaccessEx()
{
}

bool Cws_logaccessEx::onGetLogAccessMeta(IEspContext &context, IEspGetLogAccessMetaRequest &req, IEspGetLogAccessMetaResponse &resp)
{
    bool success = true;
    if (m_remoteLogAccessor)
    {
        resp.setRemoteLogManagerType(m_remoteLogAccessor->getRemoteLogAccessType());
        resp.setRemoteLogManagerConnectionString(m_remoteLogAccessor->fetchConnectionStr());
    }
    else
        success = false;

    return success;
}

void Cws_logaccessEx::init(const IPropertyTree *cfg, const char *process, const char *service)
{
    LOG(MCdebugProgress,"WsLogAccessService loading remote log access plug-in...");

    try
    {
        m_remoteLogAccessor.set(RemoteLogAccessLoader::queryRemoteLogAccessor<IRemoteLogAccess>());

        if (m_remoteLogAccessor == nullptr || stricmp(m_remoteLogAccessor->getRemoteLogAccessType(), "elasticstack")!=0)
            LOG(MCerror,"WsLogAccessService could not load remote log access plugin!");

        m_logMap.set(m_remoteLogAccessor->fetchLogMap());
    }
    catch (IException * e)
    {
        StringBuffer msg;
        e->errorMessage(msg);
        LOG(MCoperatorWarning,"WsLogAccessService could not load remote log access plug-in: %s", msg.str());
        e->Release();
    }
}

LogAccessTimeRange requestedRangeToLARange(IConstTimeRange & reqrange)
{
    struct LogAccessTimeRange range;

    range.setStart(reqrange.getStartDate());
    range.setEnd(reqrange.getEndDate());

    return range;
}

bool Cws_logaccessEx::onGetLogs(IEspContext &context, IEspGetLogsRequest &req, IEspGetLogsResponse & resp)
{
    const char * message = "success";
    bool success = true;

    if (m_remoteLogAccessor)
    {
        CLogAccessType searchByCategory = req.getLogCategory();
        const char * searchByValue = req.getSearchByValue();
        if (searchByCategory != CLogAccessType_All && isEmptyString(searchByValue))
        {
            message = "WsLogAccess::onGetLogs: Must provide log category";
            success = false;
        }
        else
        {
            LogAccessConditions logFetchOptions;
            switch (searchByCategory)
            {
                case CLogAccessType_All:
                    logFetchOptions.setFilter(getWildCardLogAccessFilter());
                    break;
                case CLogAccessType_ByWUID:
                    logFetchOptions.setFilter(getJobIDLogAccessFilter(searchByValue));
                    break;
                case CLogAccessType_ByComponent:
                    logFetchOptions.setFilter(getComponentLogAccessFilter(searchByValue));
                    break;
                case CLogAccessType_ByLogType:
                {
                    LogMsgClass logType = LogMsgClass(LogMsgClassFromAbbrev(searchByValue));
                    if (logType == 0 )
                        throw makeStringExceptionV(-1, "Invalid Log Type 3-letter code encountered: '%s' - Available values: 'DIS,ERR,WRN,INF,PRO,MET'", searchByValue);
                    logFetchOptions.setFilter(getClassLogAccessFilter(logType));
                    break;
                }
                case CLogAccessType_ByTargetAudience:
                {
                    MessageAudience targetAud = MessageAudience(LogMsgAudFromAbbrev(searchByValue));
                    if (targetAud == 0 || targetAud == MSGAUD_all)
                        throw makeStringExceptionV(-1, "Invalid Target Audience 3-letter code encountered: '%s' - Available values: 'OPR,USR,PRO,ADT'", searchByValue);

                    logFetchOptions.setFilter(getAudienceLogAccessFilter(targetAud));
                    break;
                }
                case LogAccessType_Undefined:
                    throw makeStringException(-1, "Invalid remote log access request type");
                default:
                    break;
            }

            struct LogAccessTimeRange range = requestedRangeToLARange(req.getRange());
            StringArray & cols = req.getColumns();

            unsigned limit = req.getLogLineLimit();
            if (limit < 1)
                throw makeStringExceptionV(-1, "WsLogAccess: Encountered invalid LogLineLimit value: '%u'", limit);

            long offset = req.getLogLineOffset();
            if (offset < 0)
                throw makeStringExceptionV(-1, "WsLogAccess: Encountered invalid LogLineOffset value: '%ld'", offset);

            logFetchOptions.timeRange = range;
            logFetchOptions.copyLogFieldNames(&cols);
            logFetchOptions.limit = limit;
            logFetchOptions.offset = offset;

            StringBuffer logcontent;
            m_remoteLogAccessor->fetchLog(logFetchOptions, logcontent, logAccessFormatFromName(req.getFormat()));

            resp.setLogLines(logcontent.str());
        }
    }
    else
    {
        success = false;
        message = "Remote Log Access plug-in not available!";
    }

    resp.setSuccess(success);
    if (!isEmptyString(message))
        resp.setMessage(message);

    return success;
}
