#!/bin/bash

ansible-playbook --connection=local -i hosts clonable-containers.yml
ansible-playbook --connection=local -i hosts site-containers.yml
