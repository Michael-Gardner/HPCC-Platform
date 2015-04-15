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

#ifndef __JSIGNAL__
#define __JSIGNAL__

#include "platform.h"
#include "jiface.hpp"
#include "jlib.hpp"

#ifdef _WIN32
typedef void (*sighandler_t)(int);
#endif

class SignalElement : public CInterface
{
public:
    SignalElement(int,sighandler_t);
    ~SignalElement();
    void reset();
    void modifyHandler(sighandler_t);
    int  getSigno() const;
    bool isSigno(int) const;

private:
    int signo;
#if defined(__linux__) || defined(__APPLE__)
    struct sigaction action;
#endif
};

// Singleton class to only allow one instance per process
// need to insure it is thread safe.
class Signal
{
public:
    static Signal & getSignalObject();
    void add(int , sighandler_t);
    void remove(int);
    static void coreDump();

private:
    Signal() {};
    Signal(Signal const &);
    void operator=(Signal const &);
    CIArrayOf<SignalElement> elementlist;
};
#endif //__JSIGNAL__
