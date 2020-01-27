import 'package:flutter/material.dart';

import './question.dart';
import './answer.dart';

class Result extends StatelessWidget {
  final Function resetHandler;

  Result(this.resetHandler);

  @override
  Widget build(BuildContext context) {
    return Column(
      children: <Widget>[
        Center(
          child: Text('You\'re done! You scored 100%'),
        ),
        FlatButton(child: Text('Restart Quiz!'), onPressed: resetHandler),
      ],
    );
  }
}
