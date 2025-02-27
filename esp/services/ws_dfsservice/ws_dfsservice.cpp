/*##############################################################################

    HPCC SYSTEMS software Copyright (C) 2022 HPCC Systems®.

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

#pragma warning (disable : 4786)

#include "jflz.hpp"
#include "jstring.hpp"

#include "daaudit.hpp"
#include "dautils.hpp"
#include "dadfs.hpp"
#include "dafdesc.hpp"
#include "esp.hpp"
#include "exception_util.hpp"
#include "package.h"

#include "ws_dfsclient.hpp"
#include "ws_dfsservice.hpp"

using namespace wsdfs;

// all fake for now
// this will be implemened in Dali.
static std::atomic<unsigned __int64> nextLockID{0};
static unsigned __int64 getLockId(unsigned __int64 leaseId)
{
    // associated new locks with lease
    return ++nextLockID;
}


enum LfnMetaOpts : byte
{
    LfnMOptNone   = 0x00,
    LfnMOptRemap  = 0x01,
    LfnMOptTls    = 0x02,
};
BITMASK_ENUM(LfnMetaOpts);
static void populateLFNMeta(IUserDescriptor *userDesc, const char *logicalName, unsigned __int64 leaseId, LfnMetaOpts opts, IPropertyTree *metaRoot, IPropertyTree *meta)
{
    CDfsLogicalFileName lfn;
    lfn.set(logicalName);
    if (lfn.isForeign())
        ThrowStringException(-1, "foreign file %s. Not supported", logicalName);

    assertex(!lfn.isMulti()); // not supported, don't think needs to be/will be.

    Owned<IPropertyTree> tree = queryDistributedFileDirectory().getFileTree(logicalName, userDesc);
    if (!tree)
        return;
    if (hasMask(opts, LfnMOptRemap))
    {
        bool secure = hasMask(opts, LfnMOptTls);
        // If !secure - called from insecure DFS service, remapGroupsToDafilesrv needs to direct to an insecure dafilesrv

        remapGroupsToDafilesrv(tree, false, secure);
    }

    bool isSuper = streq(tree->queryName(), queryDfsXmlBranchName(DXB_SuperFile));

    IPropertyTree *fileMeta = meta->addPropTree("FileMeta");
    fileMeta->setProp("@name", logicalName);

    // 1) establish lock 1st (from Dali)
    // TBD
    unsigned __int64 lockId = getLockId(leaseId);
    fileMeta->setPropInt64("@lockId", lockId);

    if (isSuper)
    {
        fileMeta->setPropBool("@isSuper", true);
        unsigned n = tree->getPropInt("@numsubfiles");
        if (n)
        {
            Owned<IPropertyTreeIterator> subit = tree->getElements("SubFile");
            // Adding a sub 'before' another get the list out of order (but still valid)
            OwnedMalloc<IPropertyTree *> orderedSubFiles(n, true);
            ForEach (*subit)
            {
                IPropertyTree &sub = subit->query();
                unsigned sn = sub.getPropInt("@num",0);
                if (sn == 0)
                    ThrowStringException(-1, "CDistributedSuperFile: SuperFile %s: bad subfile part number %d of %d", logicalName, sn, n);
                if (sn > n)
                    ThrowStringException(-1, "CDistributedSuperFile: SuperFile %s: out-of-range subfile part number %d of %d", logicalName, sn, n);
                if (orderedSubFiles[sn-1])
                    ThrowStringException(-1, "CDistributedSuperFile: SuperFile %s: duplicated subfile part number %d of %d", logicalName, sn, n);
                orderedSubFiles[sn-1] = &sub;
            }
            for (unsigned i=0; i<n; i++)
            {
                if (!orderedSubFiles[i])
                    ThrowStringException(-1, "CDistributedSuperFile: SuperFile %s: missing subfile part number %d of %d", logicalName, i+1, n);
            }
            StringBuffer subname;
            for (unsigned f=0; f<n; f++)
            {
                IPropertyTree &sub = *(orderedSubFiles[f]);
                sub.getProp("@name", subname.clear());
                populateLFNMeta(userDesc, subname, leaseId, opts, metaRoot, fileMeta);
            }
        }
    }
    fileMeta->setPropTree(tree->queryName(), tree.getLink());
    Owned<IPropertyTreeIterator> clusterIter = tree->getElements("Cluster");
    ForEach(*clusterIter)
    {
        IPropertyTree &cluster = clusterIter->query();
        const char *planeName = cluster.queryProp("@name");
        VStringBuffer planeXPath("planes[@name='%s']", planeName);
        if (!metaRoot->hasProp(planeXPath))
        {
            VStringBuffer storagePlaneXPath("storage/%s", planeXPath.str());
            Owned<IPropertyTree> dataPlane = getGlobalConfigSP()->getPropTree(storagePlaneXPath);

            const char *hostGroupName = dataPlane->queryProp("@hostGroup");
            if (!isEmptyString(hostGroupName))
            {
                Owned<IPropertyTree> hostGroup = getHostGroup(hostGroupName, false);
                if (hostGroup)
                {
                    // This is only likely to be used if this service is in BM
                    // Cloud based storage planes are unlikely to be backed by hosts/hostGroups
                    dataPlane.setown(createPTreeFromIPT(dataPlane));
                    unsigned daFsSrvPort = getPreferredDaFsServerPort();
                    Owned<IPropertyTreeIterator> iter = hostGroup->getElements("hosts");
                    ForEach(*iter)
                    {
                        VStringBuffer endpoint("%s:%u", iter->query().queryProp(nullptr), daFsSrvPort);
                        dataPlane->addProp("hosts", endpoint);
                    }
                    dataPlane->removeProp("@hostGroup");
                }
            }
            metaRoot->addPropTree("planes", dataPlane.getClear());
        }
    }
}


void CWsDfsEx::init(IPropertyTree *cfg, const char *process, const char *service)
{
    DBGLOG("Initializing %s service [process = %s]", service, process);
    VStringBuffer xpath("Software/EspProcess/EspBinding[@service=\"%s\"]/@protocol", service);
    isHttps = strsame("https", cfg->queryProp(xpath));
}

bool CWsDfsEx::onGetLease(IEspContext &context, IEspLeaseRequest &req, IEspLeaseResponse &resp)
{
    unsigned timeoutSecs = req.getKeepAliveExpiryFrequency();

    // TBD will get from Dali.
    resp.setLeaseId(1);
    return true;
}

bool CWsDfsEx::onKeepAlive(IEspContext &context, IEspKeepAliveRequest &req, IEspKeepAliveResponse &resp)
{
    return true;
}

bool CWsDfsEx::onDFSFileLookup(IEspContext &context, IEspDFSFileLookupRequest &req, IEspDFSFileLookupResponse &resp)
{
    try
    {
        const char *logicalName = req.getName();

        StringBuffer userID;
        context.getUserID(userID);
        Owned<IUserDescriptor> userDesc;
        if (!userID.isEmpty())
        {
            userDesc.setown(createUserDescriptor());
            userDesc->set(userID.str(), context.queryPassword(), context.querySignature());
        }

        // LDAP scope check
        checkLogicalName(logicalName, userDesc, true, false, false, nullptr); // check for read permissions

        unsigned timeoutSecs = req.getRequestTimeout();
        unsigned __int64 leaseId = req.getLeaseId();

        // populate file meta data and lock id's
        LfnMetaOpts opts = LfnMOptNone;
        if (req.getAccessViaDafilesrv())
            opts |= LfnMOptRemap;

        if (isContainerized())
        {
            // NB: if we ever have some services with tls, and some without in bare-metal, this may need revisiting.
            if (getComponentConfigSP()->getPropBool("@tls"))
                opts |= LfnMOptTls;
        }
        else if (isHttps)
            opts |= LfnMOptTls;

        Owned<IPropertyTree> responseTree = createPTree();
        populateLFNMeta(userDesc, logicalName, leaseId, opts, responseTree, responseTree);

        // serialize response
        MemoryBuffer respMb, compressedRespMb;
        responseTree->serialize(respMb);
        fastLZCompressToBuffer(compressedRespMb, respMb.length(), respMb.bytes());
        StringBuffer respStr;
        JBASE64_Encode(compressedRespMb.bytes(), compressedRespMb.length(), respStr, false);
        resp.setMeta(respStr.str());

        if (responseTree->hasProp("FileMeta")) // otherwise = not found
        {
            // update file access.
            //    Really this should be done at end (or at end as well), but this is same as existing DFS lookup.
            CDateTime dt;
            dt.setNow();
            queryDistributedFileDirectory().setFileAccessed(userDesc, logicalName, dt);

            LOG(MCauditInfo,",FileAccess,EspProcess,READ,%s,%u,%s", logicalName, timeoutSecs, userID.str());
        }
    }
    catch (IException *e)
    {
        FORWARDEXCEPTION(context, e,  ECLWATCH_INTERNAL_ERROR);
    }
    return true;
}

