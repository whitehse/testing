///
//  Generated code. Do not modify.
//  source: service.proto
//
// @dart = 2.3
// ignore_for_file: camel_case_types,unnecessary_const,non_constant_identifier_names,library_prefixes,unused_import,unused_shown_name,return_of_invalid_type

import 'dart:async' as $async;

import 'package:protobuf/protobuf.dart' as $pb;

import 'dart:core' as $core;
import 'service.pb.dart' as $0;
import 'service.pbjson.dart';

export 'service.pb.dart';

abstract class MathServiceBase extends $pb.GeneratedService {
  $async.Future<$0.Response> add($pb.ServerContext ctx, $0.Request request);
  $async.Future<$0.Response> multiply($pb.ServerContext ctx, $0.Request request);

  $pb.GeneratedMessage createRequest($core.String method) {
    switch (method) {
      case 'Add': return $0.Request();
      case 'Multiply': return $0.Request();
      default: throw $core.ArgumentError('Unknown method: $method');
    }
  }

  $async.Future<$pb.GeneratedMessage> handleCall($pb.ServerContext ctx, $core.String method, $pb.GeneratedMessage request) {
    switch (method) {
      case 'Add': return this.add(ctx, request);
      case 'Multiply': return this.multiply(ctx, request);
      default: throw $core.ArgumentError('Unknown method: $method');
    }
  }

  $core.Map<$core.String, $core.dynamic> get $json => MathServiceBase$json;
  $core.Map<$core.String, $core.Map<$core.String, $core.dynamic>> get $messageJson => MathServiceBase$messageJson;
}

