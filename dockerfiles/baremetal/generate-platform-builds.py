#! /usr/bin/env python3

##############################################################################
#
#    HPCC SYSTEMS software Copyright (C) 2022 HPCC Systems®.
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
##############################################################################

import os
import subprocess

#targets = ["centos7", "centos8", "ubuntu1804", "ubuntu2004"]
targets = ["centos7"]

for target in targets:
    print(f"Launching platform build on {target} target ...", end='',
        flush=True)
    print(" active")
    working_directory = os.getcwd()
    working_directory = f"{working_directory}/platform/{target}"
    command = f"docker build -t baremetal-platform-{target} \
        --build-arg branch=candidate-8.6.x ."
    
    process = subprocess.run(command.split(), cwd=working_directory)