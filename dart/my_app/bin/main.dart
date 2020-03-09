import 'dart:io';
//import 'package:path_provider/path_provider.dart';
import 'package:x509csr/x509csr.dart';
import "package:pointycastle/export.dart";
import 'package:asn1lib/asn1lib.dart';

import 'package:my_app/my_app.dart' as my_app;

void main(List<String> arguments) async {
/*
  print('Hello world: ${my_app.calculate()}!');

  var file = File('data.txt');
  var contents;

  if (await file.exists()) {
    // Read file
    contents = await file.readAsString();
    print(contents);

    // Write file
    var fileCopy = await File('data-copy.txt').writeAsString(contents);
    print(await fileCopy.exists());
    print(await fileCopy.length());
  }
*/
  AsymmetricKeyPair keyPair = rsaGenerateKeyPair();

  ASN1ObjectIdentifier.registerFrequentNames();
  Map<String, String> dn = {
    "CN": "www.davidjanes.com",
    "O": "Consensas",
    "L": "Toronto",
    "ST": "Ontario",
    "C": "CA",
  };

  ASN1Object encodedCSR = makeRSACSR(dn, keyPair.privateKey, keyPair.publicKey);

  //var file = File('cert.pem');
  //var fileCopy = await File('data-copy.txt').writeAsString(contents);

  var csrFile = await File('csr.pem').writeAsString(encodeCSRToPem(encodedCSR));
  var publicKeyFile = await File('public_key.pem').writeAsString(encodeRSAPublicKeyToPem(keyPair.publicKey));
  var privateKeyFile = await File('private_key.pem').writeAsString(encodeRSAPrivateKeyToPem(keyPair.privateKey));

}
