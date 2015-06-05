#!/bin/bash
################################################################################
#    HPCC SYSTEMS software Copyright (C) 2012 HPCC Systems.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
################################################################################

# Reads the Pidfile if Pidfile exists
getPid()
{
    local pidFile=$1
    if [[ -e $pidFile ]]; then
        __pidValue=$(/bin/cat $pidFile)
        return 1
    else
        __pidValue=0
        return 0
    fi
}

createPid()
{
    local pidFile=$1
    local pidValue=$2
    if [[ -e $pidFile ]]; then
        log "Pidfile already exists"
        return 1
    else
        echo $pidValue > $pidFile
        if [[ -e $pidFile ]]; then
            log "Created Pidfile $pidFile"
            return 0
        else
            log "Failed to create Pid"
            return 1
        fi
    fi
}

removePid()
{
    local pidFile=$1
    getPid $pidFile
    if [[ $? -eq 0 ]]; then
        return 0
    else
        checkPidRunning $pidFile
        if [[ $? -eq 0 ]]; then
            log "Process ($__pidValue) is still running.  Failed to remove Pid File" 
            return 1
        else
            rm -rf $pidFile > /dev/null 2>&1
            if [[ $? -ne 0 ]]; then
                log "Error attempting to remove $pidFile"
                return 1
            fi
            log "Successfully removed pid file: $pidFile"
            return 0
        fi
    fi
}

checkPidRunning()
{
    getPid $1
    if [[ $? -ne 0 ]]; then
        kill -0 $__pidValue > /dev/null 2>&1
        return $rcval
    else
        return 1
    fi
}

checkSentinelFile()
{
    local filecheck=$(find ${runtime}/${compName} -name "*sentinel" 2> /dev/null)
    [[ -n "${filecheck}" ]] && return 0
    return 1
}

removeSentinelFile()
{
    checkSentinelFile
    if [[ $? -eq 1 ]]; then
        log "Sentinel file not found, cannot remove $sentinelFile"
        return 1
    fi
    local sentinelFile=$(find ${runtime}/${compName} -name "*sentinel" 2> /dev/null)
    rm -f $sentinelFile > /dev/null 2>&1
    if [[ $? -ne 0 ]]; then
        log "Failed to remove sentinel file $sentinelFile"
        return 1
    fi
    log "removed sentinel file $sentinelFile"
    return 0
}

#    check_status
#    Parameters:
#       1: PID FILE PATH       (string)
#       2: COMP PID FILE PATH  (string)
#       3: SENTINEL FILE CHECK (bool)
#               1 = check
#    Return:
#       0: Running Healthy
#       1: Stopped Healthy
#       2: Process starting up
#       3: Orphaned Process
check_status()
{
    local PIDFILEPATH=$1
    local COMPPIDFILEPATH=$2
    local SENTINELFILECHK=$3

    checkPidRunning $PIDFILEPATH
    local initRunning=$?
    checkPidRunning $COMPPIDFILEPATH
    local compRunning=$?
    checkSentinelFile
    local sentinelFlag=$?

    # check if running and healthy
    if [[ $initRunning -eq 0 ]] && [[ $compRunning -eq 0 ]]; then
        if [[ $SENTINELFILECHK -eq 1 ]]; then
            [[ $DEBUG != "NO_DEBUG" ]] && echo "everything is up except sentinel"
            log "$compName ---> Waiting on Sentinel"
            if [[ $sentinelFlag -eq 1 ]]; then
                [[ $DEBUG != "NO_DEBUG" ]] && echo "Sentinel not yet located, process in startup"
                log "$compName ---> Process in startup"
                log "$compName ---> Waiting on sentinel file"
                return 2 
            fi
        fi
        [[ $DEBUG != "NO_DEBUG" ]] && echo "Sentinel is now up"
        log "$compName ---> Process is up"
        return 0
    # check if shutdown and healthy
    elif [[ $initRunning -eq 1 ]] && [[ $compRunning -eq 1 ]]; then
        if [[ $SENTINELFILECHK -eq 1 && $sentinelFlag -eq 0 ]]; then
            log "$compName ---> Sentinel File found, but processes are dead"
        fi
        [[ $DEBUG != "NO_DEBUG" ]] && echo "Process is down"
        log "$compName ---> Process is down"
        return 1
    # component is orphaned somehow
    elif [[ $initRunning -eq 1 ]] && [[ $compRunning -eq 0 ]]; then
        log "$compName ---> Process is orphaned"
        return 3
    # Init process is already spawned
    elif [[ $initRunning -eq 0 ]] && [[ $compRunning -eq 1 ]]; then
        log "$compName ---> Process in startup"
        return 2
    fi
}
