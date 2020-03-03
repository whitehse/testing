///
//  Generated code. Do not modify.
//  source: example.proto
//
// @dart = 2.3
// ignore_for_file: camel_case_types,unnecessary_const,non_constant_identifier_names,library_prefixes,unused_import,unused_shown_name,return_of_invalid_type

import 'dart:core' as $core;

import 'package:protobuf/protobuf.dart' as $pb;

class MyMessage extends $pb.GeneratedMessage {
  static final $pb.BuilderInfo _i = $pb.BuilderInfo('MyMessage', createEmptyInstance: create)
    ..a<$core.int>(1, 'id', $pb.PbFieldType.O3)
    ..aOS(2, 'firstName')
    ..aOB(3, 'isValidated')
    ..hasRequiredFields = false
  ;

  MyMessage._() : super();
  factory MyMessage() => create();
  factory MyMessage.fromBuffer($core.List<$core.int> i, [$pb.ExtensionRegistry r = $pb.ExtensionRegistry.EMPTY]) => create()..mergeFromBuffer(i, r);
  factory MyMessage.fromJson($core.String i, [$pb.ExtensionRegistry r = $pb.ExtensionRegistry.EMPTY]) => create()..mergeFromJson(i, r);
  MyMessage clone() => MyMessage()..mergeFromMessage(this);
  MyMessage copyWith(void Function(MyMessage) updates) => super.copyWith((message) => updates(message as MyMessage));
  $pb.BuilderInfo get info_ => _i;
  @$core.pragma('dart2js:noInline')
  static MyMessage create() => MyMessage._();
  MyMessage createEmptyInstance() => create();
  static $pb.PbList<MyMessage> createRepeated() => $pb.PbList<MyMessage>();
  @$core.pragma('dart2js:noInline')
  static MyMessage getDefault() => _defaultInstance ??= $pb.GeneratedMessage.$_defaultFor<MyMessage>(create);
  static MyMessage _defaultInstance;

  @$pb.TagNumber(1)
  $core.int get id => $_getIZ(0);
  @$pb.TagNumber(1)
  set id($core.int v) { $_setSignedInt32(0, v); }
  @$pb.TagNumber(1)
  $core.bool hasId() => $_has(0);
  @$pb.TagNumber(1)
  void clearId() => clearField(1);

  @$pb.TagNumber(2)
  $core.String get firstName => $_getSZ(1);
  @$pb.TagNumber(2)
  set firstName($core.String v) { $_setString(1, v); }
  @$pb.TagNumber(2)
  $core.bool hasFirstName() => $_has(1);
  @$pb.TagNumber(2)
  void clearFirstName() => clearField(2);

  @$pb.TagNumber(3)
  $core.bool get isValidated => $_getBF(2);
  @$pb.TagNumber(3)
  set isValidated($core.bool v) { $_setBool(2, v); }
  @$pb.TagNumber(3)
  $core.bool hasIsValidated() => $_has(2);
  @$pb.TagNumber(3)
  void clearIsValidated() => clearField(3);
}

