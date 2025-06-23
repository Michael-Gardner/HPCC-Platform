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
#include "eclrtl.hpp"
#include "jexcept.hpp"
#include "phonenumber.hpp"
#include <string>
#define PHONENUMBER_VERSION "phonenumber plugin 1.0.0"

ECL_PHONENUMBER_API bool getECLPluginDefinition(ECLPluginDefinitionBlock *pb)
{
    /*  Warning:    This function may be called without the plugin being loaded fully.
     *              It should not make any library calls or assume that dependent modules
     *              have been loaded or that it has been initialised.
     *
     *              Specifically:  "The system does not call DllMain for process and thread
     *              initialization and termination.  Also, the system does not load
     *              additional executable modules that are referenced by the specified module."
     */

    if (pb->size != sizeof(ECLPluginDefinitionBlock))
        return false;

    pb->magicVersion = PLUGIN_VERSION;
    pb->version = PHONENUMBER_VERSION;
    pb->moduleName = "lib_phonenumber";
    pb->ECL = NULL;
    pb->flags = PLUGIN_IMPLICIT_MODULE;
    pb->description = "ECL plugin library for google libphonenumber";
    return true;
}

// ForEach & verify macro is defined in eclhelper.hpp and causes a conflict with the PhoneNumberUtil class
#ifdef ForEach
#undef ForEach
#endif
#ifdef verify
#undef verify
#endif
#include <phonenumbers/phonenumberutil.h>
using namespace i18n::phonenumbers;
#include "jstring.hpp"

namespace phonenumber
{

ECL_PHONENUMBER_API bool ECL_PHONENUMBER_CALL isValidNumber(ICodeContext *_ctx, string &number)
{
    const PhoneNumberUtil* phoneUtil = PhoneNumberUtil::GetInstance();
    PhoneNumber phoneNumber;
    try
    {
        phoneUtil->Parse(number, "US", &phoneNumber);
        return phoneUtil->IsValidNumber(phoneNumber);
    }
    catch (...)
    {
        return false;
    }
}

ECL_PHONENUMBER_API string ECL_PHONENUMBER_CALL phonenumberType(ICodeContext *_ctx, string &number)
{
    const PhoneNumberUtil* phoneUtil = PhoneNumberUtil::GetInstance();
    PhoneNumber phoneNumber;
    try
    {
        phoneUtil->Parse(number, "US", &phoneNumber);
        PhoneNumberUtil::PhoneNumberType type = phoneUtil->GetNumberType(phoneNumber);
        switch (type)
        {
        case PhoneNumberUtil::PhoneNumberType::FIXED_LINE:
            return "FIXED_LINE";
        case PhoneNumberUtil::PhoneNumberType::MOBILE:
            return "MOBILE";
        case PhoneNumberUtil::PhoneNumberType::FIXED_LINE_OR_MOBILE:
            return "FIXED_LINE_OR_MOBILE";
        case PhoneNumberUtil::PhoneNumberType::TOLL_FREE:
            return "TOLL_FREE";
        case PhoneNumberUtil::PhoneNumberType::PREMIUM_RATE:
            return "PREMIUM_RATE";
        case PhoneNumberUtil::PhoneNumberType::SHARED_COST:
            return "SHARED_COST";
        case PhoneNumberUtil::PhoneNumberType::VOIP:
            return "VOIP";
        case PhoneNumberUtil::PhoneNumberType::PERSONAL_NUMBER:
            return "PERSONAL_NUMBER";
        case PhoneNumberUtil::PhoneNumberType::PAGER:
            return "PAGER";
        case PhoneNumberUtil::PhoneNumberType::UAN:
            return "UAN";
        case PhoneNumberUtil::PhoneNumberType::VOICEMAIL:
            return "VOICEMAIL";
        case PhoneNumberUtil::PhoneNumberType::UNKNOWN:
            return "UNKNOWN";
        default:
            return "INVALID";
        }
    }
    catch (...)
    {
        return "INVALID";
    }
}

ECL_PHONENUMBER_API string ECL_PHONENUMBER_CALL regionCode(ICodeContext *_ctx, string &number)
{
    const PhoneNumberUtil* phoneUtil = PhoneNumberUtil::GetInstance();
    PhoneNumber phoneNumber;
    string regionCode;
    try
    {
        phoneUtil->Parse(number, "US", &phoneNumber);
        phoneUtil->GetRegionCodeForNumber(phoneNumber, &regionCode);
    }
    catch (...)
    {
        return "";
    }
    return regionCode;
}

ECL_PHONENUMBER_API unsigned ECL_PHONENUMBER_CALL countryCode(ICodeContext *_ctx, string &number)
{
    const PhoneNumberUtil* phoneUtil = PhoneNumberUtil::GetInstance();
    PhoneNumber phoneNumber;
    try
    {
        phoneUtil->Parse(number, "US", &phoneNumber);
        return phoneNumber.country_code();
    }
    catch (...)
    {
        return 0;
    }
}

} // namespace phonenumber