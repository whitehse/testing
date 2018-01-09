#!/bin/bash
# https://www.youtube.com/watch?v=QhLSMdHEal0

SESSION=cyrus
tmux new-session -d -s $SESSION
tmux new-window -t $SESSION:1 -n 'firewall'
tmux new-window -t $SESSION:2 -n 'kdc.example.com'
tmux new-window -t $SESSION:3 -n 'ldap.example.com'
tmux new-window -t $SESSION:4 -n 'imap.example.com'
tmux new-window -t $SESSION:6 -n 'kdc.example.org'
tmux new-window -t $SESSION:7 -n 'ldap.example.org'
tmux new-window -t $SESSION:8 -n 'imap.example.org'
