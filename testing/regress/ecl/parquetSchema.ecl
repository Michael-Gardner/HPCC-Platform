/*##############################################################################
    HPCC SYSTEMS software Copyright (C) 2024 HPCC Systems®.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
       http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
############################################################################## */

//class=parquet
//nothor
//noroxie

IMPORT Std;
IMPORT Parquet;

Layout1 := RECORD
    INTEGER id;
    STRING name;
    REAL salary;
END;

Layout2 := RECORD
    STRING name;
    REAL salary;
    INTEGER id;
END;

testData := DATASET([
    { 1, 'Alice', 50000.50 },
    { 2, 'Bob', 60000.75 }
], Layout1);

basePath := Std.File.GetDefaultDropZone() + '/regress/parquet/';
FilePath := basePath + 'reorder_test.parquet';

ParquetIO.Write(testData, filePath, TRUE);

readData := ParquetIO.Read(Layout2, filePath);

OUTPUT(readData, NAMED('ReadData'));
