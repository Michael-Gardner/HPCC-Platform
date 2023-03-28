---
title: Thor
---

Introduction
============

Workunits In Thor
-----------------

Thor breaks down workunits into several different distinct jobs. These jobs are
referred to as graphs and contain a set of activities.  Graphs can contain
'edges'. An edge is an information stream that will be output to any other
graphs connected to that edge.

A workunit will generally contain many graphs and subgraphs. But Thor Workers
processes each graph as it's own job and then connects the edges as the Thor
Manager works its way down the dependency tree.

Thor Manager
------------

Thor has two slightly different code flows that it goes through to initialize
and startup, depending on if it is running on baremetal or if it's compiled to
run on a containerized environment. The Manager startup code can be located in
thorlcr/master/thmastermain.cpp

The flow of the Thor Manager is roughly
-   Initialize manager process
    -   Load configuration and ensure defaults
    -   Setup socket endpoint
    -   Logging
    -   Setup memory management values for Manager and Workers
-   Read from Thor Queue
    -   Log workunit ID
    -   Setup Graph (for worker as well?)
-   Register worker processes
-   Step into thorMain() with workunit and graph contexts

From here we step into thorlcr/master/thgraphmanager.cpp.
-   Set memory warning threshhold
-   Connect with Dali
-   Initialize File Manager
-   CJobManager initialized and ran.
    -   Docker containers use CJobManager->run()
    -   Baremetal utilizes CJobManager->execute()



Thor Worker
-----------


Activities
----------

Activities make up a graph. They are the individual operations that are
necessary to achieve a graphs desired state.

Activities can be found in thorlcr/activities.  The following are a list of the
current available activities in the master branch.

* action
* aggregate
* apply
* catch
* choosesets
* counterproject
* csvread
* degroup
* diskread
* diskwrite
* distribution
* enth
* external
* fetch
* filter
* firstn
* funnel
* group
* hashdistrib
* indexread
* indexwrite
* iterate
* join
* keydiff
* keyedjoin
* keypatch
* limit
* lookupjoin
* loop
* merge
* msort
* normalize
* nsplitter
* null
* nullaction
* parse
* piperead
* pipewrite
* project
* pull
* result
* rollup
* sample
* selectnth
* selfjoin
* soapcall
* splill
* temptable
* topn
* trace
* when
* wuidread
* wuidwrite
* xmlparse
* xmlread
* xmlwrite


### Memory Management in Thor