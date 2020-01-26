import 'package:flutter/material.dart';

import './question.dart';
import './answer.dart';

class Quiz extends StatelessWidget {
  final Function answerHandler;
  final List<Map<String, Object>> questions;
  final int index;

  Quiz({
    @required this.answerHandler,
    @required this.questions,
    @required this.index,
  });

  @override
  Widget build(BuildContext context) {
    return Column(
      children: [
        Question(
          questions[index]['questionText'],
        ),
        ...(questions[index]['answers'] as List<String>).map((answer) {
          return Answer(answerHandler, answer);
        }).toList(),
      ],
    );
  }
}
