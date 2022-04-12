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

# full suite of builds
#targets = ["centos7", "centos8", "ubuntu1804", "ubuntu2004"]
#builds = ["platform", "platform-with-plugins"
#    "clienttools", "rembed", "memcached", "sqs", "redis", "mysqlembed",
#    "javaembed", "sqlite3embed", "kafka", "couchbaseembed"]

# for individual tests uncomment
builds = ["platform-with-plugins"]
targets = ["ubuntu2204"]

repository = "michael-gardner"
branch = "fix-dali-nonnull-errors"
check_git_tag = 0

print(f"Building -- {branch} ...")

working_directory = os.getcwd()
for target in targets:
    target_path = F"{working_directory}/{target}"
    if not os.path.isdir(target_path):
        print(f"this path did not exit {target_path}")
        continue
    print(f"this path existed {target_path}")
    for build in builds:
        path = f"{working_directory}/{target}/{build}/Dockerfile"
        if not os.path.isfile(path):
            print(f"this path did not exist {path}")
            continue
        print(f"this path existed {path}")
        print(f"Launching {build} build on target {target}")
        target_build_dockerfile_path = f"{working_directory}/{target}/{build}"
        command = f"docker build -t baremetal-{target}-{build} \
            --build-arg branch={branch} \
            --build-arg repository={repository} \
            --build-arg check_git_tag={check_git_tag} ."
    
        process = subprocess.run(command.split(),
            cwd=target_build_dockerfile_path)

        print(f"Creating temporary container -- tmp-{target}-{build}")
        create_container = f"docker run --name tmp-{target}-{build} \
            baremetal-{target}-{build} /bin/true"
        process = subprocess.run(create_container.split())

        print(f"Copying artifacts for {target}-{build} to local storage")
        copy_artifact = f"docker cp tmp-{target}-{build}:/home/hpccbuild/package ."
        process = subprocess.run(copy_artifact.split())

        print(f"Removing temporary container -- ", end='', flush=True)
        cleanup_container = f"docker rm tmp-{target}-{build}"
        process = subprocess.run(cleanup_container.split())
