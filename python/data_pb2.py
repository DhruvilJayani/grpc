# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# NO CHECKED-IN PROTOBUF GENCODE
# source: data.proto
# Protobuf Python Version: 5.29.0
"""Generated protocol buffer code."""
from google.protobuf import descriptor as _descriptor
from google.protobuf import descriptor_pool as _descriptor_pool
from google.protobuf import runtime_version as _runtime_version
from google.protobuf import symbol_database as _symbol_database
from google.protobuf.internal import builder as _builder
_runtime_version.ValidateProtobufRuntimeVersion(
    _runtime_version.Domain.PUBLIC,
    5,
    29,
    0,
    '',
    'data.proto'
)
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n\ndata.proto\x12\x04\x64\x61ta\"=\n\x0b\x44\x61taMessage\x12\n\n\x02id\x18\x01 \x01(\x05\x12\x0f\n\x07payload\x18\x02 \x01(\x0c\x12\x11\n\ttimestamp\x18\x03 \x01(\t\"\x07\n\x05\x45mpty29\n\x0b\x44\x61taService\x12*\n\x08PushData\x12\x11.data.DataMessage\x1a\x0b.data.Emptyb\x06proto3')

_globals = globals()
_builder.BuildMessageAndEnumDescriptors(DESCRIPTOR, _globals)
_builder.BuildTopDescriptorsAndMessages(DESCRIPTOR, 'data_pb2', _globals)
if not _descriptor._USE_C_DESCRIPTORS:
  DESCRIPTOR._loaded_options = None
  _globals['_DATAMESSAGE']._serialized_start=20
  _globals['_DATAMESSAGE']._serialized_end=81
  _globals['_EMPTY']._serialized_start=83
  _globals['_EMPTY']._serialized_end=90
  _globals['_DATASERVICE']._serialized_start=92
  _globals['_DATASERVICE']._serialized_end=149
# @@protoc_insertion_point(module_scope)
