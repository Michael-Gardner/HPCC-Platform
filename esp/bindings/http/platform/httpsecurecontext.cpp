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

#include "httpsecurecontext.hpp"

class CHttpSecureContext : extends CEspBaseSecureContext
{
private:
    CHttpRequest* m_request;

protected:
    IEspContext* queryContext() const
    {
        return (m_request ? m_request->queryContext() : NULL);
    }

public:
    CHttpSecureContext(CHttpRequest* request)
        : m_request(request)
    {
    }

    virtual ~CHttpSecureContext()
    {
    }

    virtual const char* getProtocol() const override
    {
        return "http";
    }

    virtual bool getProp(int type, const char* name, StringBuffer& value) override
    {
        bool result = false;

        switch (type)
        {
        case HTTP_PROPERTY_TYPE_COOKIE:
            result = getCookie(name, value);
            break;
        default:
            break;
        }

        return result;
    }

private:
    bool getCookie(const char* name, StringBuffer& value)
    {
        bool result = false;

        if (name && *name)
        {
            CEspCookie* cookie = m_request->queryCookie(name);

            if (cookie)
            {
                value.set(cookie->getValue());
                result = true;
            }
        }

        return result;
    }
};

IEspSecureContext* createHttpSecureContext(CHttpRequest* request)
{
    return new CHttpSecureContext(request);
}
