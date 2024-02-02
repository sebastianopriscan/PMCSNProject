#! /bin/sh

# $1: ride index

mkdir -p "bin/output/lambdas/$1/"
./bin/lambda-evaluator-executable ./docs/whole_park.json "bin/output/lambdas/$1/lambdas.csv" $1 >> bin/output/lambdas.csv