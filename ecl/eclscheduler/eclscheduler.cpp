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

#include <jlib.hpp>
#include <jmisc.hpp>
#include <jfile.hpp>
#include <jencrypt.hpp>
#include <jregexp.hpp>
#include <jutil.hpp>
#include <mpbase.hpp>
#include <daclient.hpp>
#include <dasess.hpp>
#include <danqs.hpp>

#include "environment.hpp"
#include "workunit.hpp"
#include "wujobq.hpp"
#include "eventqueue.hpp"

//=========================================================================================
//////////////////////////////////////////////////////////////////////////////////////////////
extern "C" void caughtSIGPIPE(int sig)
{
    DBGLOG("Caught sigpipe %d", sig);
}

extern "C" void caughtSIGHUP(int sig)
{
    DBGLOG("Caught sighup %d", sig);
}


extern "C" void caughtSIGALRM(int sig)
{
    DBGLOG("Caught sigalrm %d", sig);
}

extern "C" void caughtSIGTERM(int sig)
{
    DBGLOG("Caught sigterm %d", sig);
}

void initSignals()
{
#ifndef _WIN32
//  signal(SIGTERM, caughtSIGTERM);
    signal(SIGPIPE, caughtSIGPIPE);
    signal(SIGHUP, caughtSIGHUP);
    signal(SIGALRM, caughtSIGALRM);

#endif
}   

//=========================================================================================

class Waiter : implements IAbortHandler, public CInterface
{
    Semaphore aborted;
public:
    IMPLEMENT_IINTERFACE;

    bool wait(unsigned timeout)
    {
        return aborted.wait(timeout);
    }
    void wait()
    {
        aborted.wait();
    }
    bool onAbort()
    {
        aborted.signal();
#ifdef _DEBUG
        return false; // we want full leak checking info
#else
        return true; // we don't care - just exit as fast as we can
#endif
    }
} waiter;

//=========================================================================================

class EclScheduler : public CInterface, implements IExceptionHandler
{
private:
    class WUExecutor : public CInterface, implements IScheduleEventExecutor
    {
    public:
        WUExecutor(EclScheduler * _owner) : owner(_owner) {}
        IMPLEMENT_IINTERFACE;
        virtual void execute(char const * wuid, char const * name, char const * text)
        {
            owner->execute(wuid, name, text);
        }
    private:
        EclScheduler * owner;
    };

    friend class WUExecutor;

public:
    EclScheduler(const char *serverName)
    {
        executor.setown(new WUExecutor(this));
        processor.setown(getScheduleEventProcessor(serverName, executor.getLink(), this));
    }
    void start() { processor->start(); }
    void stop() { processor->stop(); }
    virtual bool fireException(IException *e) 
    { 
        StringBuffer msg;
        OERRLOG("Scheduler error: %d: %s", e->errorCode(), e->errorMessage(msg).str()); e->Release(); 
        OERRLOG("Scheduler will now terminate"); 
        waiter.onAbort();
        return false; 
    }

private:
    void execute(char const * wuid, char const * name, char const * text)
    {
        Owned<IWorkflowScheduleConnection> wfconn(getWorkflowScheduleConnection(wuid));
        wfconn->lock();
        wfconn->push(name, text);
        if(!wfconn->queryActive())
        {
            if (!runWorkUnit(wuid))
            {
                //The work unit failed to run for some reason.. check if it has disappeared
                Owned<IWorkUnitFactory> factory = getWorkUnitFactory();
                Owned<IConstWorkUnit> w = factory->openWorkUnit(wuid);
                if (!w)
                {
                    OERRLOG("Scheduled workunit %s no longer exists - descheduling", wuid);
                    descheduleWorkunit(wuid);
                    wfconn->remove();
                }
            }
        }
        wfconn->unlock();
    }

private:
    Owned<IScheduleEventProcessor> processor;
    Owned<WUExecutor> executor;
};


