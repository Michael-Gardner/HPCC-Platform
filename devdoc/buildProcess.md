# Navigating the Platform Build System

Over the past two years we have been developing a new build system that is more open and reviewable. In this lecture we will discuss; changes to our build machine image generation, leveraging vcpkg for library dependencies, our builds on Github Actions and how to utilize our build workflow code to generate your own custom builds.

## Overview:

Our goal was to move from an on premises CI/CD solution, to one that we could host anywhere and have improvements to it saved within a version control system.
The current development to product pipeline consists of leveraging Github Actions as our main CI/CD tool. Previously we had relied upon an on prem Jenkins.  

Primary CI/CD tool: Jenkins → Github Actions
Build images: Amazon Machine Images → Docker Images
Build dependencies: wiki pages → vcpkg submodule in our code base

### Old System

The original build system was hosted on-prem using Jenkins and physical servers, and then openstack virtualization. This meant that all of our 'build' infrastructure was hidden from the public and many platform developers. The ability to build, deploy/publish wasn't a transparent process to the entire team.The builds were all based off of Job Templates that were stored on Jenkins.  Jenkins backup was cumbersome.

### The New System

The new system leverages new technologies in the 'cloud'.  We use Github Actions, Docker, Github SCM to automate publishing to both Github         

## VCPKG

Vcpkg is a microsoft tool for package management for c/c++ projects.  It contains libraries and their source reposotories so that developers can access bug fixes and security features as quickly as possible, without relying on ditribution package management systems.  This helps our developers with dependencies both on the build side, as well as packaging up-to-date libraries for runtime dependencies.

Before vcpkg, we relied on distribution package managers for both build and runtime dependencies. This required us to keep lists of dependencies per individual distribution for runtime, as well as a wiki for building the platform that held information on the build utilities and necessary libraries that had to be on the build environment before a developer could even try to build the HPCC-Platform.  The largest downside to this was the possibility that a HPCC-Platform package that we produced, could have runtime issues well after release, that didn’t occur on release, because of package changes on an individual distributions package management system.

Using vcpkg does cause our final HPCC-Platform package to be larger than it was when relying on distribution package management systems. This is because we are both building against, and in many cases building to install, third party libraries that are necessary to link against for runtime operations.  But this ensures that all of our necessary runtime libraries are always on disc and available to the Platform.  It’s been a good tradeoff for improving the simplicity of the build process, and robustness of the Platform runtime.

The current build cycle for updating vcpkg is quarterly. With every Minor release version, we update the vcpkg submodule in our HPCC-Platform repository with the latest Microsoft tag for vcpkg. We then rebuild our vcpkg cache images and do a set of tests to ensure that the platform will build properly with the updated set of libraries.  We do this a full month before release of each Minor bump to ensure that any function calls in jlib or other platform libraries will be updated in time for the release.  We document how to update our vcpkg submodule in the top level README of the hpcc-systems/vcpkg repository.

The hpcc-systems/vcpkg package repository has automated actions that run to generate our hpccsystems/platform-build-base-<distrobution>:hpcc-platform-<version> docker images.  These base images contain a full build and installation of vcpkg and build-tools necessary in compiling our HPCC-Platform source.  These base images get regenerated any time there are changes to our hpcc-systems/vcpkg repository, for every linux distribution that we target.  This caches NuGet tools and packages that coincide with the baseline hash for our vcpkg submodule.  Cacheing in this manner greatly cuts down on build time for the platform.

## Platform Generation

With our build-base images generated, our source code can now be built.  To get a general idea of the process, one can view the build-assets.yml workflow file in the hpcc-systems/HPCC-Platform repository under the .github/workflows directory.  The first step is to use the previously generated platform-build-base image to create a platform-build image for each target distribution.  The dockerfiles for this can be found under HPCC-Platform/dockerfiles/vcpkg/<distribution>.dockerfile.  These files generally just match the vcpkg submodules SHA with the SHA we tagged for platform-build-base, to ensure we are referencing the correct base image.

We then mount our source code and build directory from the github action runner into the platform-build image and utilize cmake to generate our final package.  This means our final product remains on our github action runner, whilst we have total control over the actual build environment.

Within our github actions workflows, we generate all our distribution packages for the Platform, Client tools, various plugins, ECLIDE and documentation.  We also generate a docker image based on Ubuntu-22.04 for the platform that is designed to run in a containerized environment.

We automate the publishing of all these assets to several location.
* The [Release page](https://github.com/hpcc-systems/HPCC-Platform/releases) of our organizations hpcc-systems/HPCC-Platform repository
* Internal JFrog Repositories for deb, rpm, macos, windows and docker containers
* The [HPCCSystems](https://hpccsystems.com/download) website



Currently, the only manual processes that need to be done by a developer are
* setting up the jenkins job to match the tag for older builds (8 series or earlier than 9.4)
* changelogs on jenkins
* fetch to fileserver
* package manifest and publish to HPCCSystems website


## Build Asset Workflow in detail

Github actions uses workflow files to describe the action to be taken in each branch.  A master branch workflow file allows the workflow to be ran in subsequent branches, but the workflow file in the branch being referenced will control the logic of the build.  This means we can tailor each branch to the time of release.  A candidate-9.2.x branch will have os targets of centos-7, centos-8, ubuntu-20.04 etc because those os targets existed when we cut the branch.  A candidate-9.8.x will not have any centos targets, but instead will have a rockylinux-8 os target, because at the time of cutting that branch, centos was End of Life.  Originally this functionality was mimiced in Jenkins with a set of templates that had no scm and a cumbersome backup system.  Now it's easily viewable in our scm for all of our developers to critique.

The build-assets.yml workflow has several jobs that must be successful for a complete build.

First is the preamble. This job sets up several environmental variables that will be referenced throughout the workflow. It also sets up the Release page for the github.ref tag.

Second we have the build-docker job.  This is dependent upon the preamble job and is designed to generate all of our linux based packages.  The final output of this job will be to publish releases of our deb and rpm packages to the locations previously mentioned.
* make room on the runner by cleaning up disk space
* checkout the HPCC-Platform and LN repositories with the appropriate community and internal refs
* calculate the SHA for the vcpkg submodule within the HPCC-Platform source code

