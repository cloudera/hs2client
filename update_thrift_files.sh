#! /usr/bin/env bash
# Copyright 2014 Cloudera Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

if [ -z "$IMPALA_REPO" ]; then
    echo "Need to set IMPALA_REPO"
    exit 1
fi

if [ -z "$LIBHS2CLIENT_REPO" ]; then
    echo "Need to set LIBHS2CLIENT_REPO"
    exit 1
fi

OUT_DIR=$LIBHS2CLIENT_REPO/thrift

echo "copying thrift files from the main Impala repo"
cp $IMPALA_REPO/common/thrift/TCLIService.thrift $OUT_DIR
cp $IMPALA_REPO/common/thrift/beeswax.thrift $OUT_DIR
cp $IMPALA_REPO/common/thrift/ImpalaService.thrift $OUT_DIR
cp $IMPALA_REPO/common/thrift/ErrorCodes.thrift $OUT_DIR
cp $IMPALA_REPO/common/thrift/ExecStats.thrift $OUT_DIR
cp $IMPALA_REPO/common/thrift/Status.thrift $OUT_DIR
cp $IMPALA_REPO/common/thrift/Types.thrift $OUT_DIR
cp $IMPALA_REPO/thirdparty/thrift-*/contrib/fb303/if/fb303.thrift $OUT_DIR

# hive_metastore.thrift assumes a directory structure for fb303.thrift, so we
# change the include statement here
cat $IMPALA_REPO/thirdparty/hive-*/src/metastore/if/hive_metastore.thrift \
        | sed 's/share\/fb303\/if\///g' \
        > $OUT_DIR/hive_metastore.thrift
