/*##############################################################################

    HPCC SYSTEMS software Copyright (C) 2025 HPCC Systems®.

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

#include "platform.h"
#include "azureapiutils.hpp"
#include "jlib.hpp"
#include "jexcept.hpp"
#include "jstring.hpp"
#include "jfile.hpp"
#include "jlog.hpp"
#include <cstdlib>

// Common utility functions shared by both blob and file implementations
//---------------------------------------------------------------------------------------------------------------------

static inline bool isEnvSet(const char *value)
{
    return !isEmptyString(value);
}

static void appendReason(StringBuffer *reason, const char *text)
{
    if (!reason)
        return;

    if (!reason->isEmpty())
        reason->append("; ");
    reason->append(text);
}

static bool tokenFileAvailable(const char *tokenFile, StringBuffer *reason)
{
    if (!isEnvSet(tokenFile))
        return false;

    if (checkFileExists(tokenFile))
        return true;

    if (reason)
    {
        VStringBuffer msg("managed identity token file '%s' is not accessible", tokenFile);
        appendReason(reason, msg.str());
    }
    return false;
}

static bool queryManagedIdentityAvailability(StringBuffer *reason)
{
    const char *msiEndpoint = std::getenv("MSI_ENDPOINT");
    const char *identityEndpoint = std::getenv("IDENTITY_ENDPOINT");
    const char *identityHeader = std::getenv("IDENTITY_HEADER");
    const char *imdsEndpoint = std::getenv("IMDS_ENDPOINT");

    if (isEnvSet(msiEndpoint) || isEnvSet(identityEndpoint) || isEnvSet(identityHeader) || isEnvSet(imdsEndpoint))
        return true;

    const char *clientId = std::getenv("AZURE_CLIENT_ID");
    const char *tokenFile = std::getenv("AZURE_FEDERATED_TOKEN_FILE");

    if (isEnvSet(clientId) && tokenFileAvailable(tokenFile, reason))
        return true;

    if (reason)
    {
        if (!isEnvSet(clientId) && !isEnvSet(tokenFile))
            appendReason(reason, "managed identity environment variables were not detected");
        else
        {
            if (!isEnvSet(clientId))
                appendReason(reason, "AZURE_CLIENT_ID is not set");
            if (!isEnvSet(tokenFile))
                appendReason(reason, "AZURE_FEDERATED_TOKEN_FILE is not set");
        }
    }

    return false;
}

bool areManagedIdentitiesEnabled()
{
    return queryManagedIdentityAvailability(nullptr);
}

bool areManagedIdentitiesEnabled(StringBuffer &reason)
{
    reason.clear();
    return queryManagedIdentityAvailability(&reason);
}

bool isBase64Char(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || (c == '+') || (c == '/') || (c == '=');
}

void handleRequestBackoff(const char * message, unsigned attempt, unsigned maxRetries)
{
    OWARNLOG("%s", message);

    if (attempt >= maxRetries)
        throw makeStringException(1234, message);

    // Exponential backoff with jitter
    unsigned backoffMs = (1U << attempt) * 100 + (rand() % 100);
    Sleep(backoffMs);
}

void handleRequestException(const Azure::Core::RequestFailedException& e, const char * op, unsigned attempt, unsigned maxRetries, const char * filename, offset_t pos, offset_t len)
{
    VStringBuffer msg("%s failed (attempt %u/%u) for file %s at offset %llu, len %llu: %s (%d)",
                      op, attempt, maxRetries, filename, pos, len, e.ReasonPhrase.c_str(), static_cast<int>(e.StatusCode));

    handleRequestBackoff(msg, attempt, maxRetries);
}

void handleRequestException(const std::exception& e, const char * op, unsigned attempt, unsigned maxRetries, const char * filename, offset_t pos, offset_t len)
{
    VStringBuffer msg("%s failed (attempt %u/%u) for file %s at offset %llu, len %llu: %s",
                      op, attempt, maxRetries, filename, pos, len, e.what());

    handleRequestBackoff(msg, attempt, maxRetries);
}
