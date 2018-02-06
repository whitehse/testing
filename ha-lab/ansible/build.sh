#!/bin/bash

ansible-playbook --connection=local -i hosts site-containers.yml
