#! /usr/bin/env python3

import subprocess

target_platforms = ['centos7', 'centos8', 'ubuntu1804', 'ubuntu2004']

for target in target_platforms:
    command = f"docker build -t container-{target} {target}"
    process = subprocess.Popen(command.split(), stdout=subprocess.PIPE)
    output, error = process.communicate()