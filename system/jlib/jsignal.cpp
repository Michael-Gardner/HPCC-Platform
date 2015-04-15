/*##############################################################################

    HPCC SYSTEMS software Copyright (C) 2012 HPCC Systems.

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
#include "jsignal.hpp"
#include "jlib.hpp"

SignalElement::SignalElement(int _signo, sighandler_t _handle)
{
#if defined(_WIN32)
    signal(signo, _handle);
#else
    signo = _signo;
    if ( signo == SIGTERM )
        sigfillset(&action.sa_mask);
    else
        sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
    action.sa_handler = _handle;
    sigaction(signo, &action, NULL);
#endif
}

SignalElement::~SignalElement()
{
#if defined(_WIN32)
    signal(signo,SIG_DFL);
#else
    action.sa_handler = SIG_DFL;
    sigaction(signo, &action, NULL);
#endif
}

void SignalElement::modifyHandler(sighandler_t _handle)
{
#if defined(_WIN32)
    signal(this->getSigno(), _handle);
#else
    action.sa_handler = _handle;
    sigaction(signo, &action, NULL);
#endif
}

void SignalElement::reset()
{
#if defined(_WIN32)
    signal(this->getSigno(), SIG_DFL);
#else
    action.sa_handler = SIG_DFL;
    sigaction(signo, &action, NULL);
#endif
}

int SignalElement::getSigno() const
{
    return signo;
}

bool SignalElement::isSigno(int _signo) const
{
    if (signo == _signo)
        return true;
    return false;
}

void Signal::add(int _signo, sighandler_t _handle)
{
    ForEachItemIn(idx, Signal::elementlist)
    {
        if (elementlist.item(idx).isSigno(_signo))
        {
            elementlist.item(idx).modifyHandler(_handle);
            return;
        }
    }
    elementlist.append(*new SignalElement(_signo, _handle));
}

void Signal::remove(int _signo)
{
    ForEachItemInRev(idx, elementlist)
    {
        if (elementlist.item(idx).isSigno(_signo))
            elementlist.item(idx).reset();
    }
}

Signal & Signal::getSignalObject()
{
    static Signal s_object;
    return s_object;
}

// TODO: check windows and osx implementation of SIGABRT and how to appropriately raise()
// method used to create core dump and exit the program on error
void Signal::coreDump()
{
#if defined(__linux__) || defined(__APPLE__)
    Signal & signals = Signal::getSignalObject();
    ForEachItemInRev(idx, signals.elementlist)
    {
        if (signals.elementlist.item(idx).isSigno(SIGABRT))
        {
            signals.elementlist.item(idx).reset();
            break;
        }
    }
    // abort basically does a raise(SIGABRT) so we reset the SIGABRT handler to
    // its default before calling
    abort();
#endif
}
