
from re import sub
import subprocess
import os
import logging

logger = logging.getLogger(__name__)

class hpccpkg:

    def build(self):
        command = f"docker build -t {self.tag} \
            --build-arg branch={self.branch} \
            --build-arg repository={self.repository} \
            --build-arg cmake_build_type={self.cmake_build_type} \
            --build-arg check_git_tag={str(self.check_git_tag)} \
            {self.path}"
        logger.info(f"building tag {self.tag}")
        logger.debug(f"running command - {command}")
        try:
            process = subprocess.run(command.split(), timeout=1200)
            logger.debug(process.stdout)
            if process.check_returncode():
                logger.warning(process.stderr)
        except Exception as e:
            logger.exception(e)

    def copy(self):
        base_tag = str(self.tag).replace('baremetal-','')
        logger.info(f"creating temporary container tmp-{base_tag}")
        create_container = f"docker run --name tmp-{base_tag} {self.tag} /bin/true"
        try:
            process = subprocess.run(create_container.split(), timeout=15)
            logger.debug(process.stdout)
            if process.check_returncode():
                logger.warning(process.stderr)
        except Exception as e:
            logger.exception(e)
        logger.info(f"copying artifacts for {self.tag} to local storage")
        copy_artifact = f"docker cp tmp-{base_tag}:/home/hpccbuild/package ."
        try:
            process = subprocess.run(copy_artifact.split(), timeout=300)
            logger.debug(process.stdout)
            if process.check_returncode():
                logger.warning(process.stderr)
        except Exception as e:
            logger.exception(e)

    def cleanup(self):
        base_tag= str(self.tag).replace('baremetal-','')
        logger.info(f"removing temporary container tmp-{base_tag}")
        cleanup_container = f"docker rm tmp-{base_tag}"
        try:
            process = subprocess.run(cleanup_container.split(), timeout=120)
            logger.debug(process.stdout)
            if process.check_returncode():
                logger.warning(process.stderr)
        except Exception as e:
            logger.exception(e)

    def __init__(self, path, tag, log=True, repository='hpcc-systems', branch='master',
        check_git_tag=False, cmake_build_type='Release'):
        self.path = path
        self.tag = tag
        self.repository = repository
        self.branch = branch
        self.check_git_tag = check_git_tag
        self.cmake_build_type = cmake_build_type

        