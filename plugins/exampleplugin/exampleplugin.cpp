/*##############################################################################

    HPCC SYSTEMS software Copyright (C) 2015 HPCC Systems®.

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
#include "jstring.hpp"
#include "example-plugin.hpp"

#define EXAMPLE-PLUGIN_VERSION "example-plugin plugin 1.0.0"
ECL_REDIS_API bool getECLPluginDefinition(ECLPluginDefinitionBlock *pb)
{
    if (pb->size != sizeof(ECLPluginDefinitionBlock))
        return false;

    pb->magicVersion = PLUGIN_VERSION;
    pb->version = EXMAPLE-PLUGIN_VERSION;
    pb->moduleName = "lib_redis";
    pb->ECL = NULL;
    pb->flags = PLUGIN_IMPLICIT_MODULE;
    pb->description = "ECL plugin library for BLAH BLAH BLAH\n";
    return true;
}

namespace ExamplePlugin {


//--------------------------------------------------------------------------------
//                           ECL SERVICE ENTRYPOINTS
//--------------------------------------------------------------------------------
ECL_EXAMPLE-PLUGIN_API unsigned ECL_EXMAPLE-PLUGIN_CALL func1(ICodeContext * ctx, const char * param1, const char * param2, unsigned param3)
{
    return param3 + 1;
}

ECL_EXAMPLE-PLUGIN_API void ECL_EXAMPLE-PLUGIN_CALL func2 (ICodeContext * _ctx, , size32_t & returnLength, char * & returnValue, const char * param1, const char * param2, size32_t param3ValueLength, const char * param3Value);
{
    StringBuffer buffer(param3Value);
    buffer.toLowerCase();
    returnLength = buffer.length();
    returnValue = buffer.detach();
    return;
}

}//close namespace
