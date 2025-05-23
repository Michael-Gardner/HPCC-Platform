/*##############################################################################

    Copyright (C) 2025 HPCC Systems®.

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

#include "evtool.hpp"
#include "evtindex.hpp"
#include "jevent.hpp"
#include "jstream.hpp"
#include "jstring.hpp"
#include <map>

class CIndexFileSummary : public IEventVisitor, public CEventConsumingOp
{
public: // IEventVisitor
    virtual bool visitFile(const char *filename, uint32_t version) override
    {
        return true;
    }

    virtual Continuation visitEvent(EventType id) override
    {
        switch (id)
        {
        case EventIndexLookup:
        case EventIndexLoad:
        case EventIndexEviction:
        case MetaFileInformation:
            currentEvent = id;
            return IEventVisitor::visitContinue;
        default:
            return IEventVisitor::visitSkipEvent;
        }
    }

    virtual Continuation visitAttribute(EventAttr id, const char *value) override
    {
        switch (id)
        {
        case EvAttrPath:
            // assumes file ID is seen before path
            fileInfo[currentFileId].set(value);
            break;
        default:
            break;
        }
        return IEventVisitor::visitContinue;
    }

    virtual Continuation visitAttribute(EventAttr id, bool value) override
    {
        switch (id)
        {
        case EvAttrInCache:
            if (value)
                nodeSummary().hits++;
            else
                nodeSummary().misses++;
            break;
        }
        return IEventVisitor::visitContinue;
    }

    virtual Continuation visitAttribute(EventAttr id, __uint64 value) override
    {
        switch (id)
        {
        case EvAttrFileId:
            currentFileId = value;
            break;
        case EvAttrNodeKind:
            currentNodeKind = value;
            switch (currentEvent)
            {
            case EventIndexLoad:
                nodeSummary().loads++;
                break;
            case EventIndexEviction:
                nodeSummary().evictions++;
                break;
            default:
                break;
            }
            break;
        case EvAttrExpandedSize:
            switch (currentEvent)
            {
            case EventIndexLoad:
                nodeSummary().loadedSize += value;
                break;
            case EventIndexEviction:
                nodeSummary().evictedSize += value;
                break;
            default:
                break;
            }
            break;
        case EvAttrReadTime:
            nodeSummary().read += value;
            break;
        case EvAttrElapsedTime:
            nodeSummary().elapsed += value;
            break;
        default:
            break;
        }
        return IEventVisitor::visitContinue;
    }

    virtual bool departEvent() override
    {
        currentEvent = EventNone;
        currentFileId = 0;
        currentNodeKind = 0;
        return true;
    }

    virtual void departFile(uint32_t bytesRead) override
    {
        static const char* header = "FileId,FilePath,BranchHits,BranchMisses,BranchLoads,BranchLoadedSize,BranchEvictions,BranchEvictedSize,BranchElapsed,BranchRead,LeafHits,LeafMisses,LeafLoads,LeafLoadedSize,LeafEvictions,LeafEvictedSize,LeafElapsed,LeafRead\n";
        static size32_t headerSize = size32_t(strlen(header));
        out->put(headerSize, header);
        StringBuffer line;
        for (const Summary::value_type& s : summary)
        {
            line.append(s.first);
            appendCSVColumn(line, fileInfo[s.first].str());
            appendCSVColumn(line, s.second.branch.hits);
            appendCSVColumn(line, s.second.branch.misses);
            appendCSVColumn(line, s.second.branch.loads);
            appendCSVColumn(line, s.second.branch.loadedSize);
            appendCSVColumn(line, s.second.branch.evictions);
            appendCSVColumn(line, s.second.branch.evictedSize);
            appendCSVColumn(line, s.second.branch.elapsed);
            appendCSVColumn(line, s.second.branch.read);
            appendCSVColumn(line, s.second.leaf.hits);
            appendCSVColumn(line, s.second.leaf.misses);
            appendCSVColumn(line, s.second.leaf.loads);
            appendCSVColumn(line, s.second.leaf.loadedSize);
            appendCSVColumn(line, s.second.leaf.evictions);
            appendCSVColumn(line, s.second.leaf.evictedSize);
            appendCSVColumn(line, s.second.leaf.elapsed);
            appendCSVColumn(line, s.second.leaf.read);
            line.append('\n');
            out->put(line.length(), line.str());
            line.clear();
        }
    }

public:
    IMPLEMENT_IINTERFACE;
    bool doOp() override
    {
        return traverseEvents(inputPath, *this);
    }

protected:
    struct NodeKindSummary;
    NodeKindSummary& nodeSummary()
    {
        switch (currentNodeKind)
        {
        case 0: // branch
            return summary[currentFileId].branch;
        case 1: // leaf
            return summary[currentFileId].leaf;
        default:
            throw makeStringExceptionV(-1, "unknown node kind: %llu", currentNodeKind);
        }
    }

    void appendCSVColumn(StringBuffer& line, const char* value)
    {
        encodeCSVColumn(line.append(','), value);
    }
    void appendCSVColumn(StringBuffer& line, __uint64 value)
    {
        line.append(',').append(value);
    }

protected:
    using FileInfo = std::map<unsigned, StringAttr>;
    struct NodeKindSummary
    {
        __uint64 hits{0};
        __uint64 misses{0};
        __uint64 loads{0};
        __uint64 loadedSize{0};
        __uint64 evictions{0};
        __uint64 evictedSize{0};
        __uint64 elapsed{0};
        __uint64 read{0};
    };
    struct FileSummary
    {
        NodeKindSummary branch;
        NodeKindSummary leaf;
    };
    using Summary = std::map<unsigned, FileSummary>;
    FileInfo fileInfo;
    Summary summary;
    EventType currentEvent{EventNone};
    __uint64 currentFileId{0};
    __uint64 currentNodeKind{0};
};

// Connector between the CLI and the index file summary operation.
class CEvtIndexSummaryCommand : public TEventConsumingCommand<CIndexFileSummary>
{
public:
    virtual void usageSyntax(int argc, const char* argv[], int pos, IBufferedSerialOutputStream& out) override
    {
        TEventConsumingCommand<CIndexFileSummary>::usageSyntax(argc, argv, pos, out);
        static const char* usageStr =
R"!!!([options] [filters] <filename>
)!!!";
        static size32_t usageStrLength = size32_t(strlen(usageStr));
        out.put(usageStrLength, usageStr);
    }

    virtual void usageSynopsis(IBufferedSerialOutputStream& out) override
    {
        static const char* usageStr = R"!!!(
Summarize the index events in a binary event file.
)!!!";
        static size32_t usageStrLength = size32_t(strlen(usageStr));
        out.put(usageStrLength, usageStr);
    }
};

IEvToolCommand* createIndexSummaryCommand()
{
    return new CEvtIndexSummaryCommand();
}
