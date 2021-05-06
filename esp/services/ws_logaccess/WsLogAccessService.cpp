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
        resp.setType(m_remoteLogAccessor->getRemoteLogAccessType());
        resp.setConnection(m_remoteLogAccessor->fetchConnectionStr());
    }
    else
        success = false;

    return success;
}

void Cws_logaccessEx::init(IPropertyTree *cfg, const char *process, const char *service)
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
        CLogAccessType searchbycategory = req.getLogCategory();
        const char * searchbyvalue = req.getValue();
        if (searchbycategory != CLogAccessType_All && (!searchbyvalue || !*searchbyvalue))
        {
            message = "WsLogAccess::onGetLogs: Must provide log category";
            success = false;
        }
        else
        {
            ILogAccessFilter * filter = nullptr;
            switch (searchbycategory)
            {
                case CLogAccessType_All:
                    filter = getWildCardLogAccessFilter();
                    break;
                case CLogAccessType_ByWUID:
                    filter = getWUIDLogAccessFilter(searchbyvalue);
                    break;
                case CLogAccessType_ByComponent:
                    filter = getComponentLogAccessFilter(searchbyvalue);
                    break;
                case CLogAccessType_ByLogType:
                {
                    LogMsgClass logtype = LogMsgClass(LogMsgClassFromAbbrev(searchbyvalue));
                    if (logtype == 0 )
                        throw makeStringExceptionV(-1, "Invalid Log Type 3-letter code encountered: '%s' - Available values: 'DIS,ERR,WRN,INF,PRO,MET'", searchbyvalue);
                    filter = getClassLogAccessFilter(logtype);
                    break;
                }
                case CLogAccessType_ByTargetAudience:
                {
                    MessageAudience targetaud = MessageAudience(LogMsgAudFromAbbrev(searchbyvalue));
                    if (targetaud == 0 || targetaud == MSGAUD_all)
                        throw makeStringExceptionV(-1, "Invalid Target Audience 3-letter code encountered: '%s' - Available values: 'OPR,USR,PRO,ADT'", searchbyvalue);

                    filter = getAudienceLogAccessFilter(targetaud);
                    break;
                }
                case LogAccessType_Undefined:
                    throw makeStringException(-1, "Invalid remote log access request type");
                default:
                    break;
            }

            struct LogAccessTimeRange range = requestedRangeToLARange(req.getRange());
            StringArray & cols = req.getColumns();

            int limit = req.getLimit();
            if (limit < 1)
                throw makeStringExceptionV(-1, "WsLogAccess: Encountered invalid limit value: '%d'", limit);

            long offset = req.getOffset();
            if (offset < 0)
                throw makeStringExceptionV(-1, "WsLogAccess: Encountered invalid offset value: '%ld'", offset);

            LogAccessConditions logFetchoptions;

            logFetchoptions.timeRange = range;
            logFetchoptions.filter = filter;
            logFetchoptions.copyLogFieldNames(&cols);
            logFetchoptions.limit = limit;
            logFetchoptions.offset = offset;

            StringBuffer logcontent;
            m_remoteLogAccessor->fetchLog(logFetchoptions, logcontent, LogAccessFormatFromName(req.getFormat()));

            resp.setRemoteLog(logcontent.str());
        }
    }
    else
    {
        success = false;
        message = "Remote Log Access plug-in not available!";
    }

    resp.setSuccess(success);
    if (message && *message)
        resp.setMessage(message);

    return success;
}
