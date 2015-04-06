#!/usr/bin/env python

import os
import sys
import shutil
import getopt
import subprocess
import signal
from hpcc.cluster.host import Host
from hpcc.cluster.task import ScriptTask
from hpcc.cluster.thread import ThreadWithQueue



def signal_handler(signal, frame):
    print "\nCaught signal " + signal + "\n"
    os._exit(signal)


def usage():
    print "install-cluster.py [-e|--environment <environment.xml>] -p|--package <package>"
    print "[-e|--environmen] path to environment.xml file (default=/etc/HPCCSystems/environment.xml)"
    print "[-p|--package]  path to package"

def generate_key():
    os.mkdir(new_keys)
    cmd = 'ssh-keygen -t rsa -f ' + new_keys + '/id_rsa -P ""'
    print cmd
    os.system(cmd)

def create_payload():
    shutil.rmtree(remote_install,True)
    os.mkdir(remote_install)
    
    if keygen:
        generate_key()

    print "Package = " + package
    print "remote_install = " + remote_install
    shutil.copy(package,remote_install)
    shutil.copy(env_cnf,remote_install)
    shutil.copy(env_xml,remote_install)
    shutil.copy(script, remote_install)

    cmd = "tar -zcvf /tmp/remote_install.tgz " + remote_install + "/*"
    os.system(cmd)
    shutil.rmtree(remote_install,True)

def remove_payload():
    payload = remote_install + ".tgz"
    shutil.rmtree(payload,True)

signal.signal(signal.SIGINT,  signal_handler)
signal.signal(signal.SIGQUIT, signal_handler)
signal.signal(signal.SIGTERM, signal_handler)

remote_install  = "/tmp/remote_install"
new_keys        = remote_install + "/new_keys"
env_xml         = None
env_cnf         = None
package         = None
user            = None
password        = None
keygen          = False
script          = "./remote-install-engine.sh"

try:
    opts, args = getopt.getopt(sys.argv[1:],"h?ke:c:u:p:",
        ["help", "user=", "keygen", "package=", "environment=", "conf="])

except getopt.GetoptError as err:
    print(str(err))
    exit(1)

for arg, value in opts:
    if arg in ("-?", "-h","--help"):
        usage()
        exit(0)
    elif arg in ("-e", "--environment"):
        env_xml = os.path.abspath(value)
    elif arg in ("-p", "--package"):
        package = os.path.abspath(value)
    elif arg in ("-u", "--user"):
        user = value
    elif arg in ("-k", "--keygen"):
        keygen = True
    elif arg in ("-c", "--conf"):
        env_cnf = os.path.abspath(value)
    else:
        print "\nUnknown option " + arg
        usage()
        exit(1)

os.chdir(os.path.dirname(sys.argv[0]))

if not package:
    print "Error: Package not found"
    exit(1)

if not user:
    user = raw_input("Administrative User: ")

if not password:
    password = raw_input("Password (leave blank if using key): ")

if not env_xml:
    env_xml = os.path.abspath("./environment.xml")

if not env_cnf:
    env_cnf = os.path.abspath("./environment.conf")

options = "-l DEBUG -e " + env_cnf + " -s DEFAULT"
script  = os.path.abspath(script)

os.environ['ADMIN_USER'] = user
os.environ['PASS']       = password
os.environ['CMDPREFIX']  = "sudo"
os.environ['REMOTE_INSTALL'] = remote_install
os.environ['PKG'] = package

create_payload()

proc = subprocess.Popen("md5sum install-hpcc.exp | cut -d' ' -f1", stdout=subprocess.PIPE, shell=True)
checksum = proc.communicate()[0].strip()
cmd = os.path.abspath("./cluster_script.py") + " --env_conf " + env_cnf  + " --env_xml " + env_xml + " -f " + os.path.abspath("./install-hpcc.exp") + " -c " + checksum + " " + options
print cmd
os.system(cmd)

remove_payload()
