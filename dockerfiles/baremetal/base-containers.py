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
import os
import argparse

def main():

    target_platforms = []

    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawTextHelpFormatter
    )
    parser.add_argument("-t", "--target", default="all",
        help='''
            Target distribution for build
            default: all
            example: ubuntu1804, centos7
            '''
        )
    parser.add_argument("-p", "--push", default=False, action='store_true',
        help='''
            Push image to dockerhub
            default: False
            ''')
    args = parser.parse_args()
    if args.target == 'all':
        target_platforms = ['centos7', 'centos8', 'ubuntu1804', 'ubuntu2004', 'ubuntu2204']
    else:
        target_platforms.append(args.target)

    for target in target_platforms:
        print(f"Launching docker build of hpcc-build-base-{target} ...", end='',
            flush=True)
        try:
            cwd = f"{os.getcwd()}/{target}"
            command = f"docker build -t memeoru/hpcc-build-base:{target} ."
            process = subprocess.run(command.split(),
            text=True, check=True, cwd=cwd)
            print(" success")
            if args.push:
                command = f"docker push memeoru/hpcc-build-base:{target}"
                process = subprocess.run(command.split(), text=True,
                    check=True, cwd=cwd)
        except subprocess.CalledProcessError as e:
            print(" failed")
            print(e.stderr)

if __name__ == '__main__':
    main()
