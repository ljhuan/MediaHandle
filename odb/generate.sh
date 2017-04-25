#!/bin/bash

GEN="odb --std c++11  --generate-query --generate-schema"
DB="sqlite"

${GEN} -d ${DB} result.hxx
${GEN} -d ${DB} task.hxx

