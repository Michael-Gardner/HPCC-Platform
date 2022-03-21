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

import subprocess

target_platforms = ['centos7', 'centos8', 'ubuntu1804', 'ubuntu2004']

for target in target_platforms:
    print(f"Launching docker build of container-{target} ...", end='',
        flush=True)
    try:
        command = f"docker build -t container-{target} {target}"
        process = subprocess.run(command.split(), capture_output=True,
        text=True, check=True, timeout=600)
        print(" success")
    except subprocess.CalledProcessError:
        print(" failed")
        print(process.stderr)
    except subprocess.TimeoutExpired:
        print(" timeout")
        print(process.stdout)
