#!/usr/bin/env python3

import logging
import logging.config
import os
import time
import yaml
import argparse
from hpccpkg import hpccpkg

def main():

    # for execution time debugging
    start_time = time.time()

    # load and setup logging
    logging_conf_path = 'logging.yaml' 
    if os.path.exists(logging_conf_path):
        with open(logging_conf_path, 'r') as f:
            try:
                config = yaml.safe_load(f.read())
                logging.config.dictConfig(config)
            except Exception as e:
                print("Error in loading configuration yaml file, using basic config")
                print(e.with_traceback)
                logging.basicConfig(level=logging.INFO)
    log = logging.getLogger('pkggenLogger')

    # argument handling
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("-t", "--target", default="all",
        help='''
            Target distribution for build
            default: all
            example: ubuntu1804, centos7
            ''')
    parser.add_argument("-p", "--package", default="all",
        help='''
        Package to build on [target]
        default: all
        example: platform, clienttools, rembed
        ''')
    parser.add_argument("-b", "--branch", help="branch to build", default="master")
    parser.add_argument("-r", "--repository", default="hpcc-systems",
        help="repository for HPCC-Platform source")
    parser.add_argument("-c", "--check", action="store_true",
        help="check git tag on build")
    parser.add_argument("-v", "--verbose", help="give more verbose output",
        action="store_true")
    parser.add_argument("-q", "--quiet", help="squelch console output",
        action="store_true")
    args = parser.parse_args()

    targets = []
    packages = []

    if args.verbose:
        log.setLevel(logging.DEBUG)
        log.debug('set log level to DEBUG')
    if args.quiet:
        log = logging.getLogger()
    if args.target == 'all':
        for i in os.listdir():
            if os.path.isdir(i) and i != 'package':
                targets.append(i)
                log.info(f'adding {i} to target list')
    else:
        targets.append(args.target)
    log.debug(f'targets = {targets}')

    branch = args.branch
    log.debug(f'branch = {branch}')

    repository = args.repository
    log.debug(f'repository = {repository}')

    check_git_tag = args.check
    log.debug(f'check_git_tag = {str(check_git_tag)}')

    for t in targets:
        # regenerate build container
        log.warning("testing output")
        log.info(f"regenerating build container-{t}")
        base_container = hpccpkg(t, f"container-{t}" , branch=branch,
            repository=repository)
        base_container.build()
        if args.package != 'all':
            packages.append(args.package)
        else:
            for d in os.listdir(t):
                if os.path.isdir(f"{t}/{d}"):
                    packages.append(d)
        log.debug(f"building package {packages}")
        for p in packages:
            package = hpccpkg(f"{t}/{p}", f"baremetal-{t}-{p}", branch=branch,
                repository=repository)
            package.build()
            package.copy()
            package.cleanup()

#    time.sleep(2)
    execution_time = time.time() - start_time
    log.debug(f'execution time in {execution_time} seconds')

if __name__ == '__main__':
    main()