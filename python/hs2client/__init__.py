# Copyright 2016 Cloudera Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# flake8: noqa

from .ext import (Service, Session, Operation, Schema, ColumnType,
                  PrimitiveType, CharacterType, DecimalType)
import os

def connect(host, port=21050, user=None, protocol='v7'):
    user = os.environ.get('USER') if user is None else user
    return Service(host, port, user, protocol=protocol)