void openLogFile()
{
#ifndef _CONTAINERIZED
    Owned<IPropertyTree> config = getComponentConfig();
    Owned<IComponentLogFileCreator> lf = createComponentLogFileCreator(config, "eclscheduler");
    lf->beginLogging();
#else
    setupContainerizedLogMsgHandler();
#endif
}

//=========================================================================================

static constexpr const char * defaultYaml = R"!!(
version: "1.0"
eclscheduler:
  daliServers: dali
  enableSysLog: true
  name: myeclscheduler
)!!";

int main(int argc, const char *argv[])
{
    if (!checkCreateDaemon(argc, argv))
        return EXIT_FAILURE;

    InitModuleObjects();
    initSignals();
    NoQuickEditSection x;

    Owned<IFile> sentinelFile = createSentinelTarget();
    // We remove any existing sentinel until we have validated that we can successfully start (i.e. all options are valid...)
    removeSentinelFile(sentinelFile);

    const char *iniFileName = nullptr;
    if (checkFileExists("eclscheduler.xml") )
        iniFileName = "eclscheduler.xml";
    else if (checkFileExists("eclccserver.xml") )
        iniFileName = "eclccserver.xml";

    Owned<IPropertyTree> globals;
    try
    {
        globals.setown(loadConfiguration(defaultYaml, argv, "eclscheduler", "ECLSCHEDULER", iniFileName, nullptr));
    }
    catch (IException * e)
    {
        UERRLOG(e);
        e->Release();
        return 1;
    }
    catch(...)
    {
        if (iniFileName)
            OERRLOG("Failed to load configuration %s", iniFileName);
        else
            OERRLOG("Cannot find eclscheduler.xml or eclccserver.xml");
        return 1;
    }

    openLogFile();

    setStatisticsComponentName(SCThthor, globals->queryProp("@name"), true);
    if (globals->getPropBool("@enableSysLog",true))
        UseSysLogForOperatorMessages();
    const char *daliServers = globals->queryProp("@daliServers");
    if (!daliServers)
    {
        OWARNLOG("No Dali server list specified - assuming local");
        daliServers = ".";
    }
    Owned<IGroup> serverGroup = createIGroupRetry(daliServers, DALI_SERVER_PORT);
    try
    {
        initClientProcess(serverGroup, DCR_EclScheduler);

        CIArrayOf<EclScheduler> schedulers;

#ifdef _CONTAINERIZED
        Owned<IPTreeIterator> queues = globals->getElements("queues");
        ForEach(*queues)
        {
            IPTree &queue = queues->query();
            const char *qname = queue.queryProp("@name");
            DBGLOG("Start listening to queue %s", qname);
            Owned<EclScheduler> scheduler = new EclScheduler(qname);
            scheduler->start();
            schedulers.append(*scheduler.getClear());
        }
#else
        Owned <IStringIterator> targetClusters = getTargetClusters("EclSchedulerProcess", globals->queryProp("@name"));
        ForEach (*targetClusters)
        {
            SCMStringBuffer targetCluster;
            targetClusters->str(targetCluster);
            Owned<EclScheduler> scheduler = new EclScheduler(targetCluster.str());
            scheduler->start();
            schedulers.append(*scheduler.getClear());
        }
#endif
        if (schedulers.empty())
            throw MakeStringException(0, "No clusters found to schedule for");

        // if we got here, eclscheduler is successfully started and all options are good, so create the "sentinel file" for re-runs from the script
        writeSentinelFile(sentinelFile);
        LocalIAbortHandler abortHandler(waiter);
        waiter.wait();
        ForEachItemIn(schedIdx, schedulers)
        {
            schedulers.item(schedIdx).stop();
        }
    }
    catch (IException * e)
    {
        EXCLOG(e, "Terminating unexpectedly");
        e->Release();
    }
    catch(...)
    {
        IERRLOG("Terminating unexpectedly");
    }
    globals.clear();
    UseSysLogForOperatorMessages(false);
    ::closedownClientProcess(); // dali client closedown
    releaseAtoms();
    ExitModuleObjects();
    return 0;
}
