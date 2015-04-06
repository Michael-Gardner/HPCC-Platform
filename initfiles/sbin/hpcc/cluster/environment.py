'''
/*#############################################################################

    HPCC SYSTEMS software Copyright (C) 2012 HPCC Systems.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
############################################################################ */
'''

from xml.etree.ElementTree import ElementTree

class EnvironmentParser(object):
    '''
    This class is used for parsing an environment.xml file
    for the HPCCSystems Platform.
    '''
    
    def __init__(self, environment='/etc/HPCCSystems/environment.xml'):
        '''
        Constructor
        '''
        self.environment = environment
        self.tree = ElementTree()
        self.tree.parse(self.environment)

    def getAttribute(self, nodeName, attribute):
        nodes   = self.tree.getiterator(nodeName)
        return [ element.attrib[attribute] for element in nodes ] 

    def machines(self):
        return self.getAttribute("Computer", "netAddress")
