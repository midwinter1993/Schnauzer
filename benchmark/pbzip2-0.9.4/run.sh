#!/bin/bash


ln -s $Schnauzer/scripts/control.py ./control.py
./control.py sys ./trigger.sh $Schnauzer/logs/pbzip2-0.9.4/LN.yaml -t 3
