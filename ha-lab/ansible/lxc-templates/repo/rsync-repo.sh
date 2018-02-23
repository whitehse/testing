#!/bin/bash

rsync -avz -e ssh remoteuser@remotehost:/remote/dir /this/dir/ 
rsync -avz -e ssh . dwhite@192.168.2.11:/var/www/deb.cafedemocracy.org/debian/
