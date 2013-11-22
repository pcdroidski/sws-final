#!/bin/bash

EXECUTABLE="$(pwd)/unittest"
TESTIN='test.in'
TESTOUT='test.out'

npass=0
nrun=0

echo "Building..."
make clean test
echo

if [ $? = 0 ]; then
        echo "Running tests..."
        echo
        rm -f $TESTOUT
        echo "Date: $(date)" > $TESTOUT
        echo "Run by: $(whoami)" >> $TESTOUT
        echo -e "Host: $(uname -a)\n" >> $TESTOUT

        while read line; do
                EXPECTED=${line:0:1}

                printf "%-62s" "--> ${line:2} "
                echo "--> ${line:2}" >> $TESTOUT
                $EXECUTABLE "${line:2}" >> $TESTOUT

                RESULT=$?
                echo "Return code: $RESULT" >> $TESTOUT
                
                ((nrun++))

                if [ "0" = $EXPECTED ]; then
                        echo -n "++"
                else
                        echo -n "--"
                fi

                if [ "$RESULT" = $EXPECTED ]; then
                        echo "$(tput setaf 2)[PASS]$(tput sgr0)"
                        echo "[PASS]" >> $TESTOUT
                        ((npass++))
                else
                        echo "$(tput setaf 1)[FAIL]$(tput sgr0)"
                        echo "[FAIL]" >> $TESTOUT
                fi
        done < $TESTIN

        echo -e "\nSummary\nTests passed: $npass\nTests run: $nrun" | tee -a $TESTOUT
else
        echo "Build failed! Aborting"
fi
