#!/bin/bash
# https://www.youtube.com/watch?v=QhLSMdHEal0

SESSION=cyrus
tmux new-session -d -s $SESSION
tmux new-window -t $SESSION:1 -n 'firewall'
tmux new-window -t $SESSION:2 -n 'kdc.example.com'
tmux new-window -t $SESSION:3 -n 'ldap27.example.com'
tmux new-window -t $SESSION:4 -n 'ldap26.example.com'
tmux new-window -t $SESSION:5 -n 'ldap23.example.com'
tmux new-window -t $SESSION:6 -n 'imap.example.com'
tmux new-window -t $SESSION:7 -n 'kdc.example.org'
tmux new-window -t $SESSION:8 -n 'ldap27.example.org'
tmux new-window -t $SESSION:9 -n 'ldap26.example.org'
tmux new-window -t $SESSION:10 -n 'ldap23.example.org'
tmux new-window -t $SESSION:11 -n 'imap.example.org'
