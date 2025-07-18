/*##############################################################################

    HPCC SYSTEMS software Copyright (C) 2012 HPCC Systems®.

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


#include "jiface.hpp"
#include "jstats.h"
#include "jexcept.hpp"
#include "jiter.ipp"
#include "jlog.hpp"
#include "jregexp.hpp"
#include "jfile.hpp"
#include "jerror.hpp"
#include <math.h>
#include <sstream>
#include <iomanip>
#include <ctype.h>
#include <cmath>

#ifdef _WIN32
#include <sys/timeb.h>
#endif

static CriticalSection statsNameCs;
static StringBuffer statisticsComponentName;
static StatisticCreatorType statisticsComponentType = SCTunknown;
const static unsigned currentStatisticsVersion = 1;

StatisticCreatorType queryStatisticsComponentType()
{
    return statisticsComponentType;
}

const char * queryStatisticsComponentName()
{
    CriticalBlock c(statsNameCs);
    if (statisticsComponentName.length() == 0)
    {
        statisticsComponentName.append("unknown").append(GetCachedHostName());
        DBGLOG("getProcessUniqueName hasn't been configured correctly");
    }
    return statisticsComponentName.str();
}

void setStatisticsComponentName(StatisticCreatorType processType, const char * processName, bool appendIP)
{
    if (!processName)
        return;

    CriticalBlock c(statsNameCs);
    statisticsComponentType = processType;
    statisticsComponentName.clear().append(processName);
    if (appendIP)
        statisticsComponentName.append("@").append(GetCachedHostName());  // should I use _ instead?
}

//--------------------------------------------------------------------------------------------------------------------

// Textual forms of the different enumerations, first items are for none and all.
static constexpr const char * const measureNames[] = { "", "all", "ns", "ts", "cnt", "sz", "cpu", "skw", "node", "ppm", "ip", "cy", "en", "txt", "bool", "id", "fname", "cost", NULL };
static constexpr const char * const creatorTypeNames[]= { "", "all", "unknown", "hthor", "roxie", "roxie:s", "thor", "thor:m", "thor:s", "eclcc", "esp", "summary", NULL };
static constexpr const char * const scopeTypeNames[] = { "", "all", "global", "graph", "subgraph", "activity", "allocator", "section", "operation", "dfu", "edge", "function", "workflow", "child", "file", "channel", "unknown", nullptr };

static unsigned matchString(const char * const * names, const char * search, unsigned dft)
{
    if (!search)
        return dft;

    if (streq(search, "*"))
        search = "all";

    unsigned i=0;
    for (;;)
    {
        const char * next = names[i];
        if (!next)
            return dft;
        if (strieq(next, search))
            return i;
        i++;
    }
}

//--------------------------------------------------------------------------------------------------------------------

static const StatisticScopeType scoreOrder[] = {
    SSTedge,
    SSTactivity,
    SSTnone,
    SSTall,
    SSTglobal,
    SSTgraph,
    SSTsubgraph,
    SSTallocator,
    SSTsection,
    SSToperation,
    SSTdfuworkunit,
    SSTfunction,
    SSTworkflow,
    SSTchildgraph,
    SSTfile,
    SSTchannel,  // MORE - not sure what this means!
    SSTunknown
};
static int scopePriority[SSTmax];

MODULE_INIT(INIT_PRIORITY_STANDARD)
{
    static_assert(_elements_in(scoreOrder) == SSTmax, "Elements missing from scoreOrder[]");
    for (unsigned i=0; i < _elements_in(scoreOrder); i++)
        scopePriority[scoreOrder[i]] = i;
    return true;
}


extern jlib_decl int compareScopeName(const char * left, const char * right)
{
    if (!left || !*left)
    {
        if (!right || !*right)
            return 0;
        else
            return -1;
    }
    else
    {
        if (!right || !*right)
            return +1;
    }

    StatsScopeId leftId;
    StatsScopeId rightId;
    for(;;)
    {
        leftId.extractScopeText(left, &left);
        rightId.extractScopeText(right, &right);
        int result = leftId.compare(rightId);
        if (result != 0)
            return result;
        left = strchr(left, ':');
        right = strchr(right, ':');
        if (!left || !right)
        {
            if (left)
                return +1;
            if (right)
                return -1;
            return 0;
        }
        left++;
        right++;
    }
}

//--------------------------------------------------------------------------------------------------------------------

extern jlib_decl unsigned __int64 getTimeStampNowValue()
{
#ifdef _WIN32
    struct _timeb now;
    _ftime(&now);
    return (unsigned __int64)now.time * I64C(1000000) + now.millitm * 1000;
#else
    struct timeval tm;
    gettimeofday(&tm,NULL);
    return (unsigned __int64)tm.tv_sec * I64C(1000000) + tm.tv_usec;
#endif
}

unsigned __int64 getIPV4StatsValue(const IpAddress & ip)
{
    unsigned ipValue;
    if (ip.getNetAddress(sizeof(ipValue),&ipValue))
        return ipValue;
    return 0;
}

//--------------------------------------------------------------------------------------------------------------------

const static unsigned __int64 oneMicroSecond = I64C(1000);
const static unsigned __int64 oneMilliSecond = I64C(1000000);
const static unsigned __int64 oneSecond = I64C(1000000000);
const static unsigned __int64 oneMinute = I64C(60000000000);
const static unsigned __int64 oneHour = I64C(3600000000000);
const static unsigned __int64 oneDay = 24 * I64C(3600000000000);

void formatTime(StringBuffer & out, unsigned __int64 value)
{
    //Aim to display at least 3 significant digits in the result string
    if (value < oneMicroSecond)
        out.appendf("%uns", (unsigned)value);
    else if (value < oneMilliSecond)
    {
        unsigned uvalue = (unsigned)value;
        out.appendf("%u.%03uus", uvalue / 1000, uvalue % 1000);
    }
    else if (value < oneSecond)
    {
        unsigned uvalue = (unsigned)value;
        out.appendf("%u.%03ums", uvalue / 1000000, (uvalue / 1000) % 1000);
    }
    else
    {
        unsigned days = (unsigned)(value / oneDay);
        value = value % oneDay;
        unsigned hours = (unsigned)(value / oneHour);
        value = value % oneHour;
        unsigned mins = (unsigned)(value / oneMinute);
        value = value % oneMinute;
        unsigned secs = (unsigned)(value / oneSecond);
        unsigned ns = (unsigned)(value % oneSecond);

        if (days > 0)
            out.appendf("%u days ", days);
        if (hours > 0 || days)
            out.appendf("%uh%02um%02us", hours, mins, secs);
        else if (mins >= 10)
            out.appendf("%um%02us", mins, secs);
        else if (mins >= 1)
            out.appendf("%um%02u.%03us", mins, secs, ns / 1000000);
        else
            out.appendf("%u.%03us", secs, ns / 1000000);
    }
}

extern void formatTimeCollatable(StringBuffer & out, unsigned __int64 value, bool nano)
{
    unsigned days = (unsigned)(value / oneDay);
    value = value % oneDay;
    unsigned hours = (unsigned)(value / oneHour);
    value = value % oneHour;
    unsigned mins = (unsigned)(value / oneMinute);
    value = value % oneMinute;
    unsigned secs = (unsigned)(value / oneSecond);
    unsigned ns = (unsigned)(value % oneSecond);

    if (days)
        out.appendf("  %3ud ", days); // Two leading spaces helps the cassandra driver force to a single partition
    else
        out.appendf("       ");
    if (nano)
        out.appendf("%2u:%02u:%02u.%09u", hours, mins, secs, ns);
    else
        out.appendf("%2u:%02u:%02u.%03u", hours, mins, secs, ns/1000000);
    // More than 999 days, I don't care that it goes wrong.
}

extern unsigned __int64 extractTimestamp(const char *s, const char * * end)
{
    if (!s)
        return 0;
    const char * next = nullptr;
    CDateTime timestamp;
    try
    {
        timestamp.setString(s, &next, false); // Sets to date and time given as yyyy-mm-ddThh:mm:ss[.nnnnnnnnn]
    }
    catch(IException * e)
    {
        e->Release();
        return 0;
    }
    //Skip a trailing Z.  The setString function should really process it, but that may break other code in CScmDateTime.
    if (*next == 'Z')
        next++;
    if (end)
        *end = next;

    return timestamp.getTimeStamp();
}

extern unsigned __int64 extractTimeCollatable(const char *s, const char * * end)
{
    if (!s)
        return 0;
    unsigned days,hours,mins;
    double secs;
    unsigned numRead = 0;
    if (sscanf(s, " %ud %u:%u:%lf%n", &days, &hours, &mins, &secs, &numRead)!=4)
    {
        days = 0;
        if (sscanf(s, " %u:%u:%lf%n", &hours, &mins, &secs, &numRead) != 3)
            return 0;
    }
    if (end)
        *end = s + numRead;
    return days*oneDay + hours*oneHour + mins*oneMinute + secs*oneSecond;
}

static void formatTimeStamp(StringBuffer & out, unsigned __int64 value)
{
    time_t seconds = value / 1000000;
    unsigned us = value % 1000000;
    char timeStamp[64];
    time_t tNow = seconds;
#ifdef _WIN32
    struct tm *gmtNow;
    gmtNow = gmtime(&tNow);
    strftime(timeStamp, 64, "%Y-%m-%dT%H:%M:%S", gmtNow);
#else
    struct tm gmtNow;
    gmtime_r(&tNow, &gmtNow);
    strftime(timeStamp, 64, "%Y-%m-%dT%H:%M:%S", &gmtNow);
#endif //_WIN32
    out.append(timeStamp).appendf(".%03uZ", us / 1000);
}

void formatTimeStampAsLocalTime(StringBuffer & out, unsigned __int64 value)
{
    time_t seconds = value / 1000000;
    unsigned us = value % 1000000;
    char timeStamp[64];
    time_t tNow = seconds;
#ifdef _WIN32
    struct tm *gmtNow;
    gmtNow = localtime(&tNow);
    strftime(timeStamp, 64, "%H:%M:%S", gmtNow);
#else
    struct tm gmtNow;
    localtime_r(&tNow, &gmtNow);
    strftime(timeStamp, 64, "%H:%M:%S", &gmtNow);
#endif //_WIN32
    out.append(timeStamp).appendf(".%03u", us / 1000);
}


static const unsigned oneKb = 1024;
static const unsigned oneMb = 1024 * 1024;
static const unsigned oneGb = 1024 * 1024 * 1024;
static unsigned toPermille(unsigned x) { return (x * 1000) / 1024; }
static StringBuffer & formatSize(StringBuffer & out, unsigned __int64 value)
{

    unsigned Gb = (unsigned)(value / oneGb);
    unsigned Mb = (unsigned)((value % oneGb) / oneMb);
    unsigned Kb = (unsigned)((value % oneMb) / oneKb);
    unsigned b = (unsigned)(value % oneKb);
    if (Gb)
        return out.appendf("%u.%03uGB", Gb, toPermille(Mb));
    else if (Mb)
        return out.appendf("%u.%03uMB", Mb, toPermille(Kb));
    else if (Kb)
        return out.appendf("%u.%03uKB", Kb, toPermille(b));
    else
        return out.appendf("%uB", b);
}

static StringBuffer & formatLoad(StringBuffer & out, unsigned __int64 value)
{
    //Stored as millionth of a core.  Display as a percentage => scale by 10,000
    return out.appendf("%u.%03u%%", (unsigned)(value / 10000), (unsigned)(value % 10000) / 10);
}

static StringBuffer & formatSkew(StringBuffer & out, unsigned __int64 value)
{
    //Skew stored as 10000 = perfect, display as percentage
    const __int64 sval = (__int64) value;
    const double percent = ((double)sval) / 100.0;
    if (sval >= 10000 || sval <= -10000) // For values >= 100%, whole numbers
        return out.appendf("%.0f%%", percent);
    else if (sval >= 1000 || sval <= -1000) // For values >= 10%, 1 decimal point
        return out.appendf("%.1f%%", percent);
    else                                 // Anything < 10%, 2 decimal points
        return out.appendf("%.2f%%", percent);
}

static StringBuffer & formatIPV4(StringBuffer & out, unsigned __int64 value)
{
    byte ip1 = (value & 255);
    byte ip2 = ((value >> 8) & 255);
    byte ip3 = ((value >> 16) & 255);
    byte ip4 = ((value >> 24) & 255);
    return out.appendf("%d.%d.%d.%d", ip1, ip2, ip3, ip4);
}

StringBuffer & formatMoney(StringBuffer &out, unsigned __int64 value)
{
    return out.appendf("%.6f", cost_type2money(value));
}

StringBuffer & formatStatistic(StringBuffer & out, unsigned __int64 value, StatisticMeasure measure)
{
    switch (measure)
    {
    case SMeasureNone: // Unknown stat - e.g, on old esp accessing a new workunit
        return out.append(value);
    case SMeasureTimeNs:
        formatTime(out, value);
        return out;
    case SMeasureTimestampUs:
        formatTimeStamp(out, value);
        return out;
    case SMeasureCount:
        return out.append(value);
    case SMeasureSize:
        return formatSize(out, value);
    case SMeasureLoad:
        return formatLoad(out, value);
    case SMeasureSkew:
        return formatSkew(out, value);
    case SMeasureNode:
        return out.append(value);
    case SMeasurePercent:
        return out.appendf("%.2f%%", (double)value / 10000.0);  // stored as ppm
    case SMeasureIPV4:
        return formatIPV4(out, value);
    case SMeasureCycle:
        return out.append(value);
    case SMeasureBool:
        return out.append(boolToStr(value != 0));
    case SMeasureText:
    case SMeasureId:
    case SMeasureFilename:
        return out.append(value);
    case SMeasureEnum:
        return out.append("Enum{").append(value).append("}"); // JCS->GH for now, should map to known enum text somehow
    case SMeasureCost:
        return formatMoney(out, value);
    default:
        return out.append(value).append('?');
    }
}

StringBuffer & formatStatistic(StringBuffer & out, unsigned __int64 value, StatisticKind kind)
{
    return formatStatistic(out, value, queryMeasure(kind));
}

constexpr static stat_type usTimestampThreshold = 100ULL * 365ULL * 24ULL * 3600'000000ULL; // 100 years in us
// Support the gradual conversion of timestamps from us to ns.  If a timestamp is in the old format (before 2070)
// then it is assumed to be in us, otherwise ns.
stat_type normalizeTimestampToNs(stat_type value)
{
    if (value < usTimestampThreshold)
        return value * 1000;
    return value;
}

//--------------------------------------------------------------------------------------------------------------------

stat_type readStatisticValue(const char * cur, const char * * end, StatisticMeasure measure)
{
    char * next = nullptr;
    stat_type value = strtoll(cur, &next, 10);

    switch (measure)
    {
    case SMeasureTimestampUs:
    {
        //ignore value and check if it is a formatted timestamp
        stat_type timestamp = extractTimestamp(cur, end);
        if (timestamp)
            return timestamp;
        //Assume that it is a single number
        break;
    }
    case SMeasureTimeNs:
        //Allow s, ms and us as scaling suffixes
        if (next[0] == 's')
        {
            value *= 1000000000;
            next++;
        }
        else if ((next[0] == 'm') && (next[1] == 's'))
        {
            value *= 1000000;
            next += 2;
        }
        else if ((next[0] == 'u') && (next[1] == 's'))
        {
            value *= 1000;
            next += 2;
        }
        else if ((next[0] == 'n') && (next[1] == 's'))
        {
            next += 2;
        }
        else
        {
            stat_type timestamp = extractTimeCollatable(cur, end);
            if (timestamp)
                return timestamp;
            //Assume that it is a single number
        }
        break;
    case SMeasureCount:
    case SMeasureSize:
        //Allow K, M, G as scaling suffixes
        if (next[0] == 'K')
        {
            value *= 0x400;
            next++;
        }
        else if (next[0] == 'M')
        {
            value *= 0x100000;
            next++;
        }
        else if (next[0] == 'G')
        {
            value *= 0x40000000;
            next++;
        }
        //Skip bytes marker
        if ((*next == 'b') || (*next == 'B'))
            next++;
        break;
    case SMeasurePercent:
        //MORE: Extend to allow fractional percentages
        //Allow % to mean a percentage - instead of ppm
        if (next[0] == '%')
        {
            value *= 10000;
            next++;
        }
        break;
    }

    if (end)
        *end = next;
    return value;
}


void validateScopeId(const char * idText)
{
    StatsScopeId id;
    if (!id.setScopeText(idText))
        throw makeStringExceptionV(JLIBERR_UnexpectedValue, "'%s' does not appear to be a valid scope id", idText);
}


void validateScope(const char * scopeText)
{
    StatsScopeId id;
    const char * cur = scopeText;
    for(;;)
    {
        if (!id.setScopeText(cur, &cur))
            throw makeStringExceptionV(JLIBERR_UnexpectedValue, "'%s' does not appear to be a valid scope id", cur);
        cur = strchr(cur, ':');
        if (!cur)
            return;
        cur++;
    }
}

StatisticScopeType getScopeType(const char * scope)
{
    StatsScopeId id;
    if (nullptr == scope)
        id.setId(SSTglobal, 0);
    else if (startsWith(scope, "compile"))
        id.setId(SSToperation, 0);
    else
    {
        const char * colon = strchr(scope, ':');
        if (colon)
            id.setScopeText(colon+1);
        else
            id.setScopeText(scope);
    }
    return id.queryScopeType();
}

//--------------------------------------------------------------------------------------------------------------------

unsigned queryScopeDepth(const char * text)
{
    if (!*text)
        return 0;

    unsigned depth = 1;
    for (;;)
    {
        switch (*text)
        {
            case 0:
                return depth;
            case ':':
                depth++;
                break;
        }
        text++;
    }
}

const char * queryScopeTail(const char * scope)
{
    const char * colon = strrchr(scope, ':');
    if (colon)
        return colon+1;
    else
        return scope;
}

bool getParentScope(StringBuffer & parent, const char * scope)
{
    const char * colon = strrchr(scope, ':');
    if (colon)
    {
        parent.append(colon-scope, scope);
        return true;
    }
    else
        return false;
}


void describeScope(StringBuffer & description, const char * scope)
{
    if (!*scope)
        return;

    StatsScopeId id;
    for(;;)
    {
        id.extractScopeText(scope, &scope);
        id.describe(description);
        if (!*scope)
            return;
        description.append(": ");
        scope++;
    }
}

bool isParentScope(const char *parent, const char *scope)
{
    const char *p = parent;
    const char *q = scope;
    while(*p && (*p==*q))
    {
        ++p;
        ++q;
    }
    if ((*p==0) && (*q==':' || *q==0))
        return true;
    return false;
}

const char * queryMeasurePrefix(StatisticMeasure measure)
{
    switch (measure)
    {
    case SMeasureAll:           return nullptr;
    case SMeasureTimeNs:        return "Time";
    case SMeasureTimestampUs:   return "When";
    case SMeasureCount:         return "Num";
    case SMeasureSize:          return "Size";
    case SMeasureLoad:          return "Load";
    case SMeasureSkew:          return "Skew";
    case SMeasureNode:          return "Node";
    case SMeasurePercent:       return "Per";
    case SMeasureIPV4:          return "Ip";
    case SMeasureCycle:         return "Cycle";
    case SMeasureEnum:          return "";
    case SMeasureText:          return "";
    case SMeasureBool:          return "Is";
    case SMeasureId:            return "Id";
    case SMeasureFilename:      return "";
    case SMeasureCost:          return "Cost";
    case SMeasureNone:          return nullptr;
    default:
        return "Unknown";
    }
}

const char * queryMeasureName(StatisticMeasure measure)
{
    return measureNames[measure];
}

StatisticMeasure queryMeasure(const char * measure, StatisticMeasure dft)
{
    //MORE: Use a hash table??
    StatisticMeasure ret = (StatisticMeasure)matchString(measureNames, measure, SMeasureMax);
    if (ret != SMeasureMax)
        return ret;

    //Legacy support for an unusual statistic - pretend the sizes are in bytes instead of kb.
    if (streq(measure, "kb"))
        return SMeasureSize;

    for (unsigned i1=SMeasureAll+1; i1 < SMeasureMax; i1++)
    {
        const char * prefix = queryMeasurePrefix((StatisticMeasure)i1);
        if (strieq(measure, prefix))
            return (StatisticMeasure)i1;
    }

    return dft;
}

//--------------------------------------------------------------------------------------------------------------------

#define BASE_NAMES(x, y) \
    #x #y, \
    #x "Min" # y,  \
    #x "Max" # y, \
    #x "Avg" # y, \
    "Skew" # y, \
    "SkewMin" # y, \
    "SkewMax" # y, \
    "NodeMin" # y, \
    "NodeMax" # y,

#define NAMES(x, y) \
    BASE_NAMES(x, y) \
    #x "Delta" # y, \
    #x "StdDev" #y,


#define WHENNAMES(x, y) \
    BASE_NAMES(x, y) \
    "TimeDelta" # y, \
    "TimeStdDev" # y,

#define BASE_TAGS(x, y) \
    "@" #x "Min" # y,  \
    "@" #x "Max" # y, \
    "@" #x "Avg" # y, \
    "@Skew" # y, \
    "@SkewMin" # y, \
    "@SkewMax" # y, \
    "@NodeMin" # y, \
    "@NodeMax" # y,

//Default tags nothing special overriden
#define TAGS(x, y) \
    "@" #x #y, \
    BASE_TAGS(x, y) \
    "@" #x "Delta" # y, \
    "@" #x "StdDev" # y,

//Define the tags for time items.
#define WHENTAGS(x, y) \
    "@" #x #y, \
    BASE_TAGS(x, y) \
    "@TimeDelta" # y, \
    "@TimeStdDev" # y,

#define CORESTAT(x, y, m, mm)     St##x##y, m, mm, St##x##y, St##x##y, { NAMES(x, y) }, { TAGS(x, y) }
#define STAT(x, y, m, mm)         CORESTAT(x, y, m, mm)

//--------------------------------------------------------------------------------------------------------------------

//These are the macros to use to define the different entries in the stats meta table
//#define TIMESTAT(y) STAT(Time, y, SMeasureTimeNs)
#define TIMESTAT(y) St##Time##y, SMeasureTimeNs, StatsMergeSum, St##Time##y, St##Cycle##y##Cycles, { NAMES(Time, y) }, { TAGS(Time, y) }
#define WHENFIRSTSTAT(y) St##When##y, SMeasureTimestampUs, StatsMergeFirst, St##When##y, St##When##y, { WHENNAMES(When, y) }, { WHENTAGS(When, y) }
#define WHENLASTSTAT(y) St##When##y, SMeasureTimestampUs, StatsMergeLast, St##When##y, St##When##y, { WHENNAMES(When, y) }, { WHENTAGS(When, y) }
#define NUMSTAT(y) STAT(Num, y, SMeasureCount, StatsMergeSum)
#define SIZESTAT(y) STAT(Size, y, SMeasureSize, StatsMergeSum)
#define LOADSTAT(y) STAT(Load, y, SMeasureLoad, StatsMergeMax)
#define SKEWSTAT(y) STAT(Skew, y, SMeasureSkew, StatsMergeMax)
#define NODESTAT(y) STAT(Node, y, SMeasureNode, StatsMergeKeepNonZero)
#define PERSTAT(y) STAT(Per, y, SMeasurePercent, StatsMergeReplace)
#define IPV4STAT(y) STAT(IPV4, y, SMeasureIPV4, StatsMergeKeepNonZero)
#define CYCLESTAT(y) St##Cycle##y##Cycles, SMeasureCycle, StatsMergeSum, St##Time##y, St##Cycle##y##Cycles, { NAMES(Cycle, y##Cycles) }, { TAGS(Cycle, y##Cycles) }, "<cycles>"
#define ENUMSTAT(y) STAT(Enum, y, SMeasureEnum, StatsMergeKeepNonZero)
#define COSTSTAT(y) STAT(Cost, y, SMeasureCost, StatsMergeSum)
#define PEAKSIZESTAT(y) STAT(Size, y, SMeasureSize, StatsMergeMax)
#define PEAKNUMSTAT(y) STAT(Num, y, SMeasureCount, StatsMergeMax)
//--------------------------------------------------------------------------------------------------------------------

class StatisticMeta
{
public:
    StatisticKind kind;
    StatisticMeasure measure;
    StatsMergeAction mergeAction;
    StatisticKind serializeKind;
    StatisticKind rawKind;
    const char * names[StNextModifier/StVariantScale];
    const char * tags[StNextModifier/StVariantScale];
    const char * description;
};

constexpr const char * UNUSED = "<unused>";

//The order of entries in this table must match the order in the enumeration
static const constexpr StatisticMeta statsMetaData[StMax] = {
    { StKindNone, SMeasureNone, StatsMergeSum, StKindNone, StKindNone, { "none" }, { "@none" }, nullptr },
    { StKindAll, SMeasureAll, StatsMergeSum, StKindAll, StKindAll, { "all" }, { "@all" }, nullptr },
    { WHENFIRSTSTAT(GraphStarted), "The time when a graph started./nDeprecated" }, // Deprecated - use WhenStart
    { WHENLASTSTAT(GraphFinished), "The time when a graph finished./nDeprecated"  }, // Deprecated - use WhenFinished
    { WHENFIRSTSTAT(FirstRow), "The time when the first row is processed by an activity"  },
    { WHENFIRSTSTAT(QueryStarted), "The time when a query started./nDeprecated" }, // Deprecated - use WhenStart
    { WHENLASTSTAT(QueryFinished), "The time when a query finished./nDeprecated" }, // Deprecated - use WhenFinished
    { WHENFIRSTSTAT(Created), "The time when an item was created" },
    { WHENFIRSTSTAT(Compiled), "The time a workunit started being compiled" },
    { WHENFIRSTSTAT(WorkunitModified), UNUSED },
    { TIMESTAT(Elapsed), "The elapsed time between starting and finishing\nFor child queries this may be significantly larger than TimeTotalExecute" },
    { TIMESTAT(LocalExecute), "The time spent executing this activity not including its inputs\nSort activities by local execute time to help isolate potential processing bottlenecks" },
    { TIMESTAT(TotalExecute), "The time spent executing this activity and its inputs\nSort activities by total execute time to find sections of a query that are a bottleneck" },
    { TIMESTAT(Remaining), UNUSED },
    { SIZESTAT(GeneratedCpp), "The size of the generated c++ file" },
    { SIZESTAT(PeakMemory), "The peak memory used while processing this item" },
    { SIZESTAT(MaxRowSize), "The high water mark of the memory used for representing rows (roxiemem)" },
    { NUMSTAT(RowsProcessed), "The number of rows processed" },
    { NUMSTAT(Slaves), "The number of parallel execution processes used to execute an activity" },
    { NUMSTAT(Starts), "The number of times the activity has started executing\nAn activity is active if this does not match NumStops" },
    { NUMSTAT(Stops), "The number of times the activity has stopped executing\nAn activity is active if this is less than NumStarts" },
    { NUMSTAT(IndexSeeks), "The number of keyed lookups on an index\nThese correspond to KEYED() filters on indexes.  A single keyed filter may result in multiple seeks if a leading component is not single-valued" },
    { NUMSTAT(IndexScans), "The number of index scans\nHow many entries are sequentially examined after an initial seek (including wild seeks).  Large numbers compared to the number of seeks may indicate extra keyed filters would be worthwhile" },
    { NUMSTAT(IndexWildSeeks), "The number of seeks caused by WILD() filters\nThe number of keyed lookups that had to search for the next potential match.  If this is a high proportion of NumIndexScans it may suggest poor key design" },
    { NUMSTAT(IndexSkips), "The number of smart-stepping operations that increment the next match" },
    { NUMSTAT(IndexNullSkips), "The number of smart-stepping operations that had no effect\nIf this is large compare to NumIndexSkips it suggests the priority may not be set correctly" },
    { NUMSTAT(IndexMerges), "The number of merges set up when smart stepping"},
    { NUMSTAT(IndexMergeCompares), "The number of merge comparisons when smart stepping" },
    { NUMSTAT(PreFiltered), "The number of LEFT rows filtered before performing a keyed lookup" },
    { NUMSTAT(PostFiltered), "The number of index matches filtered by the transform and the non-keyed filter" },
    { NUMSTAT(BlobCacheHits), "The number of times a blob was resolved in the cache" },
    { NUMSTAT(LeafCacheHits), "The number of times a leaf node was resolved in the cache" },
    { NUMSTAT(NodeCacheHits), "The number of times a branch node was resolved in the cache" },
    { NUMSTAT(BlobCacheAdds), "The number of times a blob was read from disk rather than the cache" },
    { NUMSTAT(LeafCacheAdds), "The number of times a leaf node was read from disk rather than the cache\nIf this number is high it may be worth experimenting with the leaf cache size (the branch cache size is more important)" },
    { NUMSTAT(NodeCacheAdds), "The number of times a branch node was read from disk rather than the cache\nBranch cache hits are significant for performance.  If this number is high it is likely to be worth increasing the node cache size.  If this number does not increase once the system is warmed up it may be worth reducing the cache size" },
    { NUMSTAT(PreloadCacheHits), UNUSED },
    { NUMSTAT(PreloadCacheAdds), UNUSED },
    { NUMSTAT(ServerCacheHits), UNUSED },
    { NUMSTAT(IndexAccepted), "The number of KEYED JOIN matches that return a result from the TRANSFORM" },
    { NUMSTAT(IndexRejected), "The number of KEYED JOIN matches that are skipped by the TRANSFORM" },
    { NUMSTAT(AtmostTriggered), "The number of times ATMOST on a JOIN causes it to fail to match" },
    { NUMSTAT(DiskSeeks), "The number of FETCHES from disk" },
    { NUMSTAT(Iterations), "The number of LOOP iterations executed" },
    { LOADSTAT(WhileSorting), UNUSED },
    { NUMSTAT(LeftRows), "The number of LEFT rows processed" },
    { NUMSTAT(RightRows), "The number of RIGHT rows processed"  },
    { PERSTAT(Replicated), "The percentage replication complete" },
    { NUMSTAT(DiskRowsRead), "The number of rows read from the file" },
    { NUMSTAT(IndexRowsRead), "The number of rows read from the index" },
    { NUMSTAT(DiskAccepted), "The number of disk rows that return a result from the TRANSFORM" },
    { NUMSTAT(DiskRejected), "The number of disk rows that are skipped by the TRANSFORM" },
    { TIMESTAT(Soapcall), "The time taken to execute a SOAPCALL" },
    { TIMESTAT(FirstExecute), "The time taken to return the first row from this activity" },
    { TIMESTAT(DiskReadIO), "Total time spent reading from disk" },
    { TIMESTAT(DiskWriteIO), "Total time spent writing to disk" },
    { SIZESTAT(DiskRead), "Total size of data read from disk" },
    { SIZESTAT(DiskWrite), "Total size of data written to disk" },
    { CYCLESTAT(DiskReadIO) },
    { CYCLESTAT(DiskWriteIO) },
    { NUMSTAT(DiskReads), "The number of disk read operations" },
    { NUMSTAT(DiskWrites), "The number of disk write operations" },
    { NUMSTAT(Spills), "The number of times the activity spilt to disk"},
    { TIMESTAT(SpillElapsed), "Time spent spilling rows from memory to disk" }, //MORE: Do we have a similar stat for SpillRead?
    { TIMESTAT(SortElapsed), "Time spent sorting rows in memory" },
    { NUMSTAT(Groups), "The number of groups processed by this activity" },
    { NUMSTAT(GroupMax), "The size of the largest group processed by this activity\nA skew in group size can cause a skew in processing time.  A large skew may indicate some special values would benefit from special casing" },
    { SIZESTAT(SpillFile), "Total size of data spilled to disk" },
    { CYCLESTAT(SpillElapsed) },
    { CYCLESTAT(SortElapsed) },
    { NUMSTAT(Strands), "The number of parallel execution strands\n(A partially implemented feature to allow parallel execution within an activity)" },
    { CYCLESTAT(TotalExecute) },
    { NUMSTAT(Executions), "The number of times a graph has been executed" },
    { TIMESTAT(TotalNested), UNUSED },
    { CYCLESTAT(LocalExecute) },
    { NUMSTAT(Compares), UNUSED },
    { NUMSTAT(ScansPerRow), UNUSED },
    { NUMSTAT(Allocations), "The number of allocations from the row memory" },
    { NUMSTAT(AllocationScans), "The number of scans within the memory manager when allocating row memory\nOnly applies to the scanning heap manager (not used by default)" },
    { NUMSTAT(DiskRetries), "The number of times an I/O operation was retried\nIf this is non-zero it may suggest a problem with the underlying disk storage" },
    { CYCLESTAT(Elapsed) },
    { CYCLESTAT(Remaining) },
    { CYCLESTAT(Soapcall) },
    { CYCLESTAT(FirstExecute) },
    { CYCLESTAT(TotalNested) },
    { TIMESTAT(Generate), "Time taken to generate the c++ code from the parsed ECL" },
    { CYCLESTAT(Generate) },
    { WHENFIRSTSTAT(Started), "Time when this activity or operation started" },
    { WHENLASTSTAT(Finished), "Time when this activity or operation finished" },
    { NUMSTAT(AnalyseExprs), "Code generator internal\nThe number of expressions that were processed by transformer::analyse()" },
    { NUMSTAT(TransformExprs), "Code generator internal\nThe number of expressions that were processed by transformer::transform()" },
    { NUMSTAT(UniqueAnalyseExprs), "Code generator internal\nThe number of unique expressions that were processed by transformer::analyse()" },
    { NUMSTAT(UniqueTransformExprs), "Code generator internal\nThe number of unique expressions that were processed by transformer::transform()" },
    { NUMSTAT(DuplicateKeys), "The number of duplicate keys that were present in the index" },
    { NUMSTAT(AttribsProcessed), "The number of attributes processed when parsing the ECL" },
    { NUMSTAT(AttribsSimplified), UNUSED },
    { NUMSTAT(AttribsFromCache), UNUSED },
    { NUMSTAT(SmartJoinDegradedToLocal), "The number of times a global smart-join switched to a LOCAL JOIN (with distribute)\nThis will be 0 or 1 unless the activity is within a LOOP" },
    { NUMSTAT(SmartJoinSlavesDegradedToStd), "The number of times a global smart-join degraded to a standard join" },
    { NUMSTAT(AttribsSimplifiedTooComplex), UNUSED },
    { NUMSTAT(SysContextSwitches), "The number of context switches that occurred when processing" },
    { TIMESTAT(OsUser), "Total elapsed user-space time" },
    { TIMESTAT(OsSystem), "Total time spent in the system/kernel" },
    { TIMESTAT(OsTotal), "Total elapsed time according to the OS\nIncludes system, user, idle and iowait times" },
    { CYCLESTAT(OsUser) },
    { CYCLESTAT(OsSystem) },
    { CYCLESTAT(OsTotal) },
    //The following seem to be duplicates of the values above
    { NUMSTAT(ContextSwitches), "The number of context switches that occurred when processing" },
    { TIMESTAT(User), "Total elapsed user-space time" },
    { TIMESTAT(System), "Total time spent in the system/kernel" },
    { TIMESTAT(Total), "Total elapsed time according to the OS\nInclude system,user,idle and iowait times" },
    { CYCLESTAT(User) },
    { CYCLESTAT(System) },
    { CYCLESTAT(Total) },
    { SIZESTAT(OsDiskRead), UNUSED },
    { SIZESTAT(OsDiskWrite), UNUSED },
    { TIMESTAT(Blocked), "Time spent blocked waiting for another operation to complete" },
    { CYCLESTAT(Blocked) },
    { COSTSTAT(Execute), "The CPU cost of executing" },
    { SIZESTAT(AgentReply), "Size of data sent from the workers to the agent" },
    { TIMESTAT(AgentWait), "Time that the agent spent waiting for a reply from the workers" },
    { CYCLESTAT(AgentWait) },
    { COSTSTAT(FileAccess), "The transactional cost of any file operations" },
    { NUMSTAT(Pods), "The number of pods used" },
    { COSTSTAT(Compile), "The cost to compile this workunit"},
    { TIMESTAT(NodeLoad), "Time spent reading branch nodes from disk and decompressing them" },
    { CYCLESTAT(NodeLoad) },
    { TIMESTAT(LeafLoad), "Time spent reading leaf nodes from disk and decompressing them\nIf this is a high proportion of the time (especially compared to TimeLeafRead) then consider using the new index compression formats" },
    { CYCLESTAT(LeafLoad) },
    { TIMESTAT(BlobLoad), "Time spent reading blob nodes from disk and decompressing them" },
    { CYCLESTAT(BlobLoad) },
    { TIMESTAT(Dependencies), "Time spent processing dependencies for this activity"},
    { CYCLESTAT(Dependencies) },
    { TIMESTAT(Start), "Time taken to start an activity\nThis includes the time spent processing dependencies" },
    { CYCLESTAT(Start) },
    { ENUMSTAT(ActivityCharacteristics), "A bitfield describing characteristics of the activity\nurgentStart = 0x01, hasRowLatency = 0x02, hasDependencies = 0x04, slowDependencies = 0x08" },
    { TIMESTAT(NodeRead), "Time spent reading branch nodes from disk (including linux page cache)" },
    { CYCLESTAT(NodeRead) },
    { TIMESTAT(LeafRead), "Time spent reading leaf nodes from disk (including linux page cache)" },
    { CYCLESTAT(LeafRead) },
    { TIMESTAT(BlobRead), "Time spent reading blob from disk (including linux page cache)" },
    { CYCLESTAT(BlobRead) },
    { NUMSTAT(NodeDiskFetches), "Number of times a branch node was read from disk rather than the linux page cache" },
    { NUMSTAT(LeafDiskFetches), "Number of times a leaf node was read from disk rather than the linux page cache\nIf this is a significant proportion of NumLeafAdds then consider allocating more memory, or reducing the leaf cache size" },
    { NUMSTAT(BlobDiskFetches), "Number of times a blob was read from disk rather than the linux page cache" },
    { TIMESTAT(NodeFetch), "Time spent reading branch nodes from disk (EXCLUDING the linux page cache)" },
    { CYCLESTAT(NodeFetch) },
    { TIMESTAT(LeafFetch), "Time spent reading leaf nodes from disk (EXCLUDING the linux page cache)" },
    { CYCLESTAT(LeafFetch) },
    { TIMESTAT(BlobFetch), "Time spent reading blobs from disk (EXCLUDING the linux page cache)" },
    { CYCLESTAT(BlobFetch) },
    { PEAKSIZESTAT(GraphSpill), "High water mark for inter-subgraph spill size" },
    { TIMESTAT(AgentQueue), "Time worker items were received and queued before being processed\nThis may indicate that the primary node on a channel was down, or that the workers are overloaded with requests" },
    { CYCLESTAT(AgentQueue) },
    { TIMESTAT(IBYTIDelay), "Time spent waiting for another worker to start processing a request\nA non-zero value indicates that the primary node on a channel was down or very busy" },
    { CYCLESTAT(IBYTIDelay) },
    { WHENFIRSTSTAT(Queued), "The time when this item was added to a queue" },
    { WHENFIRSTSTAT(Dequeued), "The time when this item was removed from a queue" },
    { WHENFIRSTSTAT(K8sLaunched), "The time when the K8s job to process this item was launched" },
    { WHENFIRSTSTAT(K8sStarted), "The time when the K8s job to process this item started executing/nThe difference between the K8sStarted and K8sLaunched indicates how long Kubernetes took to resource and initialised the job" },
    { WHENFIRSTSTAT(K8sReady), "The time when the Thor job is ready to process\nThe difference with K8sStarted indicates how long it took to resource and start the slave processes" },
    { NUMSTAT(SocketWrites), "The number of writes to the client socket" },
    { SIZESTAT(SocketWrite), "The size of data written to the client socket" },
    { TIMESTAT(SocketWriteIO), "The total time spent writing data to the client socket" },
    { CYCLESTAT(SocketWriteIO) },
    { NUMSTAT(SocketReads), "The number of reads from the client socket" },
    { SIZESTAT(SocketRead), "The size of data read from the client socket" },
    { TIMESTAT(SocketReadIO), "The total time spent reading data from the client socket" },
    { CYCLESTAT(SocketReadIO) },
    { SIZESTAT(Memory), "The total memory allocated from the system" },
    { SIZESTAT(RowMemory), "The size of memory used to store rows" },
    { SIZESTAT(PeakRowMemory), "The peak memory used to store rows" },
    { SIZESTAT(AgentSend), "The size of data sent to the agent from the server" },
    { TIMESTAT(IndexCacheBlocked), "The time spent waiting to access the index page cache" },
    { CYCLESTAT(IndexCacheBlocked) },
    { TIMESTAT(AgentProcess), "The total time spent by the agents processing requests" },
    { CYCLESTAT(AgentProcess) },
    { NUMSTAT(AckRetries), "The number of times the server failed to receive a response from an agent within the expected time" },
    { SIZESTAT(ContinuationData), "The total size of continuation data sent from agent to the server\nA large number may indicate a poor filter, or merging from many different index locations" },
    { NUMSTAT(ContinuationRequests), "The number of times the agent indicated there was more data to be returned" },
    { NUMSTAT(Failures), "The number of times a query has failed" },
    { NUMSTAT(LocalRows), "Number of rows handled locally"},
    { NUMSTAT(RemoteRows), "Number of rows sent to remote workers"},
    { SIZESTAT(RemoteWrite), "Size of data sent to remote workers"},
    { PEAKSIZESTAT(PeakTempDisk), "High water mark for temporary files"},
    { PEAKSIZESTAT(PeakEphemeralDisk), "High water mark for emphemeral storage use"},
    { NUMSTAT(MatchLeftRowsMax), "The largest number of left rows in a join group" },
    { NUMSTAT(MatchRightRowsMax), "The largest number of right rows in a join group" },
    { NUMSTAT(MatchCandidates), "The number of candidate combinations of left and right rows forming join groups" },
    { NUMSTAT(MatchCandidatesMax), "The largest number of candidate combinations of left and right rows in a single group" },
    { NUMSTAT(ParallelExecute), "The number of parallel execution paths for this activity" },
    { NUMSTAT(AgentRequests), "The number of agent request packets for this activity" },
    { SIZESTAT(AgentRequests), "The total size of agent request packets for this activity" },
    { TIMESTAT(SoapcallDNS), "The time taken for DNS lookup in SOAPCALL" },
    { TIMESTAT(SoapcallConnect), "The time taken for connect[+SSL_connect] in SOAPCALL" },
    { CYCLESTAT(SoapcallDNS) },
    { CYCLESTAT(SoapcallConnect) },
    { NUMSTAT(SoapcallConnectFailures), "The number of SOAPCALL connect failures" },
    { TIMESTAT(LookAhead), "The total time lookahead thread spend prefetching rows from upstream activities" },
    { CYCLESTAT(LookAhead) },
    { NUMSTAT(CacheHits), "The number of times an item was retrieved from a cache" },
    { NUMSTAT(CacheAdds), "The number of times an item was added to a cache" },
    { PEAKNUMSTAT(PeakCacheObjects), "High water mark for number of objects in a cache"},
    { NUMSTAT(CacheDuplicates), "The number of times an item was added to a cache by two threads at the same time" },
    { NUMSTAT(CacheEvictions), "The number of times an item was evicted from a cache" },
    { SIZESTAT(OffsetBranches), "The 1st branch node offset position in the index" },
    { SIZESTAT(BranchMemory), "The estimated size of the branch nodes when stored in memory" },
    { SIZESTAT(LeafMemory), "The estimated size of the leaf nodes when stored in memory"},
    { SIZESTAT(LargestExpandedLeaf), "The size of the largest leaf node when expanded in memory"},
    { TIMESTAT(Delayed), "Time spent waiting for minimum query execution period" },
    { CYCLESTAT(Delayed) },
    { TIMESTAT(PostMortemCapture), "The time taken for post mortem capture" },
    { CYCLESTAT(PostMortemCapture) },
    { NUMSTAT(BloomAccepts), "The number of times a bloom filter accepts an index lookup" },
    { NUMSTAT(BloomRejects), "The number of times a bloom filter rejects an index lookup" },
    { NUMSTAT(BloomSkips), "The number of times a bloom filter cannot filter an index lookup" },
    { NUMSTAT(Accepts), "The number of items accepted for processing" },
    { NUMSTAT(Waits), "The number of times a component waits for an entry on a queue" },
    { TIMESTAT(Provision), "The total time spent provisioning a component" },
    { CYCLESTAT(Provision) },
    { COSTSTAT(Start), "The cost associated with starting a component or operation" },
    { TIMESTAT(WaitSuccess), "The time waiting for an item on a queue, when an item was eventually received" },
    { CYCLESTAT(WaitSuccess) },
    { TIMESTAT(WaitFailure), "The time waiting for an item on a queue, when no item was received" },
    { CYCLESTAT(WaitFailure) },
    { COSTSTAT(Wait), "The cost associated with a component being idle waiting for an event" },
    { NUMSTAT(Aborts), "The number of times an abort was processed" },
    { COSTSTAT(Abort), "The cost associated with aborted actions" },
    { NUMSTAT(RowsRead), "The number of rows read from an input" },
    { NUMSTAT(RowsWritten), "The number of rows written to an output" },
    { TIMESTAT(QueryConsume), "The total time spent consuming and processing a query input" },
    { CYCLESTAT(QueryConsume) },
    { NUMSTAT(Successes), "The number of times something was successful" },
    { NUMSTAT(SoapcallRetries), "The number of times a soapcall request retries" },
};

static MapStringTo<StatisticKind, StatisticKind> statisticNameMap(true);

MODULE_INIT(INIT_PRIORITY_STANDARD)
{
    //Insert the names into a hash table to allow fast string->kind mapping
    statisticNameMap.setValue("*", StKindAll);

    for (unsigned variant=0; variant < StNextModifier; variant += StVariantScale)
    {
        for (unsigned i=0; i < StMax; i++)
        {
            StatisticKind kind = (StatisticKind)(i+variant);
            const char * shortName = queryStatisticName(kind);
            if (shortName)
                statisticNameMap.setValue(shortName, kind);
        }
    }
    return true;
}

//Is a 0 value likely, and useful to be reported if it does happen to be zero?
bool includeStatisticIfZero(StatisticKind kind)
{
    switch (kind)
    {
    case StNumRowsProcessed:
    case StNumIterations:
    case StNumIndexSeeks:
    case StNumDuplicateKeys:
        return true;
    }
    return false;
}


inline StatsMergeAction queryBaseMergeMode(StatisticKind kind)
{
    dbgassertex(queryStatsVariant(kind) == 0);
    dbgassertex(kind >= StKindNone && kind < StMax);
    return statsMetaData[kind].mergeAction;
}

extern jlib_decl StatsMergeAction queryMergeMode(StatisticKind kind)
{
    if (queryStatsVariant(kind) == 0)
        return queryBaseMergeMode(kind);
    unsigned variant = queryStatsVariant(kind);
    switch (variant)
    {
    case StSkew:
    case StSkewMin:
    case StSkewMax:
        return StatsMergeMax;
    case StNodeMin:
    case StNodeMax:
        return StatsMergeKeepNonZero;
    }
    return queryBaseMergeMode((StatisticKind)(kind & StKindMask));
}

//--------------------------------------------------------------------------------------------------------------------

StatisticMeasure queryMeasure(StatisticKind kind)
{
    unsigned variant = queryStatsVariant(kind);
    switch (variant)
    {
    case StSkew:
    case StSkewMin:
    case StSkewMax:
        return SMeasureSkew;
    case StNodeMin:
    case StNodeMax:
        return SMeasureNode;
    case StDeltaX:
    case StStdDevX:
        {
            StatisticMeasure measure = queryMeasure((StatisticKind)(kind & StKindMask));
            switch (measure)
            {
            case SMeasureTimestampUs:
                return SMeasureTimeNs;
            default:
                return measure;
            }
            break;
        }
    }

    StatisticKind rawkind = (StatisticKind)(kind & StKindMask);
    if (rawkind >= StKindNone && rawkind < StMax)
        return statsMetaData[rawkind].measure;
    return SMeasureNone;
}

const char * queryStatisticName(StatisticKind kind)
{
    StatisticKind rawkind = (StatisticKind)(kind & StKindMask);
    unsigned variant = (kind / StVariantScale);
    dbgassertex(variant < (StNextModifier/StVariantScale));
    if (rawkind >= StKindNone && rawkind < StMax)
        return statsMetaData[rawkind].names[variant];
    return "Unknown";
}


const char * queryStatisticDescription(StatisticKind kind)
{
    StatisticKind rawkind = (StatisticKind)(kind & StKindMask);
    if (rawkind >= StKindNone && rawkind < StMax)
        return statsMetaData[rawkind].description;
    return nullptr;
}

unsigned __int64 convertMeasure(StatisticMeasure from, StatisticMeasure to, unsigned __int64 value)
{
    if (from == to)
        return value;
    if ((from == SMeasureCycle) && (to == SMeasureTimeNs))
        return cycle_to_nanosec(value);
    if ((from == SMeasureTimeNs) && (to == SMeasureCycle))
        return nanosec_to_cycle(value);
    if ((from == SMeasureTimestampUs) && (to == SMeasureTimeNs))
        return value * 1000;
    if ((from == SMeasureTimeNs) && (to == SMeasureTimestampUs))
        return value / 1000;

#ifdef _DEBUG
    throwUnexpected();
#else
    return value;
#endif
}

unsigned __int64 convertMeasure(StatisticKind from, StatisticKind to, unsigned __int64 value)
{
    if (from == to)
        return value;
    return convertMeasure(queryMeasure(from), queryMeasure(to), value);
}

static unsigned __int64 convertSumMeasure(StatisticKind from, StatisticKind to, double value)
{
    if (from == to)
        return value;
    return convertMeasure(queryMeasure(from), queryMeasure(to), value);
}


static double convertSquareMeasure(StatisticMeasure from, StatisticMeasure to, double value)
{
    if (from == to)
        return value;

    //Coded to a avoid overflow of unsigned __int64 in cycle_to_nanosec etc.
    const unsigned __int64 largeValue = 1000000000;
    double scale;
    if ((from == SMeasureCycle) && (to == SMeasureTimeNs))
        scale = (double)cycle_to_nanosec(largeValue) / (double)largeValue;
    else if ((from == SMeasureTimeNs) && (to == SMeasureCycle))
        scale = (double)nanosec_to_cycle(largeValue) / (double)largeValue;
    else
    {
#ifdef _DEBUG
        throwUnexpected();
#else
        scale = 1.0;
#endif
    }

    return value * scale * scale;
}

static double convertSquareMeasure(StatisticKind from, StatisticKind to, double value)
{
    return convertSquareMeasure(queryMeasure(from), queryMeasure(to), value);
}


static StatisticKind querySerializedKind(StatisticKind kind)
{
    StatisticKind rawkind = (StatisticKind)(kind & StKindMask);
    if (rawkind >= StMax)
        return kind;
    StatisticKind serialKind = statsMetaData[rawkind].serializeKind;
    return (StatisticKind)(serialKind | (kind & ~StKindMask));
}

static StatisticKind queryRawKind(StatisticKind kind)
{
    StatisticKind basekind = (StatisticKind)(kind & StKindMask);
    if (basekind >= StMax)
        return kind;
    StatisticKind rawKind = statsMetaData[basekind].rawKind;
    return (StatisticKind)(rawKind | (kind & ~StKindMask));
}

//--------------------------------------------------------------------------------------------------------------------

void queryLongStatisticName(StringBuffer & out, StatisticKind kind)
{
    out.append(queryStatisticName(kind));
}

//--------------------------------------------------------------------------------------------------------------------

const char * queryTreeTag(StatisticKind kind)
{
    StatisticKind rawkind = (StatisticKind)(kind & StKindMask);
    unsigned variant = (kind / StVariantScale);
    dbgassertex(variant < (StNextModifier/StVariantScale));
    if (rawkind >= StKindNone && rawkind < StMax)
        return statsMetaData[rawkind].tags[variant];
    return "@Unknown";
}

//--------------------------------------------------------------------------------------------------------------------

StatisticKind queryStatisticKind(const char * search, StatisticKind dft)
{
    if (!search)
        return dft;
    StatisticKind * match = statisticNameMap.getValue(search);
    if (match)
        return *match;
    return dft;
}

//--------------------------------------------------------------------------------------------------------------------

const char * queryCreatorTypeName(StatisticCreatorType sct)
{
    return creatorTypeNames[sct];
}

StatisticCreatorType queryCreatorType(const char * sct, StatisticCreatorType dft)
{
    //MORE: Use a hash table??
    return (StatisticCreatorType)matchString(creatorTypeNames, sct, dft);
}

//--------------------------------------------------------------------------------------------------------------------

const char * queryScopeTypeName(StatisticScopeType sst)
{
    return scopeTypeNames[sst];
}

extern jlib_decl StatisticScopeType queryScopeType(const char * sst, StatisticScopeType dft)
{
    //MORE: Use a hash table??
    return (StatisticScopeType)matchString(scopeTypeNames, sst, dft);
}

//--------------------------------------------------------------------------------------------------------------------

inline void mergeUpdate(unsigned __int64 & value, const unsigned __int64 otherValue, StatsMergeAction mergeAction)
{
    switch (mergeAction)
    {
    case StatsMergeKeepNonZero:
        if (otherValue && !value)
            value = otherValue;
        break;
    case StatsMergeAppend:
    case StatsMergeReplace:
        value = otherValue;
        break;
    case StatsMergeSum:
        value += otherValue;
        break;
    case StatsMergeMin:
        if (otherValue < value)
            value = otherValue;
        break;
    case StatsMergeFirst:
        if (otherValue && ((otherValue < value) || !value))
            value = otherValue;
        break;
    case StatsMergeMax:
    case StatsMergeLast:
        if (otherValue > value)
            value = otherValue;
        break;
#ifdef _DEBUG
    default:
        throwUnexpected();
#endif
    }
}

unsigned __int64 mergeStatisticValue(unsigned __int64 prevValue, unsigned __int64 newValue, StatsMergeAction mergeAction)
{
    unsigned __int64 value = prevValue;
    mergeUpdate(value, newValue, mergeAction);
    return value;
}

unsigned __int64 mergeStatisticValue(unsigned __int64 prevValue, unsigned __int64 newValue, StatisticKind kind)
{
    return mergeStatisticValue(prevValue, newValue, queryMergeMode(kind));
}

//--------------------------------------------------------------------------------------------------------------------

class CComponentStatistics
{
 protected:
    StringAttr creator;
    byte creatorDepth;
    byte scopeDepth;
//    StatisticArray stats;
};

//--------------------------------------------------------------------------------------------------------------------

//This object is accessed by all StatisticsMapping constructors.  It is important that it is initialised before any of
//them are called.  Coverity routinely complains about possible GLOBAL_INIT_ORDER problems.  These are false positives
//as long as
//   * this definition of allStatsMappings comes first in this file before any StatisticsMappings
//   * there must be no other files in jlib that declare a StatisticMapping.
// StatisticsMappings in other dlls are ok because it is guaranteed that the dependent dll/so is initialised first

static std::unordered_map<unsigned, const StatisticsMapping *> allStatsMappings;

static int compareUnsigned(unsigned const * left, unsigned const * right)
{
    return (*left < *right) ? -1 : (*left > *right) ? +1 : 0;
}

void StatisticsMapping::createMappings()
{
    //Possibly not needed, but sort the kinds, so that it is easy to merge/stream the results out in the correct order.
    indexToKind.sort(compareUnsigned);

    //Provide mappings to all statistics to map them to the "unknown" bin by default
    kindToIndex.ensureCapacity(StMax);
    for (unsigned i=0; i < StMax; i++)
        kindToIndex.append(numStatistics());

    ForEachItemIn(i2, indexToKind)
    {
        unsigned kind = indexToKind.item(i2);
        unsigned existing = kindToIndex.item(kind);
        assertex(existing == numStatistics());  // Throw an error if there are duplicate statistics in the mapping
        kindToIndex.replace(i2, kind);
    }

    const unsigned * kinds = indexToKind.getArray();
    hashcode = hashc((const byte *)kinds, indexToKind.ordinality() * sizeof(unsigned), fnvInitialHash32);

    //All StatisticsMapping objects are assumed to be static, and never destroyed.
    const StatisticsMapping * existing = allStatsMappings[hashcode];
    if (existing)
    {
        //Another mapping object hashes to the same code - we need to re-implement the hash function so it can be used to deserialized nested mappings...
        //Throw a c++ standard exception because this will almost certainly be called from the static initializers, so will not be caught
        if (!equals(*existing))
            throw std::runtime_error("Need to reimplement the hash function for the statistic mappings");
    }
    else
        allStatsMappings[hashcode] = this;
}

bool StatisticsMapping::equals(const StatisticsMapping & other)
{
    if (numStatistics() != other.numStatistics())
        return false;

    ForEachItemIn(i, indexToKind)
    {
        if (indexToKind.item(i) != other.indexToKind.item(i))
            return false;
    }
    return true;
}

const StatisticsMapping noStatistics({});
const StatisticsMapping jhtreeCacheStatistics({ StNumIndexSeeks, StNumIndexScans, StNumPostFiltered, StNumIndexWildSeeks,
                                                StNumNodeCacheAdds, StNumLeafCacheAdds, StNumBlobCacheAdds, StNumNodeCacheHits, StNumLeafCacheHits, StNumBlobCacheHits, StCycleNodeLoadCycles, StCycleLeafLoadCycles,
                                                StCycleBlobLoadCycles, StCycleNodeReadCycles, StCycleLeafReadCycles, StCycleBlobReadCycles, StNumNodeDiskFetches, StNumLeafDiskFetches, StNumBlobDiskFetches,
                                                StCycleNodeFetchCycles, StCycleLeafFetchCycles, StCycleBlobFetchCycles, StCycleIndexCacheBlockedCycles, StNumIndexMergeCompares, StNumIndexMerges, StNumIndexSkips,
                                                StNumBloomAccepts, StNumBloomRejects, StNumBloomSkips,
                                                StNumIndexNullSkips, StTimeLeafLoad, StTimeLeafRead, StTimeLeafFetch, StTimeIndexCacheBlocked, StTimeNodeFetch, StTimeNodeLoad, StTimeNodeRead});

const StatisticsMapping allStatistics(StKindAll);
const StatisticsMapping heapStatistics({StNumAllocations, StNumAllocationScans});
const StatisticsMapping diskLocalStatistics({StCycleDiskReadIOCycles, StSizeDiskRead, StNumDiskReads, StCycleDiskWriteIOCycles, StSizeDiskWrite, StNumDiskWrites, StNumDiskRetries});
const StatisticsMapping diskRemoteStatistics({StTimeDiskReadIO, StSizeDiskRead, StNumDiskReads, StTimeDiskWriteIO, StSizeDiskWrite, StNumDiskWrites, StNumDiskRetries});
const StatisticsMapping diskReadRemoteStatistics({StTimeDiskReadIO, StSizeDiskRead, StNumDiskReads, StNumDiskRetries, StCycleDiskReadIOCycles});
const StatisticsMapping diskWriteRemoteStatistics({StTimeDiskWriteIO, StSizeDiskWrite, StNumDiskWrites, StNumDiskRetries, StCycleDiskWriteIOCycles});
const StatisticsMapping stdAggregateKindStatistics({StCostFileAccess, StSizeGraphSpill, StSizeSpillFile});

const StatisticsMapping * queryStatsMapping(const StatsScopeId & scope, unsigned hashcode)
{
    const StatisticsMapping * match = allStatsMappings[hashcode];
    if (match)
        return match;

    StringBuffer scopeText;
    scope.getScopeText(scopeText);
    throw makeStringExceptionV(0, "No Stats mapping found for scope '%s' hashcode %u", scopeText.str(), hashcode);
}

//--------------------------------------------------------------------------------------------------------------------

StringBuffer & StatisticValueFilter::describe(StringBuffer & out) const
{
    out.append(queryStatisticName(kind));
    if (minValue == maxValue)
        out.append("=").append(minValue);
    else
        out.append("=").append(minValue).append("..").append(maxValue);
    return out;
}
//--------------------------------------------------------------------------------------------------------------------

class Statistic
{
public:
    Statistic(StatisticKind _kind, unsigned __int64 _value) : kind(_kind), value(_value)
    {
    }
    Statistic(MemoryBuffer & in, unsigned version)
    {
        unsigned _kind;
        in.read(_kind);
        kind = (StatisticKind)_kind;
        in.read(value);
    }

    StatisticKind queryKind() const
    {
        return kind;
    }
    unsigned __int64 queryValue() const
    {
        return value;
    }

    void merge(unsigned __int64 otherValue)
    {
        StatsMergeAction mergeAction = queryMergeMode(kind);
        mergeUpdate(value, otherValue, mergeAction);
    }
    void serialize(MemoryBuffer & out) const
    {
        //MORE: Could compress - e.g., store as a packed integers
        out.append((unsigned)kind);
        out.append(value);
    }
    void mergeInto(IStatisticGatherer & target) const
    {
        StatsMergeAction mergeAction = queryMergeMode(kind);
        target.updateStatistic(kind, value, mergeAction);
    }
    StringBuffer & toXML(StringBuffer &out) const
    {
        return out.append("  <Stat name=\"").append(queryStatisticName(kind)).append("\" value=\"").append(value).append("\"/>\n");
    }
public:
    StatisticKind kind;
    unsigned __int64 value;
};

//--------------------------------------------------------------------------------------------------------------------

StringBuffer & StatsScopeId::getScopeText(StringBuffer & out) const
{
    switch (scopeType)
    {
    case SSTgraph:
        return out.append(GraphScopePrefix).append(id);
    case SSTsubgraph:
        return out.append(SubGraphScopePrefix).append(id);
    case SSTactivity:
        return out.append(ActivityScopePrefix).append(id);
    case SSTedge:
        return out.append(EdgeScopePrefix).append(id).append("_").append(extra);
    case SSTfunction:
        return out.append(FunctionScopePrefix).append(name);
    case SSTworkflow:
        return out.append(WorkflowScopePrefix).append(id);
    case SSTchildgraph:
        return out.append(ChildGraphScopePrefix).append(id);
    case SSTfile:
        return out.append(FileScopePrefix).append(name);
    case SSTchannel:
        return out.append(ChannelScopePrefix).append(id);
    case SSTdfuworkunit:
        return out.append(DFUWorkunitScopePrefix).append(name);
    case SSTsection:
        return out.append(SectionScopePrefix).append(name);
    case SSToperation:
        return out.append(OperationScopePrefix).append(name);
    case SSTunknown:
        return out.append(name);
    case SSTglobal:
        return out;
    default:
#ifdef _DEBUG
        throwUnexpected();
#endif
        return out.append("????").append(id);
    }
}

unsigned StatsScopeId::getHash() const
{
    switch (scopeType)
    {
    case SSTfunction:
    case SSTdfuworkunit:
    case SSTfile:
    case SSTsection:
    case SSToperation:
    case SSTunknown:
        return hashcz((const byte *)name.get(), (unsigned)scopeType);
    default:
        return hashc((const byte *)&id, sizeof(id), (unsigned)scopeType);
    }
}

bool StatsScopeId::isWildcard() const
{
    return (id == 0) && (extra == 0) && !name;
}

int StatsScopeId::compare(const StatsScopeId & other) const
{
    if (scopeType != other.scopeType)
        return scopePriority[scopeType] - scopePriority[other.scopeType];
    if (id != other.id)
        return (int)(id - other.id);
    if (extra != other.extra)
        return (int)(extra - other.extra);
    if (name && other.name)
        return strcmp(name, other.name);
    if (name)
        return +1;
    if (other.name)
        return -1;
    return 0;
}

void StatsScopeId::describe(StringBuffer & description) const
{
    const char * name = queryScopeTypeName(scopeType);
    description.append((char)toupper(*name)).append(name+1);
    switch (scopeType)
    {
    case SSTgraph:
        description.append(" graph").append(id);
        break;
    case SSTsubgraph:
    case SSTactivity:
    case SSTworkflow:
    case SSTchildgraph:
    case SSTchannel:
        description.append(' ').append(id);
        break;
    case SSTedge:
        description.append(' ').append(id).append(',').append(extra);
        break;
    case SSTfunction:
    case SSTdfuworkunit:
    case SSTfile:
    case SSTsection:
    case SSToperation:
        description.append(' ').append(name);
        break;
    default:
        throwUnexpected();
        break;
    }

}


bool StatsScopeId::matches(const StatsScopeId & other) const
{
    return (scopeType == other.scopeType) && (id == other.id) && (extra == other.extra) && strsame(name, other.name);
}

unsigned StatsScopeId::queryActivity() const
{
    switch (scopeType)
    {
    case SSTactivity:
    case SSTedge:
        return id;
    default:
        return 0;
    }
}

void StatsScopeId::deserialize(MemoryBuffer & in, unsigned version)
{
    byte scopeTypeByte;
    in.read(scopeTypeByte);
    scopeType = (StatisticScopeType)scopeTypeByte;
    switch (scopeType)
    {
    case SSTgraph:
    case SSTsubgraph:
    case SSTactivity:
    case SSTworkflow:
    case SSTchildgraph:
    case SSTchannel:
        in.read(id);
        break;
    case SSTedge:
        in.read(id);
        in.read(extra);
        break;
    case SSTfunction:
    case SSTdfuworkunit:
    case SSTfile:
    case SSTsection:
    case SSToperation:
        in.read(name);
        break;
    default:
        throwUnexpected();
        break;
    }
}

void StatsScopeId::serialize(MemoryBuffer & out) const
{
    out.append((byte)scopeType);
    switch (scopeType)
    {
    case SSTgraph:
    case SSTsubgraph:
    case SSTactivity:
    case SSTworkflow:
    case SSTchildgraph:
    case SSTchannel:
        out.append(id);
        break;
    case SSTedge:
        out.append(id);
        out.append(extra);
        break;
    case SSTfunction:
    case SSTdfuworkunit:
    case SSTfile:
    case SSTsection:
    case SSToperation:
        out.append(name);
        break;
    default:
        throwUnexpected();
        break;
    }
}

void StatsScopeId::setId(StatisticScopeType _scopeType, unsigned _id, unsigned _extra)
{
    scopeType = _scopeType;
    id = _id;
    extra = _extra;
}

bool StatsScopeId::setScopeText(const char * text, const char * * _next)
{
    auto setScope = [&](StatisticScopeType _scopeType, const char *text, const char * *next)
    {
        if (!*text)
            return false;
        scopeType = _scopeType;
        const char * endScopeNamePtr = strchr(text, ':');
        if (endScopeNamePtr)
        {
            name.set(text, endScopeNamePtr-text);
            if (next)
                * next = endScopeNamePtr;
        }
        else
        {
            name.set(text);
            if (next)
                *next = text + strlen(text);
        }
        return true;
    };
    char * * next = (char * *)_next;
    switch (*text)
    {
    case ActivityScopePrefix[0]:
        if (MATCHES_CONST_PREFIX(text, ActivityScopePrefix))
        {
            if (isdigit(text[strlen(ActivityScopePrefix)]))
            {
                unsigned id = strtoul(text + strlen(ActivityScopePrefix), next, 10);
                setActivityId(id);
                return true;
            }
        }
        break;
    case GraphScopePrefix[0]:
        if (MATCHES_CONST_PREFIX(text, GraphScopePrefix))
        {
            if (isdigit(text[strlen(GraphScopePrefix)]))
            {
                unsigned id = strtoul(text + strlen(GraphScopePrefix), next, 10);
                setId(SSTgraph, id);
                return true;
            }
        }
        break;
    case SubGraphScopePrefix[0]:
        if (MATCHES_CONST_PREFIX(text, SubGraphScopePrefix))
        {
            if (isdigit(text[strlen(SubGraphScopePrefix)]))
            {
                unsigned id = strtoul(text + strlen(SubGraphScopePrefix), next, 10);
                setSubgraphId(id);
                return true;
            }
        }
        break;
    case EdgeScopePrefix[0]:
        if (MATCHES_CONST_PREFIX(text, EdgeScopePrefix))
        {
            const char * underscore = strchr(text, '_');
            if (!underscore || !isdigit(underscore[1]))
                return false;
            unsigned id1 = atoi(text + strlen(EdgeScopePrefix));
            unsigned id2 = strtoul(underscore+1, next, 10);
            setEdgeId(id1, id2);
            return true;
        }
        break;
    case FunctionScopePrefix[0]:
        if (MATCHES_CONST_PREFIX(text, FunctionScopePrefix))
            return setScope(SSTfunction, text+strlen(FunctionScopePrefix), _next);
        break;
    case FileScopePrefix[0]:
        if (MATCHES_CONST_PREFIX(text, FileScopePrefix))
            return setScope(SSTfile, text+strlen(FileScopePrefix), _next);
        break;
    case WorkflowScopePrefix[0]:
        if (MATCHES_CONST_PREFIX(text, WorkflowScopePrefix) && isdigit(text[strlen(WorkflowScopePrefix)]))
        {
            setWorkflowId(strtoul(text+ strlen(WorkflowScopePrefix), next, 10));
            return true;
        }
        break;
    case ChildGraphScopePrefix[0]:
        if (MATCHES_CONST_PREFIX(text, ChildGraphScopePrefix))
        {
            setChildGraphId(strtoul(text+ strlen(ChildGraphScopePrefix), next, 10));
            return true;
        }
        if (MATCHES_CONST_PREFIX(text, "compile"))
        {
            setOperationId("compile");
            return true;
        }
        break;
    case ChannelScopePrefix[0]:
        if (MATCHES_CONST_PREFIX(text, ChannelScopePrefix) && isdigit(text[strlen(ChannelScopePrefix)]))
        {
            setChannelId(strtoul(text+ strlen(ChannelScopePrefix), next, 10));
            return true;
        }
        break;
    case DFUWorkunitScopePrefix[0]:
        if (MATCHES_CONST_PREFIX(text, DFUWorkunitScopePrefix))
            return setScope(SSTdfuworkunit, text+strlen(DFUWorkunitScopePrefix), _next);
        break;
    case SectionScopePrefix[0]:
        if (MATCHES_CONST_PREFIX(text, SectionScopePrefix))
            return setScope(SSTsection, text+strlen(SectionScopePrefix), _next);
        break;
    case OperationScopePrefix[0]:
        if (MATCHES_CONST_PREFIX(text, OperationScopePrefix))
            return setScope(SSToperation, text+strlen(OperationScopePrefix), _next);
        break;
    case '\0':
        setId(SSTglobal, 0);
        return true;
    }
    return false;
}

bool StatsScopeId::extractScopeText(const char * text, const char * * next)
{
    if (setScopeText(text, next))
        return true;

    scopeType = SSTunknown;
    const char * end = strchr(text, ':');
    if (end)
    {
        name.set(text, end-text);
        if (next)
            *next = end;
    }
    else
    {
        name.set(text);
        if (next)
            *next = text + strlen(text);
    }
    return false;
}


void StatsScopeId::setActivityId(unsigned _id)
{
    setId(SSTactivity, _id);
}

void StatsScopeId::setEdgeId(unsigned _id, unsigned _output)
{
    setId(SSTedge, _id, _output);
}
void StatsScopeId::setSubgraphId(unsigned _id)
{
    setId(SSTsubgraph, _id);
}
void StatsScopeId::setFunctionId(const char * _name)
{
    scopeType = SSTfunction;
    name.set(_name);
}
void StatsScopeId::setFileId(const char * _name)
{
    scopeType = SSTfile;
    name.set(_name);
}
void StatsScopeId::setChannelId(unsigned _id)
{
    setId(SSTchannel, _id);
}
void StatsScopeId::setWorkflowId(unsigned _id)
{
    setId(SSTworkflow, _id);
}
void StatsScopeId::setChildGraphId(unsigned _id)
{
    setId(SSTchildgraph, _id);
}
void StatsScopeId::setDfuWorkunitId(const char * _name)
{
    scopeType = SSTdfuworkunit;
    name.set(_name);
}
void StatsScopeId::setSectionId(const char * _name)
{
    scopeType = SSTsection;
    name.set(_name);
}
void StatsScopeId::setOperationId(const char * _name)
{
    scopeType = SSToperation;
    name.set(_name);
}

//--------------------------------------------------------------------------------------------------------------------

enum
{
    SCroot,
    SCintermediate,
    SCleaf,
};

class CStatisticCollection;
static CStatisticCollection * deserializeCollection(CStatisticCollection * parent, MemoryBuffer & in, unsigned version);

//MORE: Create an implementation with no children
typedef StructArrayOf<Statistic> StatsArray;
class CollectionHashTable : public SuperHashTableOf<CStatisticCollection, StatsScopeId>
{
public:
    ~CollectionHashTable() { _releaseAll(); }
    virtual void     onAdd(void *et);
    virtual void     onRemove(void *et);
    virtual unsigned getHashFromElement(const void *et) const;
    virtual unsigned getHashFromFindParam(const void *fp) const;
    virtual const void * getFindParam(const void *et) const;
    virtual bool matchesFindParam(const void *et, const void *key, unsigned fphash) const;
    virtual bool matchesElement(const void *et, const void *searchET) const;
};
typedef IArrayOf<CStatisticCollection> CollectionArray;

static int compareCollection(IInterface * const * pl, IInterface * const *pr);

class SortedCollectionIterator : public ArrayIIteratorOf<IArrayOf<IStatisticCollection>, IStatisticCollection, IStatisticCollectionIterator>
{
    IArrayOf<IStatisticCollection> elems;
public:
    SortedCollectionIterator(IStatisticCollectionIterator &iter) : ArrayIIteratorOf<IArrayOf<IStatisticCollection>, IStatisticCollection, IStatisticCollectionIterator>(elems)
    {
        ForEach(iter)
            elems.append(iter.get());
        elems.sort(compareCollection);
    }
};

class CStatisticCollection : public CInterfaceOf<IStatisticCollection>
{
    friend class CollectionHashTable;

    CStatisticCollection * ensureSubScopePath(std::initializer_list<const StatsScopeId> & path)
    {
        CStatisticCollection * curScope = this;
        for (const auto & scopeItem: path)
            curScope = curScope->ensureSubScope(scopeItem, true); // n.b. this will always return a valid pointer
        return curScope;
    }
    void markDirty()
    {
        isDirty=true;
        if (parent) parent->markDirty();
    }
    void refreshAggregates(CRuntimeStatisticCollection & parentTotals, AggregateUpdatedCallBackFunc & fWhenAggregateUpdated)
    {
        const StatisticsMapping & mapping = parentTotals.queryMapping();
        // if this scope is not dirty, the aggregates are accurate at this level so return totals (no need to descend)
        // Also there is no stats below sg scope, so no need to descend
        if (isDirty==false || id.queryScopeType()==SSTsubgraph)
        {
            // return the aggregates at this scope level
            ForEachItemIn(i, stats)
            {
                Statistic & stat = stats.element(i);
                StatisticKind kind = stat.queryKind();
                if (queryStatsVariant(kind) != 0)
                    continue; // ignore variants (shouldn't happen as the mapping ensure only the aggregator kinds are present)
                if (mapping.hasKind(kind))
                {
                    // Totals required from this level by parent, even if they are not dirty
                    parentTotals.mergeStatistic(kind, stat.queryValue());
                }
            }
            isDirty=false;
            return;
        }
        else
        {
            // descend down to lower level to obtain totals required for aggregation and then aggregate
            CRuntimeStatisticCollection childTotals(mapping);
            for (auto & child : children)
            {
                child.refreshAggregates(childTotals, fWhenAggregateUpdated);
            }
            // 1) Set any values that has changed for this scope and 2) update ALL totals for parent
            const unsigned numStats = mapping.numStatistics();
            for (unsigned i=0; i<numStats; ++i)
            {
                StatisticKind kind = mapping.getKind(i);
                unsigned __int64 value = childTotals.queryStatisticByIndex(i).get();

                if (updateStatistic(kind, value, StatsMergeReplace))
                {
                    if (value || includeStatisticIfZero(kind))
                    {
                        StringBuffer s;
                        fWhenAggregateUpdated(getFullScope(s).str(), queryScopeType(), kind, value);
                    }
                }

                // All totals still required at parent level (even unchanged totals)
                if (value)
                    parentTotals.mergeStatistic(kind, value);
            }
            isDirty=false;
            return;
        }
    }
public:
    CStatisticCollection(CStatisticCollection * _parent, const StatsScopeId & _id) : id(_id), parent(_parent)
    {
    }
    CStatisticCollection(CStatisticCollection * _parent, MemoryBuffer & in, unsigned version) : parent(_parent)
    {
        id.deserialize(in, version);

        unsigned numStats;
        in.read(numStats);
        stats.ensureCapacity(numStats);
        while (numStats-- > 0)
        {
            Statistic next (in, version);
            stats.append(next);
        }

        unsigned numChildren;
        in.read(numChildren);
        children.ensure(numChildren);
        while (numChildren-- > 0)
        {
            CStatisticCollection * next = deserializeCollection(this, in, version);
            children.add(*next);
        }
    }
    virtual byte getCollectionType() const { return SCintermediate; }
//interface IStatisticCollection:
    virtual StringBuffer &toXML(StringBuffer &out) const override;
    virtual StatisticScopeType queryScopeType() const override
    {
        return id.queryScopeType();
    }
    virtual unsigned __int64 queryWhenCreated() const override
    {
        if (parent)
            return parent->queryWhenCreated();
        return 0;
    }
    virtual StringBuffer & getScope(StringBuffer & str) const override
    {
        return id.getScopeText(str);
    }
    virtual StringBuffer & getFullScope(StringBuffer & str) const override
    {
        if (parent)
        {
            parent->getFullScope(str);
            if (!str.isEmpty())
                str.append(':');
        }
        id.getScopeText(str);
        return str;
    }
    virtual unsigned __int64 queryStatistic(StatisticKind kind) const override
    {
        ForEachItemIn(i, stats)
        {
            const Statistic & cur = stats.item(i);
            if (cur.kind == kind)
                return cur.value;
        }
        return 0;
    }
    virtual bool getStatistic(StatisticKind kind, unsigned __int64 & value) const override
    {
        ForEachItemIn(i, stats)
        {
            const Statistic & cur = stats.item(i);
            if (cur.kind == kind)
            {
                value = cur.value;
                return true;
            }
        }
        return false;
    }
    virtual unsigned getNumStatistics() const override
    {
        return stats.ordinality();
    }
    virtual void getStatistic(StatisticKind & kind, unsigned __int64 & value, unsigned idx) const override
    {
        const Statistic & cur = stats.item(idx);
        kind = cur.kind;
        value = cur.value;
    }
    virtual IStatisticCollectionIterator & getScopes(const char * filter, bool sorted) override
    {
        assertex(!filter);
        Owned<IStatisticCollectionIterator> hashIter = new SuperHashIIteratorOf<IStatisticCollection, IStatisticCollectionIterator, false>(children);
        if (!sorted)
            return *hashIter.getClear();
        return * new SortedCollectionIterator(*hashIter);
    }
    virtual void getMinMaxScope(IStringVal & minValue, IStringVal & maxValue, StatisticScopeType searchScopeType) const override
    {
        if (id.queryScopeType() == searchScopeType)
        {
            const char * curMin = minValue.str();
            const char * curMax = maxValue.str();
            StringBuffer name;
            id.getScopeText(name);
            if (!curMin || !*curMin || strcmp(name.str(), curMin) < 0)
                minValue.set(name.str());
            if (!curMax || strcmp(name.str(), curMax) > 0)
                maxValue.set(name.str());
        }

        for (auto & curChild : children)
            curChild.getMinMaxScope(minValue, maxValue, searchScopeType);
    }
    virtual void getMinMaxActivity(unsigned & minValue, unsigned & maxValue) const override
    {
        unsigned activityId = id.queryActivity();
        if (activityId)
        {
            if ((minValue == 0) || (activityId < minValue))
                minValue = activityId;
            if (activityId > maxValue)
                maxValue = activityId;
        }

        SuperHashIteratorOf<CStatisticCollection> iter(children, false);
        for (iter.first(); iter.isValid(); iter.next())
            iter.query().getMinMaxActivity(minValue, maxValue);
    }
    virtual bool setStatistic(const char *scope, StatisticKind kind, unsigned __int64 value) override
    {
        if (*scope=='\0')
        {
            return updateStatistic(kind, value, StatsMergeReplace);
        }
        else
        {
            StatsScopeId childScopeId;
            const char * next;
            if (!childScopeId.setScopeText(scope, &next) || (*next!=':' && *next!='\0'))
                throw makeStringExceptionV(JLIBERR_UnexpectedValue, "'%s' does not appear to be a valid scope id", scope);
            CStatisticCollection * child = ensureSubScope(childScopeId, true);

            if (*next==':')
                next++;
            return child->setStatistic(next, kind, value);
        }
    }

//other public interface functions
    // addStatistic() should only be used if it is known that the kind is not already there
    void addStatistic(StatisticKind kind, unsigned __int64 value)
    {
#ifdef _DEBUG
        // In debug builds check that a value for this kind has not already been added.
        // We do not want the O(N) overhead in release mode, especially as N is beginning to get larger
        unsigned __int64 debugTest;
        assertex(getStatistic(kind,debugTest)==false);
#endif
        Statistic s(kind, value);
        stats.append(s);
    }
    bool updateStatistic(StatisticKind kind, unsigned __int64 value, StatsMergeAction mergeAction)
    {
        if (mergeAction != StatsMergeAppend)
        {
            ForEachItemIn(i, stats)
            {
                Statistic & cur = stats.element(i);
                if (cur.kind == kind)
                {
                    if (mergeAction==StatsMergeReplace)
                    {
                        if (cur.value==value)
                            return false;
                    }
                    cur.value = mergeStatisticValue(cur.value, value, mergeAction);
                    return true;
                }
            }
        }
        Statistic s(kind, value);
        stats.append(s);
        return true;
    }
    CStatisticCollection * ensureSubScope(const StatsScopeId & search, bool hasChildren)
    {
        //Once the CStatisticCollection is created it should not be replaced - so that returned pointers remain valid.
        CStatisticCollection * match = children.find(&search);
        if (match)
            return match;

        CStatisticCollection * ret = new CStatisticCollection(this, search);
        children.add(*ret);
        return ret;
    }
    virtual void serialize(MemoryBuffer & out) const override
    {
        out.append(getCollectionType());
        id.serialize(out);

        out.append(stats.ordinality());
        ForEachItemIn(iStat, stats)
            stats.item(iStat).serialize(out);

        out.append(children.ordinality());
        SuperHashIteratorOf<CStatisticCollection> iter(children, false);
        for (iter.first(); iter.isValid(); iter.next())
            iter.query().serialize(out);
    }
    inline const StatsScopeId & queryScopeId() const { return id; }
    virtual void mergeInto(IStatisticGatherer & target) const
    {
        StatsOptScope block(target, id);
        ForEachItemIn(iStat, stats)
            stats.item(iStat).mergeInto(target);

        for (auto const & cur : children)
            cur.mergeInto(target);
    }
    virtual void visit(IStatisticVisitor & visitor) const
    {
        if (visitor.visitScope(*this))
        {
            for (auto const & cur : children)
                cur.visit(visitor);
        }
    }
    virtual void visitChildren(IStatisticVisitor & visitor) const
    {
        for (auto const & cur : children)
            cur.visit(visitor);
    }
    virtual void refreshAggregates(const StatisticsMapping & mapping, AggregateUpdatedCallBackFunc & fWhenAggregateUpdated) override
    {
        if (isDirty)
        {
            CRuntimeStatisticCollection totals(mapping);
            refreshAggregates(totals, fWhenAggregateUpdated);
        }
    }
    virtual stat_type aggregateStatistic(StatisticKind kind) const override
    {
        stat_type sum = 0;
        if (!getStatistic(kind, sum)) // get sum of statistics at this level
        {
            // if no stats at this level, then get sum of stats from children
            for (auto & child : children)
                sum += child.aggregateStatistic(kind);
        }
        return sum;
    }
    virtual void recordStats(const StatisticsMapping & mapping, IStatisticCollection * sourceStatsCollection, std::initializer_list<const StatsScopeId> path) override
    {
        CStatisticCollection * curSrcCollection = static_cast<CStatisticCollection *>(sourceStatsCollection);
        const StatsScopeId * scopeItem = path.begin();
        // n.b. sourceStatsCollection has workflow as root but this collection has global as root
        // Locate the collection with the stats and make curSrcCollection point to that
        if (!curSrcCollection || curSrcCollection->queryScopeId().compare(*scopeItem)!=0)
            return; // Required path doesn't exist in source collection so nothing more to do here
        ++scopeItem;
        while (scopeItem!=path.end())
        {
            curSrcCollection = curSrcCollection->children.find(scopeItem);
            if (!curSrcCollection)
                return; // Required path doesn't exist in source collection so nothing more to do here
            ++scopeItem;
        }

        CStatisticCollection * tgtScopeCollection = ensureSubScopePath(path);
        bool wasUpdated = false;
        // More efficient to iterate over stats rather than mapping...
        ForEachItemIn(i, curSrcCollection->stats)
        {
            Statistic & cur = curSrcCollection->stats.element(i);
            if (queryStatsVariant(cur.kind) != 0)
                continue; // ignore variants
            if (mapping.hasKind(cur.kind))
            {
                if (tgtScopeCollection->updateStatistic(cur.kind, cur.value, StatsMergeReplace))
                    wasUpdated=true;
            }
        }
        if (wasUpdated)
            tgtScopeCollection->markDirty();
    }
private:
    StatsScopeId id;
    CStatisticCollection * parent;
protected:
    CollectionHashTable children;
    StatsArray stats;
    bool isDirty = false; // used to track which scope has changed (used to workout what aggregates to recalculate)
};

StringBuffer &CStatisticCollection::toXML(StringBuffer &out) const
{
    out.append("<Scope id=\"");
    id.getScopeText(out).append("\">\n");
    if (stats.ordinality())
    {
        out.append(" <Stats>");
        ForEachItemIn(i, stats)
            stats.item(i).toXML(out);
        out.append(" </Stats>\n");
    }

    SuperHashIteratorOf<CStatisticCollection> iter(children, false);
    for (iter.first(); iter.isValid(); iter.next())
        iter.query().toXML(out);
    out.append("</Scope>\n");
    return out;
}

static int compareCollection(IInterface * const * pl, IInterface * const *pr)
{
    CStatisticCollection * l = static_cast<CStatisticCollection *>(static_cast<IStatisticCollection *>(*pl));
    CStatisticCollection * r = static_cast<CStatisticCollection *>(static_cast<IStatisticCollection *>(*pr));
    return l->queryScopeId().compare(r->queryScopeId());
}

//---------------------------------------------------------------------------------------------------------------------

void CollectionHashTable::onAdd(void *et)
{
}
void CollectionHashTable::onRemove(void *et)
{
    CStatisticCollection * elem = reinterpret_cast<CStatisticCollection *>(et);
    elem->Release();
}
unsigned CollectionHashTable::getHashFromElement(const void *et) const
{
    const CStatisticCollection * elem = reinterpret_cast<const CStatisticCollection *>(et);
    return elem->id.getHash();
}
unsigned CollectionHashTable::getHashFromFindParam(const void *fp) const
{
    const StatsScopeId * search = reinterpret_cast<const StatsScopeId *>(fp);
    return search->getHash();
}
const void * CollectionHashTable::getFindParam(const void *et) const
{
    const CStatisticCollection * elem = reinterpret_cast<const CStatisticCollection *>(et);
    return &elem->id;
}
bool CollectionHashTable::matchesFindParam(const void *et, const void *key, unsigned fphash) const
{
    const CStatisticCollection * elem = reinterpret_cast<const CStatisticCollection *>(et);
    const StatsScopeId * search = reinterpret_cast<const StatsScopeId *>(key);
    return elem->id.matches(*search);
}
bool CollectionHashTable::matchesElement(const void *et, const void *searchET) const
{
    const CStatisticCollection * elem = reinterpret_cast<const CStatisticCollection *>(et);
    const CStatisticCollection * searchElem = reinterpret_cast<const CStatisticCollection *>(searchET);
    return elem->id.matches(searchElem->id);
}

//---------------------------------------------------------------------------------------------------------------------

class CRootStatisticCollection : public CStatisticCollection
{
public:
    CRootStatisticCollection(StatisticCreatorType _creatorType, const char * _creator, const StatsScopeId & _id)
        : CStatisticCollection(NULL, _id), creatorType(_creatorType), creator(_creator)
    {
        whenCreated = getTimeStampNowValue();
    }
    CRootStatisticCollection(MemoryBuffer & in, unsigned version) : CStatisticCollection(NULL, in, version)
    {
        byte creatorTypeByte;
        in.read(creatorTypeByte);
        creatorType = (StatisticCreatorType)creatorTypeByte;
        in.read(creator);
        in.read(whenCreated);
    }

    virtual byte getCollectionType() const { return SCroot; }

    virtual unsigned __int64 queryWhenCreated() const
    {
        return whenCreated;
    }
    virtual void serialize(MemoryBuffer & out) const
    {
        CStatisticCollection::serialize(out);
        out.append((byte)creatorType);
        out.append(creator);
        out.append(whenCreated);
    }
    virtual void mergeInto(IStatisticGatherer & target) const override
    {
        // Similar to CStatisticCollection::mergeInfo but do not add the root scope.
        ForEachItemIn(iStat, stats)
            stats.item(iStat).mergeInto(target);

        for (auto const & cur : children)
            cur.mergeInto(target);
    }
public:
    StatisticCreatorType creatorType;
    StringAttr creator;
    unsigned __int64 whenCreated;
};

//---------------------------------------------------------------------------------------------------------------------

void serializeStatisticCollection(MemoryBuffer & out, IStatisticCollection * collection)
{
    out.append(currentStatisticsVersion);
    collection->serialize(out);
}

static CStatisticCollection * deserializeCollection(CStatisticCollection * parent, MemoryBuffer & in, unsigned version)
{
    byte kind;
    in.read(kind);
    switch (kind)
    {
    case SCroot:
        assertex(!parent);
        return new CRootStatisticCollection(in, version);
    case SCintermediate:
        return new CStatisticCollection(parent, in, version);
    default:
        UNIMPLEMENTED;
    }
}

IStatisticCollection * createStatisticCollection(MemoryBuffer & in)
{
    unsigned version;
    in.read(version);
    return deserializeCollection(NULL, in, version);
}

IStatisticCollection * createStatisticCollection(const StatsScopeId & scopeId)
{
    return new CStatisticCollection(nullptr, scopeId);
}


//--------------------------------------------------------------------------------------------------------------------

class StatisticGatherer : implements CInterfaceOf<IStatisticGatherer>
{
public:
    StatisticGatherer(CStatisticCollection * scope) : rootScope(scope)
    {
        scopes.append(*scope);
    }
    virtual void beginScope(const StatsScopeId & id) override
    {
        CStatisticCollection & tos = scopes.tos();
        scopes.append(*tos.ensureSubScope(id, true));
    }
    virtual void beginActivityScope(unsigned id) override
    {
        StatsScopeId scopeId(SSTactivity, id);
        CStatisticCollection & tos = scopes.tos();
        scopes.append(*tos.ensureSubScope(scopeId, false));
    }
    virtual void beginSubGraphScope(unsigned id) override
    {
        StatsScopeId scopeId(SSTsubgraph, id);
        CStatisticCollection & tos = scopes.tos();
        scopes.append(*tos.ensureSubScope(scopeId, true));
    }
    virtual void beginEdgeScope(unsigned id, unsigned oid) override
    {
        StatsScopeId scopeId(SSTedge, id, oid);
        CStatisticCollection & tos = scopes.tos();
        scopes.append(*tos.ensureSubScope(scopeId, false));
    }
    virtual void beginChildGraphScope(unsigned id) override
    {
        StatsScopeId scopeId(SSTchildgraph, id);
        CStatisticCollection & tos = scopes.tos();
        scopes.append(*tos.ensureSubScope(scopeId, true));
    }
    virtual void beginChannelScope(unsigned id) override
    {
        StatsScopeId scopeId(SSTchannel, id);
        CStatisticCollection & tos = scopes.tos();
        scopes.append(*tos.ensureSubScope(scopeId, true));
    }
    virtual void endScope() override
    {
        scopes.pop();
    }
    virtual void addStatistic(StatisticKind kind, unsigned __int64 value) override
    {
        CStatisticCollection & tos = scopes.tos();
        tos.addStatistic(kind, value);
    }
    virtual void updateStatistic(StatisticKind kind, unsigned __int64 value, StatsMergeAction mergeAction) override
    {
        CStatisticCollection & tos = scopes.tos();
        tos.updateStatistic(kind, value, mergeAction);
    }
    virtual IStatisticCollection * getResult() override
    {
        return LINK(rootScope);
    }

protected:
    ICopyArrayOf<CStatisticCollection> scopes;
    Linked<CStatisticCollection> rootScope;
};

extern IStatisticGatherer * createStatisticsGatherer(StatisticCreatorType creatorType, const char * creator, const StatsScopeId & rootScope)
{
    //creator unused at the moment.
    Owned<CStatisticCollection> rootCollection = new CRootStatisticCollection(creatorType, creator, rootScope);
    return new StatisticGatherer(rootCollection);
}

//--------------------------------------------------------------------------------------------------------------------

extern IPropertyTree * selectTreeStat(IPropertyTree *node, const char *statName, const char *statType)
{
    StringBuffer xpath;
    xpath.appendf("att[@name='%s']", statName);
    IPropertyTree *att = node->queryPropTree(xpath.str());
    if (!att)
    {
        att = node->addPropTree("att", createPTree());
        att->setProp("@name", statName);
        att->setProp("@type", statType);
    }
    return att;
}

extern void putStatsTreeValue(IPropertyTree *node, const char *statName, const char *statType, unsigned __int64 val)
{
    if (val)
        selectTreeStat(node, statName, statType)->setPropInt64("@value", val);
}

class TreeNodeStatisticGatherer : public CInterfaceOf<IStatisticGatherer>
{
public:
    TreeNodeStatisticGatherer(IPropertyTree & root) : node(&root) {}

    virtual void beginScope(const StatsScopeId & id)
    {
        StringBuffer temp;
        id.getScopeText(temp);
        beginScope(temp.str());
    }
    virtual void beginSubGraphScope(unsigned id) { throwUnexpected(); }
    virtual void beginChildGraphScope(unsigned id) { throwUnexpected(); }
    virtual void beginActivityScope(unsigned id) { throwUnexpected(); }
    virtual void beginEdgeScope(unsigned id, unsigned oid) { throwUnexpected(); }
    virtual void beginChannelScope(unsigned id) { throwUnexpected(); }
    virtual void endScope()
    {
        node = &stack.popGet();
    }
    virtual void addStatistic(StatisticKind kind, unsigned __int64 value)
    {
        putStatsTreeValue(node, queryStatisticName(kind), "sum", value);
    }

    virtual void updateStatistic(StatisticKind kind, unsigned __int64 value, StatsMergeAction mergeAction)
    {
        if (value)
        {
            IPropertyTree * stat = selectTreeStat(node, queryStatisticName(kind), "sum");
            unsigned __int64 newValue = mergeStatisticValue(stat->getPropInt64("@value"), value, mergeAction);
            stat->setPropInt64("@value", newValue);
        }
    }

    virtual IStatisticCollection * getResult() { throwUnexpected(); }

protected:
    void beginScope(const char * id)
    {
        stack.append(*node);

        StringBuffer xpath;
        xpath.appendf("scope[@name='%s']", id);
        IPropertyTree *att = node->queryPropTree(xpath.str());
        if (!att)
        {
            att = node->addPropTree("scope", createPTree());
            att->setProp("@name", id);
        }
        node = att;
    }

protected:
    IPropertyTree * node;
    ICopyArrayOf<IPropertyTree> stack;
};

//--------------------------------------------------------------------------------------------------------------------

void CRuntimeStatistic::merge(unsigned __int64 otherValue, StatsMergeAction mergeAction)
{
    switch (mergeAction)
    {
    case StatsMergeKeepNonZero:
        if (otherValue && !value)
        {
            unsigned __int64 zero = 0;
            value.compare_exchange_strong(zero, otherValue);
        }
        break;
    case StatsMergeAppend:
    case StatsMergeReplace:
        value = otherValue;
        break;
    case StatsMergeSum:
        if (otherValue)
            addAtomic(otherValue);
        break;
    case StatsMergeMin:
        value.store_min(otherValue);
        break;
    case StatsMergeFirst:
        value.store_first(otherValue);
        break;
    case StatsMergeLast:
    case StatsMergeMax:
        value.store_max(otherValue);
        break;
    default:
#ifdef _DEBUG
        throwUnexpected();
#else
        value = otherValue;
#endif
    }
}

//--------------------------------------------------------------------------------------------------------------------

CRuntimeStatisticCollection::CRuntimeStatisticCollection(const CRuntimeStatisticCollection & _other) : mapping(_other.mapping)
#ifdef _DEBUG
, ignoreUnknown(_other.ignoreUnknown)
#endif
{
    unsigned num = mapping.numStatistics();
    values = new CRuntimeStatistic[num+1];
    for (unsigned i=0; i <= num; i++)
        values[i].set(_other.values[i].get());

    CNestedRuntimeStatisticMap *otherNested = _other.queryNested();
    if (otherNested)
    {
        nested = createNested();
        queryNested()->set(*otherNested, 0);
    }
}

CRuntimeStatisticCollection::~CRuntimeStatisticCollection()
{
    delete [] values;
    delete queryNested();
}

CNestedRuntimeStatisticMap * CRuntimeStatisticCollection::queryNested() const
{
    return nested.load(std::memory_order_relaxed);
}

CNestedRuntimeStatisticMap * CRuntimeStatisticCollection::createNested() const
{
    return new CNestedRuntimeStatisticMap;
}

CNestedRuntimeStatisticMap & CRuntimeStatisticCollection::ensureNested()
{
    return *querySingleton(nested, nestlock, [this]{ return this->createNested(); });
}

CriticalSection CRuntimeStatisticCollection::nestlock;

unsigned __int64 CRuntimeStatisticCollection::getSerialStatisticValue(StatisticKind kind) const
{
    unsigned __int64 value = getStatisticValue(kind);
    StatisticKind rawKind= queryRawKind(kind);
    if (kind == rawKind)
        return value;
    unsigned __int64 rawValue = getStatisticValue(rawKind);
    return value + convertMeasure(rawKind, kind, rawValue);
}

void CRuntimeStatisticCollection::set(const CRuntimeStatisticCollection & other, unsigned node)
{
    ForEachItemIn(i, other)
    {
        StatisticKind kind = other.getKind(i);
        unsigned __int64 value = other.getStatisticValue(kind);
        setStatistic(kind, value, node);
    }

    CNestedRuntimeStatisticMap *otherNested = other.queryNested();
    if (otherNested)
    {
        ensureNested().set(*otherNested, node);
    }
}

void CRuntimeStatisticCollection::merge(const CRuntimeStatisticCollection & other, unsigned node)
{
    ForEachItemIn(i, other)
    {
        StatisticKind kind = other.getKind(i);
        unsigned __int64 value = other.queryStatisticByIndex(i).get();
        mergeStatistic(kind, value, node);
    }

    CNestedRuntimeStatisticMap *otherNested = other.queryNested();
    if (otherNested)
    {
        ensureNested().merge(*otherNested, node);
    }
}

void CRuntimeStatisticCollection::updateDelta(CRuntimeStatisticCollection & target, const CRuntimeStatisticCollection & source)
{
    ForEachItemIn(i, source)
    {
        StatisticKind kind = source.getKind(i);
        unsigned __int64 sourceValue = source.getStatisticValue(kind);
        if (queryMergeMode(kind) == StatsMergeSum)
        {
            unsigned __int64 prevValue = getStatisticValue(kind);
            if (sourceValue != prevValue)
            {
                target.mergeStatistic(kind, sourceValue - prevValue);
                setStatistic(kind, sourceValue);
            }
        }
        else
        {
            if (sourceValue)
                target.mergeStatistic(kind, sourceValue);
        }
    }
    CNestedRuntimeStatisticMap *sourceNested = source.queryNested();
    if (sourceNested)
    {
        ensureNested().updateDelta(target.ensureNested(), *sourceNested);
    }
}

void CRuntimeStatisticCollection::mergeStatistic(StatisticKind kind, unsigned __int64 value)
{
    CRuntimeStatistic * target = queryOptStatistic(kind);
    if (target)
    {
        target->merge(value, queryMergeMode(kind));
    }
    else
    {
        StatisticKind serialKind= querySerializedKind(kind);
        if (kind != serialKind)
            mergeStatistic(serialKind, convertMeasure(kind, serialKind, value));
    }
}

void CRuntimeStatisticCollection::sumStatistic(StatisticKind kind, unsigned __int64 value)
{
    queryStatistic(kind).sum(value);
}

void CRuntimeStatisticCollection::mergeStatistic(StatisticKind kind, unsigned __int64 value, unsigned node)
{
    mergeStatistic(kind, value);
}

void CRuntimeStatisticCollection::setStatistic(StatisticKind kind, unsigned __int64 value, unsigned node)
{
    setStatistic(kind, value);
}

void CRuntimeStatisticCollection::reset()
{
    unsigned num = mapping.numStatistics();
    for (unsigned i = 0; i <= num; i++)
        values[i].clear();
}

void CRuntimeStatisticCollection::reset(const StatisticsMapping & toClear)
{
    unsigned num = toClear.numStatistics();
    for (unsigned i = 0; i < num; i++)
        queryStatistic(toClear.getKind(i)).clear();
}

CRuntimeStatisticCollection & CRuntimeStatisticCollection::registerNested(const StatsScopeId & scope, const StatisticsMapping & mapping)
{
    return ensureNested().addNested(scope, mapping).queryStats();
}

void CRuntimeStatisticCollection::recordStatistics(IStatisticGatherer & target, bool clear) const
{
    ForEachItem(i)
    {
        StatisticKind kind = getKind(i);
        unsigned __int64 value = clear ? values[i].getClearAtomic() : values[i].get();
        if (value || includeStatisticIfZero(kind))
        {
            StatisticKind serialKind= querySerializedKind(kind);
            value = convertMeasure(kind, serialKind, value);

            StatsMergeAction mergeAction = queryMergeMode(serialKind);
            target.updateStatistic(serialKind, value, mergeAction);
        }
    }
    reportIgnoredStats();
    CNestedRuntimeStatisticMap *qn = queryNested();
    if (qn)
        qn->recordStatistics(target, clear);
}

void CRuntimeStatisticCollection::reportIgnoredStats() const
{
    if (values[mapping.numStatistics()].getClear())
        DBGLOG("Some statistics were added but thrown away");
}

StringBuffer & CRuntimeStatisticCollection::toXML(StringBuffer &str) const
{
    ForEachItem(iStat)
    {
        unsigned __int64 value = values[iStat].get();
        if (value)
        {
            StatisticKind kind = getKind(iStat);
            const char * name = queryStatisticName(kind);
            str.appendf("<%s>%" I64F "d</%s>", name, value, name);
        }
    }
    CNestedRuntimeStatisticMap *qn = queryNested();
    if (qn)
        qn->toXML(str);
    return str;
}

StringBuffer & CRuntimeStatisticCollection::toStr(StringBuffer &str) const
{
    ForEachItem(iStat)
    {
        StatisticKind kind = getKind(iStat);
        StatisticKind serialKind = querySerializedKind(kind);
        if (kind != serialKind)
            continue; // ignore - we will roll this one into the corresponding serialized value's output
        unsigned __int64 value = values[iStat].get();
        StatisticKind rawKind = queryRawKind(kind);
        if (kind != rawKind)
        {
            // roll raw values into the corresponding serialized value, if present...
            unsigned __int64 rawValue = getStatisticValue(rawKind);
            if (rawValue)
                value += convertMeasure(rawKind, kind, rawValue);
        }
        if (value)
        {
            const char * name = queryStatisticName(serialKind);
            str.append(' ').append(name).append("=");
            formatStatistic(str, value, serialKind);
        }
    }
    CNestedRuntimeStatisticMap *qn = queryNested();
    if (qn)
        qn->toStr(str);
    return str;
}

//MORE: This could be commoned up with the toStr() method by using a visitor pattern
void CRuntimeStatisticCollection::exportToSpan(ISpan * span, StringBuffer & prefix) const
{
    unsigned lenPrefix = prefix.length();
    ForEachItem(iStat)
    {
        StatisticKind kind = getKind(iStat);
        StatisticKind serialKind = querySerializedKind(kind);
        if (kind != serialKind)
            continue; // ignore - we will roll this one into the corresponding serialized value's output
        unsigned __int64 value = values[iStat].get();
        StatisticKind rawKind = queryRawKind(kind);
        if (kind != rawKind)
        {
            // roll raw values into the corresponding serialized value, if present...
            unsigned __int64 rawValue = getStatisticValue(rawKind);
            if (rawValue)
                value += convertMeasure(rawKind, kind, rawValue);
        }
        if (value)
        {
            //Convert timestamp to nanoseconds so it is reported consistently.
            if (queryMeasure(serialKind) == SMeasureTimestampUs)
                value = normalizeTimestampToNs(value);

            const char * name = queryStatisticName(serialKind);
            getSnakeCase(prefix, name);
            span->setSpanAttribute(prefix, value);
            prefix.setLength(lenPrefix);
        }
    }
    CNestedRuntimeStatisticMap *qn = queryNested();
    if (qn)
        qn->exportToSpan(span, prefix);
}

void CRuntimeStatisticCollection::deserialize(MemoryBuffer& in)
{
    unsigned numValid;
    in.readPacked(numValid);
    for (unsigned i=0; i < numValid; i++)
    {
        unsigned kindVal;
        unsigned __int64 value;
        in.readPacked(kindVal).readPacked(value);
        StatisticKind kind = (StatisticKind)kindVal;
        setStatistic(kind, value);
    }
    bool hasNested;
    in.read(hasNested);
    if (hasNested)
    {
        ensureNested().deserialize(in);
    }
}

void CRuntimeStatisticCollection::deserializeMerge(MemoryBuffer& in)
{
    unsigned numValid;
    in.readPacked(numValid);
    for (unsigned i=0; i < numValid; i++)
    {
        unsigned kindVal;
        unsigned __int64 value;
        in.readPacked(kindVal).readPacked(value);
        StatisticKind kind = (StatisticKind)kindVal;
        StatsMergeAction mergeAction = queryMergeMode(kind);
        mergeStatistic(kind, value, mergeAction);
    }
    bool hasNested;
    in.read(hasNested);
    if (hasNested)
    {
        ensureNested().deserializeMerge(in);
    }
}

bool CRuntimeStatisticCollection::serialize(MemoryBuffer& out) const
{
    unsigned numValid = 0;
    ForEachItem(i1)
    {
        if (values[i1].get())
            numValid++;
    }

    out.appendPacked(numValid);
    ForEachItem(i2)
    {
        unsigned __int64 value = values[i2].get();
        if (value)
        {
            StatisticKind kind = mapping.getKind(i2);
            StatisticKind serialKind= querySerializedKind(kind);
            if (kind != serialKind)
                value = convertMeasure(kind, serialKind, value);

            out.appendPacked((unsigned)serialKind);
            out.appendPacked(value);
        }
    }

    bool nonEmpty = (numValid != 0);
    CNestedRuntimeStatisticMap *qn = queryNested();
    out.append(qn != nullptr);
    if (qn)
    {
        if (qn->serialize(out))
            nonEmpty = true;
    }
    return nonEmpty;
}


//---------------------------------------------------------------------------------------------------------------------

void CRuntimeSummaryStatisticCollection::DerivedStats::mergeStatistic(unsigned __int64 value, unsigned node)
{
    if (count == 0)
    {
        min = value;
        max = value;
        minNode = node;
        maxNode = node;
    }
    else
    {
        if (value < min)
        {
            min = value;
            minNode = node;
        }
        if (value > max)
        {
            max = value;
            maxNode = node;
        }
    }
    count++;
    sum += value;
    double dvalue = (double)value;
    sumSquares += dvalue * dvalue;
}

void CRuntimeSummaryStatisticCollection::DerivedStats::setStatistic(unsigned __int64 value, unsigned node)
{
    if (count == 0)
    {
        min = value;
        max = value;
        minNode = node;
        maxNode = node;
    }
    else
    {
        if (value < min)
        {
            min = value;
            minNode = node;
        }
        if (value > max)
        {
            max = value;
            maxNode = node;
        }
    }
    count++;
    sum = value;
    double dvalue = (double)value;
    sumSquares = dvalue * dvalue;
}

double CRuntimeSummaryStatisticCollection::DerivedStats::queryStdDevInfo(unsigned __int64 &_min, unsigned __int64 &_max, unsigned &_minNode, unsigned &_maxNode) const
{
    _min = min;
    _max = max;
    _minNode = minNode;
    _maxNode = maxNode;
    double mean = sum / count;
    double variance = (sumSquares - sum * mean) / count;
    return sqrt(variance);
}

CRuntimeSummaryStatisticCollection::CRuntimeSummaryStatisticCollection(const StatisticsMapping & _mapping) : CRuntimeStatisticCollection(_mapping)
{
    derived = new DerivedStats[ordinality()+1];
}

CRuntimeSummaryStatisticCollection::~CRuntimeSummaryStatisticCollection()
{
    delete[] derived;
}

CNestedRuntimeStatisticMap * CRuntimeSummaryStatisticCollection::createNested() const
{
    return new CNestedRuntimeSummaryStatisticMap;
}

void CRuntimeSummaryStatisticCollection::mergeStatistic(StatisticKind kind, unsigned __int64 value, unsigned node)
{
    CRuntimeStatisticCollection::mergeStatistic(kind, value);
    unsigned index = queryMapping().getIndex(kind);
    derived[index].mergeStatistic(value, node);
}

void CRuntimeSummaryStatisticCollection::setStatistic(StatisticKind kind, unsigned __int64 value, unsigned node)
{
    CRuntimeStatisticCollection::setStatistic(kind, value);
    unsigned index = queryMapping().getIndex(kind);
    derived[index].setStatistic(value, node);
}

double CRuntimeSummaryStatisticCollection::queryStdDevInfo(StatisticKind kind, unsigned __int64 &_min, unsigned __int64 &_max, unsigned &_minNode, unsigned &_maxNode) const
{
    return derived[queryMapping().getIndex(kind)].queryStdDevInfo(_min, _max, _minNode, _maxNode);
}

static bool skewHasMeaning(StatisticKind kind)
{
    //Check that skew makes any sense for the type of measurement
    switch (queryMeasure(kind))
    {
    case SMeasureTimeNs:
    case SMeasureCount:
    case SMeasureSize:
        return true;
    default:
        return false;
    }
}

static bool isSignificantRange(StatisticKind kind, unsigned __int64 range, unsigned __int64 mean)
{
    //Ignore tiny differences (often occur with counts of single rows on 1 slave node)
    unsigned insignificantDiff = 1;
    switch (queryMeasure(kind))
    {
    case SMeasureTimestampUs:
        insignificantDiff = 1000;       // Ignore 1ms timestamp difference between nodes
        break;
    case SMeasureTimeNs:
        insignificantDiff = 1000;       // Ignore 1us timing difference between nodes
        break;
    case SMeasureSize:
        insignificantDiff = 1024;
        break;
    }
    if (range <= insignificantDiff)
        return false;

    if (queryMergeMode(kind) == StatsMergeSum)
    {
        //if the range is < 0.01% of the mean, then it is unlikely to be interesting
        if (range * 10000 < mean)
            return false;
    }

    return true;
}

static bool isWorthReportingMergedValue(StatisticKind kind)
{
    switch (queryMergeMode(kind))
    {
    //Does the merged value have a meaning?
    case StatsMergeSum:
    case StatsMergeMin:
    case StatsMergeMax:
    case StatsMergeFirst:
    case StatsMergeLast:
        break;
    default:
        return false;
    }

    switch (queryMeasure(kind))
    {
    case SMeasureTimeNs:
        //Not generally worth reporting the total time across all slaves
        return false;
    }

    switch (kind)
    {
    case StSizePeakMemory:
    case StSizePeakRowMemory:
    case StNumMatchLeftRowsMax:
    case StNumMatchRightRowsMax:
    case StNumMatchCandidatesMax:
    case StSizeLargestExpandedLeaf:
        //These only make sense for individual nodes, the aggregated value is meaningless
        return false;
    }

    return true;
}


void CRuntimeSummaryStatisticCollection::recordStatistics(IStatisticGatherer & target, bool clear) const
{
    for (unsigned i = 0; i < ordinality(); i++)
    {
        DerivedStats & cur = derived[i];
        StatisticKind kind = getKind(i);
        StatisticKind serialKind = querySerializedKind(kind);
        if (cur.count)
        {
            //Thor should always publish the average value for a stat, and the merged value if it makes sense.
            //So that it is easy to analyse graphs independent of the number of slave nodes it is executed on.

            unsigned __int64 mergedValue = convertMeasure(kind, serialKind, clear ? values[i].getClearAtomic() : values[i].get());
            if (isWorthReportingMergedValue(serialKind))
            {
                if (mergedValue || includeStatisticIfZero(serialKind))
                    target.addStatistic(serialKind, mergedValue);
            }

            unsigned __int64 minValue = convertMeasure(kind, serialKind, cur.min);
            unsigned __int64 maxValue = convertMeasure(kind, serialKind, cur.max);
            if (minValue != maxValue)
            {
                //Avoid rounding errors summing values as doubles - if they were also summed as integers.  Probably overkill!
                //There may still be noticeable rounding errors with timestamps...  revisit if it is an issue with any measurement
                double sum = (queryMergeMode(kind) == StatsMergeSum) ? (double)mergedValue : convertSumMeasure(kind, serialKind, cur.sum);
                double mean = (double)(sum / cur.count);
                unsigned __int64 range = maxValue - minValue;

                target.addStatistic((StatisticKind)(serialKind|StAvgX), (unsigned __int64)mean);
                target.addStatistic((StatisticKind)(serialKind|StMinX), minValue);
                target.addStatistic((StatisticKind)(serialKind|StMaxX), maxValue);

                //Exclude delta and std dev if a single node was the only one that provided a value
                if ((minValue != 0) || (maxValue != mergedValue))
                {
                    //The delta/std dev may have a different unit from the original values e.g., timestamps->times, so needs scaling
                    unsigned __int64 scaledRange = convertMeasure(serialKind, serialKind|StDeltaX, range);
                    target.addStatistic(serialKind|StDeltaX, scaledRange);

                    if (skewHasMeaning(serialKind))
                    {
                        //Sum of squares needs to be translated twice
                        double sumSquares = convertSquareMeasure(kind, serialKind, cur.sumSquares);
                        double variance = (sumSquares - sum * mean) / cur.count;
                        double stdDev = sqrt(variance);
                        unsigned __int64 scaledStdDev = convertMeasure(serialKind, serialKind|StStdDevX, stdDev);
                        target.addStatistic(serialKind|StStdDevX, scaledStdDev);
                    }
                }

                //First test is redundant - but protects against minValue != maxValue test above changing.
                if ((cur.minNode != cur.maxNode) && isSignificantRange(serialKind, range, mean))
                {
                    target.addStatistic((StatisticKind)(serialKind|StNodeMin), cur.minNode+1);
                    target.addStatistic((StatisticKind)(serialKind|StNodeMax), cur.maxNode+1);

                    if (skewHasMeaning(serialKind))
                    {
                        double maxSkew = (10000.0 * ((maxValue-mean)/mean));
                        double minSkew = (10000.0 * ((mean-minValue)/mean));
                        target.addStatistic((StatisticKind)(serialKind|StSkewMin), (unsigned __int64)minSkew);
                        target.addStatistic((StatisticKind)(serialKind|StSkewMax), (unsigned __int64)maxSkew);
                    }
                }
            }
            else
            {
                if (minValue || includeStatisticIfZero(serialKind))
                    target.addStatistic((StatisticKind)(serialKind|StAvgX), minValue);
            }
        }
        else
        {
            //No results received from any of the slave yet... so do not report any stats
        }
    }

    reportIgnoredStats();
    CNestedRuntimeStatisticMap *qn = queryNested();
    if (qn)
        qn->recordStatistics(target, clear);
}

bool CRuntimeSummaryStatisticCollection::serialize(MemoryBuffer & out) const
{
    UNIMPLEMENTED; // NB: Need to convert sum squares twice.
}

void CRuntimeSummaryStatisticCollection::deserialize(MemoryBuffer & in)
{
    UNIMPLEMENTED;
}

void CRuntimeSummaryStatisticCollection::deserializeMerge(MemoryBuffer& in)
{
    UNIMPLEMENTED;
}

//---------------------------------------------------

bool CNestedRuntimeStatisticCollection::matches(const StatsScopeId & otherScope) const
{
    return scope.matches(otherScope);
}

//NOTE: When deserializing, the scope is deserialized by the caller, and the correct target selected
//which is why there is no corresponding deserialize() ofthe scope at this point
void CNestedRuntimeStatisticCollection::deserialize(MemoryBuffer & in)
{
    stats->deserialize(in);
}

void CNestedRuntimeStatisticCollection::deserializeMerge(MemoryBuffer& in)
{
    stats->deserializeMerge(in);
}

void CNestedRuntimeStatisticCollection::merge(const CNestedRuntimeStatisticCollection & other, unsigned node)
{
    stats->merge(other.queryStats(), node);
}

void CNestedRuntimeStatisticCollection::set(const CNestedRuntimeStatisticCollection & other, unsigned node)
{
    stats->set(other.queryStats(), node);
}

bool CNestedRuntimeStatisticCollection::serialize(MemoryBuffer& out) const
{
    scope.serialize(out);
    out.append(stats->queryMapping().getUniqueHash());
    return stats->serialize(out);
}

void CNestedRuntimeStatisticCollection::recordStatistics(IStatisticGatherer & target, bool clear) const
{
    target.beginScope(scope);
    stats->recordStatistics(target, clear);
    target.endScope();
}

StringBuffer & CNestedRuntimeStatisticCollection::toStr(StringBuffer &str) const
{
    str.append(' ');
    scope.getScopeText(str).append("={");
    stats->toStr(str);
    return str.append(" }");
}

void CNestedRuntimeStatisticCollection::exportToSpan(ISpan * span, StringBuffer & prefix) const
{
    unsigned lenPrefix = prefix.length();
    scope.getScopeText(prefix).append(".");
    stats->exportToSpan(span, prefix);
    prefix.setLength(lenPrefix);
}

StringBuffer & CNestedRuntimeStatisticCollection::toXML(StringBuffer &str) const
{
    str.append("<Scope id=\"");
    scope.getScopeText(str).append("\">");
    stats->toXML(str);
    return str.append("</Scope>");
}

void CNestedRuntimeStatisticCollection::updateDelta(CNestedRuntimeStatisticCollection & target, const CNestedRuntimeStatisticCollection & source)
{
    stats->updateDelta(*target.stats, *source.stats);
}

//---------------------------------------------------

CNestedRuntimeStatisticCollection & CNestedRuntimeStatisticMap::addNested(const StatsScopeId & scope, const StatisticsMapping & mapping)
{
    unsigned mapSize;
    unsigned entry;
    {
        ReadLockBlock b(lock);
        mapSize = map.length();
        for (entry = 0; entry < mapSize; entry++)
        {
            CNestedRuntimeStatisticCollection & cur = map.item(entry);
            if (cur.matches(scope))
                return cur;
        }
    }
    {
        WriteLockBlock b(lock);
        // Check no-one added anything between the read and write locks
        mapSize = map.length();
        for (; entry < mapSize; entry++)
        {
            CNestedRuntimeStatisticCollection & cur = map.item(entry);
            if (cur.matches(scope))
                return cur;
        }
        CNestedRuntimeStatisticCollection * stats = new CNestedRuntimeStatisticCollection(scope, createStats(mapping));
        map.append(*stats);
        return *stats;
    }
}

void CNestedRuntimeStatisticMap::deserialize(MemoryBuffer& in)
{
    unsigned numItems;
    in.readPacked(numItems);
    for (unsigned i=0; i < numItems; i++)
    {
        StatsScopeId scope;
        unsigned mappingCode;
        scope.deserialize(in, currentStatisticsVersion);
        in.read(mappingCode);

        //Use allStatistics as the default mapping if it hasn't already been added.
        CNestedRuntimeStatisticCollection & child = addNested(scope, *queryStatsMapping(scope, mappingCode));
        child.deserialize(in);
    }
}

void CNestedRuntimeStatisticMap::deserializeMerge(MemoryBuffer& in)
{
    unsigned numItems;
    in.readPacked(numItems);
    for (unsigned i=0; i < numItems; i++)
    {
        StatsScopeId scope;
        unsigned mappingCode;
        scope.deserialize(in, currentStatisticsVersion);
        in.read(mappingCode);

        //Use allStatistics as the default mapping if it hasn't already been added.
        CNestedRuntimeStatisticCollection & child = addNested(scope, *queryStatsMapping(scope, mappingCode));
        child.deserializeMerge(in);
    }
}

void CNestedRuntimeStatisticMap::merge(const CNestedRuntimeStatisticMap & other, unsigned node)
{
    ReadLockBlock b(other.lock);
    ForEachItemIn(i, other.map)
    {
        CNestedRuntimeStatisticCollection & cur = other.map.item(i);
        CNestedRuntimeStatisticCollection & target = addNested(cur.scope, cur.queryMapping());
        target.merge(cur, node);
    }
}

void CNestedRuntimeStatisticMap::set(const CNestedRuntimeStatisticMap & other, unsigned node)
{
    ReadLockBlock b(other.lock);
    ForEachItemIn(i, other.map)
    {
        CNestedRuntimeStatisticCollection & cur = other.map.item(i);
        CNestedRuntimeStatisticCollection & target = addNested(cur.scope, cur.queryMapping());
        target.set(cur, node);
    }
}

void CNestedRuntimeStatisticMap::updateDelta(CNestedRuntimeStatisticMap & target, const CNestedRuntimeStatisticMap & source)
{
    ReadLockBlock b(source.lock);
    ForEachItemIn(i, source.map)
    {
        CNestedRuntimeStatisticCollection & curSource = source.map.item(i);
        CNestedRuntimeStatisticCollection & curTarget = target.addNested(curSource.scope, curSource.queryMapping());
        CNestedRuntimeStatisticCollection & curDelta = addNested(curSource.scope, curSource.queryMapping());
        curDelta.updateDelta(curTarget, curSource);
    }
}

bool CNestedRuntimeStatisticMap::serialize(MemoryBuffer& out) const
{
    ReadLockBlock b(lock);
    out.appendPacked(map.ordinality());
    bool nonEmpty = false;
    ForEachItemIn(i, map)
    {
        if (map.item(i).serialize(out))
            nonEmpty = true;
    }
    return nonEmpty;
}

void CNestedRuntimeStatisticMap::recordStatistics(IStatisticGatherer & target, bool clear) const
{
    ReadLockBlock b(lock);
    ForEachItemIn(i, map)
        map.item(i).recordStatistics(target, clear);
}

StringBuffer & CNestedRuntimeStatisticMap::toStr(StringBuffer &str) const
{
    ReadLockBlock b(lock);
    ForEachItemIn(i, map)
        map.item(i).toStr(str);
    return str;
}

void CNestedRuntimeStatisticMap::exportToSpan(ISpan * span, StringBuffer & prefix) const
{
    ReadLockBlock b(lock);
    ForEachItemIn(i, map)
        map.item(i).exportToSpan(span, prefix);
}

StringBuffer & CNestedRuntimeStatisticMap::toXML(StringBuffer &str) const
{
    ReadLockBlock b(lock);
    ForEachItemIn(i, map)
        map.item(i).toXML(str);
    return str;
}

CRuntimeStatisticCollection * CNestedRuntimeStatisticMap::createStats(const StatisticsMapping & mapping)
{
    return new CRuntimeStatisticCollection(mapping);
}

CRuntimeStatisticCollection * CNestedRuntimeSummaryStatisticMap::createStats(const StatisticsMapping & mapping)
{
    return new CRuntimeSummaryStatisticCollection(mapping);
}

//---------------------------------------------------

void StatsAggregation::noteValue(stat_type value)
{
    if (count == 0)
    {
        minValue = value;
        maxValue = value;
    }
    else
    {
        if (value < minValue)
            minValue = value;
        else if (value > maxValue)
            maxValue = value;
    }

    count++;
    sumValue += value;
}

stat_type StatsAggregation::getAve() const
{
    return (sumValue / count);
}


//---------------------------------------------------

ScopeCompare compareScopes(const char * scope, const char * key)
{
    byte left = *scope;
    byte right = *key;
    //Check for root scope "" compared with anything
    if (!left)
    {
        if (!right)
            return SCequal;
        return SCparent;
    }
    else if (!right)
    {
        return SCchild;
    }

    bool hadCommonScope = false;
    for (;;)
    {
        if (left != right)
        {
            //FUTURE: Extend this function to support skipping numbers to allow wildcard matching
            if (!left)
            {
                if (right == ':')
                    return SCparent; // scope is a parent (prefix) of the key
            }
            if (!right)
            {
                if (left == ':')
                    return SCchild;  // scope is a child (superset) of the key
            }
            return hadCommonScope ? SCrelated : SCunrelated;
        }

        if (!left)
            return SCequal;

        if (left == ':')
            hadCommonScope = true;

        left = *++scope;
        right =*++key;
    }
}

ScopeFilter::ScopeFilter(const char * scopeList)
{
    //MORE: This currently expands a list of scopes - it should probably be improved
    scopes.appendList(scopeList, ",");
}

void ScopeFilter::addScope(const char * scope)
{
    if (!scope)
        return;

    if (streq(scope, "*"))
    {
        scopes.kill();
        minDepth = 0;
        maxDepth = UINT_MAX;
        return;
    }

    if (ids)
        throw makeStringExceptionV(0, "Cannot filter by id and scope in the same request");

    unsigned depth = queryScopeDepth(scope);
    if ((scopes.ordinality() == 0) || (depth < minDepth))
        minDepth = depth;
    if ((scopes.ordinality() == 0) || (depth > maxDepth))
        maxDepth = depth;
    scopes.append(scope);
}

void ScopeFilter::addScopes(const char * scope)
{
    StringArray list;
    list.appendList(scope, ",");
    ForEachItemIn(i, list)
        addScope(list.item(i));
}

void ScopeFilter::addScopeType(StatisticScopeType scopeType)
{
    if (scopeType == SSTall)
        return;

    scopeTypes.append(scopeType);
}

void ScopeFilter::addId(const char * id)
{
    if (scopes)
        throw makeStringExceptionV(0, "Cannot filter by id and scope in the same request");

    ids.append(id);
}

void ScopeFilter::setDepth(unsigned low, unsigned high)
{
    if (low > high)
        throw makeStringExceptionV(0, "Depth parameters in wrong order %u..%u", low, high);

    minDepth = low;
    maxDepth = high;
}


void ScopeFilter::intersectDepth(const unsigned low, const unsigned high)
{
    if (low > high)
        throw makeStringExceptionV(0, "Depth parameters in wrong order %u..%u", low, high);

    if (minDepth < low)
        minDepth = low;
    if (maxDepth > high)
        maxDepth = high;
}


ScopeCompare ScopeFilter::compare(const char * scope) const
{
    ScopeCompare result = SCunknown;
    if (scopes)
    {
        //If scopes have been provided, then we are searching for an exact match against that scope
        ForEachItemIn(i, scopes)
            result |= compareScopes(scope, scopes.item(i));
    }
    else
    {
        //How does the depth of the scope compare with the range we are expecting?
        unsigned depth = queryScopeDepth(scope);
        if (depth < minDepth)
            return SCparent;
        if (depth > maxDepth)
            return SCchild;

        //Assume it is a match until proven otherwise
        result |= SCequal;
        // Could be the child of a match
        if (depth > minDepth)
            result |= SCchild;
        //Could be the parent of a match
        if (depth < maxDepth)
            result |= SCparent;

        //Check if the type of the current object matches the type
        const char * tail = queryScopeTail(scope);
        if (scopeTypes.ordinality())
        {
            StatsScopeId id(tail);
            if (!scopeTypes.contains(id.queryScopeType()))
                result &= ~SCequal;
        }

        if (ids)
        {
            if (!ids.contains(tail))
                result &= ~SCequal;
        }
    }

    if (!(result & SCequal))
        return result;

    //Have a match - now check that the attributes match as required
    //MORE:
    if (false)
    {
        result &= ~SCequal;
    }

    return result;
}

bool ScopeFilter::canAlwaysPreFilter() const
{
    //If the only filter being applied is a restriction on the minimum depth, then you can always apply it as a pre-filter
    return (!ids && !scopeTypes && !scopes && maxDepth == UINT_MAX);
}

int ScopeFilter::compareDepth(unsigned depth) const
{
    if (depth < minDepth)
        return -1;
    if (depth > maxDepth)
        return +1;
    return 0;
}

StringBuffer & ScopeFilter::describe(StringBuffer & out) const
{
    if ((minDepth != 0) || (maxDepth != UINT_MAX))
    {
        if (minDepth == maxDepth)
            out.appendf(",depth(%u)", minDepth);
        else
            out.appendf(",depth(%u,%u)", minDepth, maxDepth);
    }
    if (scopeTypes)
    {
        out.append(",stype[");
        ForEachItemIn(i, scopeTypes)
        {
            if (i)
                out.append(",");
            out.append(queryScopeTypeName((StatisticScopeType)scopeTypes.item(i)));
        }
        out.append("]");
    }
    if (scopes)
    {
        out.append(",scope[");
        ForEachItemIn(i, scopes)
        {
            if (i)
                out.append(",");
            out.append(scopes.item(i));
        }
        out.append("]");
    }
    if (ids)
    {
        out.append(",id[");
        ForEachItemIn(i, ids)
        {
            if (i)
                out.append(",");
            out.append(ids.item(i));
        }
        out.append("]");
    }
    return out;
}

void ScopeFilter::finishedFilter()
{
    //If scopeTypes are provided, then this code ensure that any scopes and ids match them
    //but that would have little benefit, and would cause complications if the only id was removed

    //Some scope types can only exist at a single level.
    if (scopeTypes.ordinality() == 1)
    {
        switch (scopeTypes.item(0))
        {
        case SSTglobal:
            intersectDepth(0, 0);
            break;
        case SSTworkflow:
            intersectDepth(1, 1);
            break;
        case SSTgraph:
            //This should really be intersectDepth(2,2) but workunits prior to 7.4 did not have graph ids prefixed by the wfid
            //Remove once 7.2 is a distant memory (see HPCC-22887)
            intersectDepth(1, 2);
            break;
        case SSTsubgraph:
            intersectDepth(3, UINT_MAX);
            break;
        case SSTactivity:
            intersectDepth(4, UINT_MAX);
            break;
        }
    }
}

bool ScopeFilter::hasSingleMatch() const
{
    return scopes.ordinality() == 1 || ids.ordinality() == 1;
}

bool ScopeFilter::matchOnly(StatisticScopeType scopeType) const
{
    if ((scopeTypes.ordinality() == 1) && (scopeTypes.item(0) == scopeType))
        return true;

    //Check the types of the scopes that are being searched
    if (scopes.ordinality())
    {
        ForEachItemIn(i, scopes)
        {
            const char * scopeId = queryScopeTail(scopes.item(i));
            StatsScopeId id(scopeId);
            if (id.queryScopeType() != scopeType)
                return false;
        }
        return true;
    }

    if (ids.ordinality())
    {
        ForEachItemIn(i, ids)
        {
            const char * scopeId = ids.item(i);
            StatsScopeId id(scopeId);
            if (id.queryScopeType() != scopeType)
                return false;
        }
        return true;
    }

    return false;
}

//---------------------------------------------------

bool ScopedItemFilter::matchDepth(unsigned low, unsigned high) const
{
    if (maxDepth && low && maxDepth < low)
        return false;
    if (minDepth && high && minDepth > high)
        return false;
    return true;
}

bool ScopedItemFilter::match(const char * search) const
{
    if (search)
    {
        if (value)
        {
            if (hasWildcard)
            {
                //MORE: If wildcarding ends up being used a lot then this should be replaced with something that creates a DFA
                if (!WildMatch(search, value, false))
                    return false;
            }
            else
            {
                return streq(search, value);
            }
        }

        if (minDepth || maxDepth)
        {
            unsigned searchDepth = queryScopeDepth(search);
            if (searchDepth < minDepth)
                return false;
            if (maxDepth && searchDepth > maxDepth)
                return false;
        }
    }
    return true;
}


bool ScopedItemFilter::recurseChildScopes(const char * curScope) const
{
    if (maxDepth == 0 || !curScope)
        return true;

    if (queryScopeDepth(curScope) >= maxDepth)
        return false;
    return true;
}

void ScopedItemFilter::set(const char * _value)
{
    if (_value && !streq(_value, "*") )
    {
        value.set(_value);
        minDepth = queryScopeDepth(_value);
        if (!strchr(_value, '*'))
        {
            maxDepth = minDepth;
            hasWildcard = strchr(_value, '?') != NULL;
        }
        else
            hasWildcard = true;
    }
    else
        value.clear();
}

void ScopedItemFilter::setDepth(unsigned _depth)
{
    minDepth = _depth;
    maxDepth = _depth;
}

void ScopedItemFilter::setDepth(unsigned _minDepth, unsigned _maxDepth)
{
    minDepth = _minDepth;
    maxDepth = _maxDepth;
}


StatisticsFilter::StatisticsFilter()
{
    init();
}

StatisticsFilter::StatisticsFilter(const char * filter)
{
    init();
    setFilter(filter);
}

StatisticsFilter::StatisticsFilter(StatisticCreatorType _creatorType, StatisticScopeType _scopeType, StatisticMeasure _measure, StatisticKind _kind)
{
    init();
    creatorType = _creatorType;
    scopeType = _scopeType;
    measure = _measure;
    kind = _kind;
}

StatisticsFilter::StatisticsFilter(const char * _creatorType, const char * _scopeType, const char * _kind)
{
    init();
    set(_creatorType, _scopeType, _kind);
}

StatisticsFilter::StatisticsFilter(const char * _creatorTypeText, const char * _creator, const char * _scopeTypeText, const char * _scope, const char * _measureText, const char * _kindText)
{
    init();
    set(_creatorTypeText, _creator, _scopeTypeText, _scope, _measureText, _kindText);
}

StatisticsFilter::StatisticsFilter(StatisticCreatorType _creatorType, const char * _creator, StatisticScopeType _scopeType, const char * _scope, StatisticMeasure _measure, StatisticKind _kind)
{
    init();
    creatorType = _creatorType;
    setCreator(_creator);
    scopeType = _scopeType;
    setScope(_scope);
    measure = _measure;
    kind = _kind;
}

void StatisticsFilter::init()
{
    creatorType = SCTall;
    scopeType = SSTall;
    measure = SMeasureAll;
    kind = StKindAll;
}

bool StatisticsFilter::matches(StatisticCreatorType curCreatorType, const char * curCreator, StatisticScopeType curScopeType, const char * curScope, StatisticMeasure curMeasure, StatisticKind curKind, unsigned __int64 value) const
{
    if ((curCreatorType != SCTall) && (creatorType != SCTall) && (creatorType != curCreatorType))
        return false;
    if ((curScopeType != SSTall) && (scopeType != SSTall) && (scopeType != curScopeType))
        return false;
    if ((curMeasure != SMeasureAll) && (measure != SMeasureAll) && (measure != curMeasure))
        return false;
    if ((curKind!= StKindAll) && (kind != StKindAll) && (kind != curKind))
        return false;
    if (!creatorFilter.match(curCreator))
        return false;
    if (!scopeFilter.match(curScope))
        return false;
    if (value != MaxStatisticValue)
    {
        if ((value < minValue) || (value > maxValue))
            return false;
    }
    return true;
}

bool StatisticsFilter::recurseChildScopes(StatisticScopeType curScopeType, const char * curScope) const
{
    switch (curScopeType)
    {
    case SSTgraph:
        // A child of a graph will have depth 2 or more
        if (!scopeFilter.matchDepth(2, (unsigned)-1))
            return false;
        break;
    case SSTsubgraph:
        // A child of a subgraph will have depth 3 or more
        if (!scopeFilter.matchDepth(3, (unsigned)-1))
            return false;
        break;
    }
    if (!curScope)
        return true;
    return scopeFilter.recurseChildScopes(curScope);
}


void StatisticsFilter::set(const char * creatorTypeText, const char * scopeTypeText, const char * kindText)
{
    StatisticCreatorType creatorType = queryCreatorType(creatorTypeText, SCTnone);
    StatisticScopeType scopeType = queryScopeType(scopeTypeText, SSTnone);

    if (creatorType != SCTnone)
        setCreatorType(creatorType);
    if (scopeType != SSTnone)
        setScopeType(scopeType);
    setKind(kindText);
}

void StatisticsFilter::set(const char * _creatorTypeText, const char * _creator, const char * _scopeTypeText, const char * _scope, const char * _measureText, const char * _kindText)
{
    StatisticMeasure newMeasure = queryMeasure(_measureText, SMeasureNone);
    if (newMeasure != SMeasureNone)
        setMeasure(newMeasure);
    set(_creatorTypeText, _scopeTypeText, _kindText);
    setCreator(_creator);
    setScope(_scope);
}

void StatisticsFilter::setCreatorDepth(unsigned _minCreatorDepth, unsigned _maxCreatorDepth)
{
    creatorFilter.setDepth(_minCreatorDepth, _maxCreatorDepth);
}

void StatisticsFilter::setCreator(const char * _creator)
{
    creatorFilter.set(_creator);
}

void StatisticsFilter::setCreatorType(StatisticCreatorType _creatorType)
{
    creatorType = _creatorType;
}

void StatisticsFilter::addFilter(const char * filter)
{
    //Match a filter of the form category[value]  (use square brackets to avoid bash grief)
    const char * openBra = strchr(filter, '[');
    if (!openBra)
        return;
    const char * closeBra = strchr(openBra, ']');
    if (!closeBra)
        return;

    const char * start = openBra + 1;
    StringBuffer value(closeBra - start, start);
    if (hasPrefix(filter, "creator[", false))
        setCreator(value);
    else if (hasPrefix(filter, "creatortype[", false))
        setCreatorType(queryCreatorType(value, SCTall));
    else if (hasPrefix(filter, "depth[", false))
    {
        const char * comma = strchr(value, ',');
        if (comma)
            setScopeDepth(atoi(value), atoi(comma+1));
        else
            setScopeDepth(atoi(value));
    }
    else if (hasPrefix(filter, "kind[", false))
        setKind(value);
    else if (hasPrefix(filter, "measure[", false))
        setMeasure(queryMeasure(value, SMeasureAll));
    else if (hasPrefix(filter, "scope[", false))
        setScope(value);
    else if (hasPrefix(filter, "scopetype[", false))
        setScopeType(queryScopeType(value, SSTall));
    else if (hasPrefix(filter, "value[", false))
    {
        //value[exact|low..high] where low and high are optional
        unsigned __int64 lowValue = 0;
        unsigned __int64 highValue = MaxStatisticValue;
        if (isdigit(*value))
            lowValue = (unsigned __int64)atoi64(value);
        const char * dotdot = strstr(value, "..");
        if (dotdot)
        {
            unsigned __int64 maxValue = (unsigned __int64)atoi64(dotdot + 2);
            if (maxValue != 0)
                highValue = maxValue;
        }
        else
            highValue = lowValue;
        setValueRange(lowValue, highValue);
    }
    else
        throw MakeStringException(1, "Unknown stats filter '%s' - expected creator,creatortype,depth,kind,measure,scope,scopetype", filter);
}


void StatisticsFilter::setFilter(const char * filter)
{
    if (isEmptyString(filter))
        return;

    for (;;)
    {
        const char * closeBra = strchr(filter, ']');
        if (!closeBra)
            throw MakeStringException(1, "Missing close bracket ']' in '%s' ", filter);

        const char * comma = strchr(closeBra, ',');
        if (comma)
        {
            //Take a copy - simplicity rather than efficiency
            StringBuffer temp(comma - filter, filter);
            addFilter(temp);
            filter = comma + 1;
        }
        else
        {
            addFilter(filter);
            return;
        }
    }
}

void StatisticsFilter::setScopeDepth(unsigned _scopeDepth)
{
    scopeFilter.setDepth(_scopeDepth);
}

void StatisticsFilter::setScopeDepth(unsigned _minScopeDepth, unsigned _maxScopeDepth)
{
    scopeFilter.setDepth(_minScopeDepth, _maxScopeDepth);
}

void StatisticsFilter::setScope(const char * _scope)
{
    scopeFilter.set(_scope);
}

void StatisticsFilter::setScopeType(StatisticScopeType _scopeType)
{
    scopeType = _scopeType;
    switch (scopeType)
    {
    case SSTglobal:
    case SSTgraph:
        scopeFilter.setDepth(1);
        break;
    case SSTsubgraph:
        scopeFilter.setDepth(2);
        break;
    case SSTactivity:
        scopeFilter.setDepth(3);
        break;
    }
}

void StatisticsFilter::setMeasure(StatisticMeasure _measure)
{
    measure = _measure;
}

void StatisticsFilter::setValueRange(unsigned __int64 _minValue, unsigned __int64 _maxValue)
{
    minValue = _minValue;
    maxValue = _maxValue;
}

void StatisticsFilter::setKind(StatisticKind _kind)
{
    kind = _kind;
    if (measure == SMeasureAll)
        measure = queryMeasure(kind);
}

void StatisticsFilter::setKind(const char * _kind)
{
    if (!_kind || !*_kind || streq(_kind, "*"))
    {
        if (measure == SMeasureNone)
            measure = SMeasureAll;
        kind = StKindAll;
        return;
    }

    //Convert a kind wildcard to a measure
    for (unsigned i1=SMeasureAll+1; i1 < SMeasureMax; i1++)
    {
        const char * prefix = queryMeasurePrefix((StatisticMeasure)i1);
        size_t len = strlen(prefix);
        if (len && strnicmp(_kind, prefix, len) == 0)
        {
            setMeasure((StatisticMeasure)i1);
            //Treat When* and When as filters on times.
            if (streq(_kind + len, "*") || !_kind[len])
                return;
        }
    }

    //Other wildcards not currently supported
    kind = queryStatisticKind(_kind, StKindAll);
}


//---------------------------------------------------

class CStatsCategory : public CInterface
{
public:
    StringAttr longName;
    StringAttr shortName;

    CStatsCategory(const char *_longName, const char *_shortName)
        : longName(_longName), shortName(_shortName)
    {
    }
    bool match(const char *_longName, const char *_shortName)
    {
        bool lm = stricmp(_longName, longName)==0;
        bool sm = stricmp(_shortName, shortName)==0;
        if (lm || sm)
        {
            if (lm && sm)
                return true;
            throw MakeStringException(0, "A stats category %s (%s) is already registered", shortName.get(), longName.get());
        }
        return false;
    }
};

static CIArrayOf<CStatsCategory> statsCategories;
static CriticalSection statsCategoriesCrit;

extern int registerStatsCategory(const char *longName, const char *shortName)
{
    CriticalBlock b(statsCategoriesCrit);
    ForEachItemIn(idx, statsCategories)
    {
        if (statsCategories.item(idx).match(longName, shortName))
            return idx;
    }
    statsCategories.append(*new CStatsCategory(longName, shortName));
    return statsCategories.ordinality()-1;
}

static void checkKind(StatisticKind kind)
{
    if (kind < StMax)
    {
        const StatisticMeta & meta = statsMetaData[kind];
        if (meta.kind != kind)
            throw makeStringExceptionV(0, "Statistic %u in the wrong order", kind);
    }

    StatisticKind serialKind = querySerializedKind(kind);
    StatisticKind rawKind = queryRawKind(kind);
    if (kind != serialKind)
        assertex(queryRawKind(serialKind) == kind);
    if (kind != rawKind)
        assertex(querySerializedKind(rawKind) == kind);

    StatisticMeasure measure = queryMeasure(kind);
    const char * shortName = queryStatisticName(kind);
    StringBuffer longName;
    queryLongStatisticName(longName, kind);
    const char * tagName __attribute__ ((unused)) = queryTreeTag(kind);
    const char * prefix = queryMeasurePrefix(measure);
    //Check short names are all correctly prefixed.
    assertex(strncmp(shortName, prefix, strlen(prefix)) == 0);
}

static void checkDistributedKind(StatisticKind kind)
{
    checkKind(kind);
    checkKind((StatisticKind)(kind|StMinX));
    checkKind((StatisticKind)(kind|StMaxX));
    checkKind((StatisticKind)(kind|StAvgX));
    checkKind((StatisticKind)(kind|StSkew));
    checkKind((StatisticKind)(kind|StSkewMin));
    checkKind((StatisticKind)(kind|StSkewMax));
    checkKind((StatisticKind)(kind|StNodeMin));
    checkKind((StatisticKind)(kind|StNodeMax));
    checkKind((StatisticKind)(kind|StDeltaX));
}

void verifyStatisticFunctions()
{
    static_assert(_elements_in(statsMetaData) == StMax, "statsMetaData is missing entries");
    static_assert(_elements_in(measureNames) == SMeasureMax+1 && !measureNames[SMeasureMax], "measureNames needs updating");
    static_assert(_elements_in(creatorTypeNames) == SCTmax+1 && !creatorTypeNames[SCTmax], "creatorTypeNames needs updating");
    static_assert(_elements_in(scopeTypeNames) == SSTmax+1 && !scopeTypeNames[SSTmax], "scopeTypeNames needs updating");

    //Check the various functions return values for all possible values.
    for (unsigned i1=SMeasureAll; i1 < SMeasureMax; i1++)
    {
        const char * prefix __attribute__((unused)) = queryMeasurePrefix((StatisticMeasure)i1);
        const char * name = queryMeasureName((StatisticMeasure)i1);
        assertex(queryMeasure(name, SMeasureMax) == i1);
    }

    for (StatisticScopeType sst = SSTnone; sst < SSTmax; sst = (StatisticScopeType)(sst+1))
    {
        const char * name = queryScopeTypeName(sst);
        assertex(queryScopeType(name, SSTmax) == sst);

    }

    for (StatisticCreatorType sct = SCTnone; sct < SCTmax; sct = (StatisticCreatorType)(sct+1))
    {
        const char * name = queryCreatorTypeName(sct);
        assertex(queryCreatorType(name, SCTmax) == sct);
    }

    for (unsigned i2=StKindAll+1; i2 < StMax; i2++)
    {
        checkDistributedKind((StatisticKind)i2);
    }
}

#ifdef _DEBUG
MODULE_INIT(INIT_PRIORITY_STANDARD)
{
    verifyStatisticFunctions();
    return true;
}
#endif


