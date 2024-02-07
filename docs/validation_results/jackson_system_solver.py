import sys
import numpy as np
import json


def extractProbabilities(file) :
  file = open(file, "r")

  probabilities = []
  content_json = json.loads(file.read())
  for ride in content_json['rides']:
    probabilities.append(float(ride['popularity']))
  
  exit_rate = float(content_json['park_exit_rate'])

  return probabilities, exit_rate

def createMatrix(probabilities, q):
  rows = []
  for i in range(len(probabilities)):
    p = probabilities[i]
    row = []
    for j in range(len(probabilities)):
      if i == j:
        row.append(-(1 - p*(1-q)))
      else:
        row.append(p*(1-q))
    rows.append(row)
  return np.array(rows)
 
def main():
  if len(sys.argv) != 3:
    print(f"Incorrect usage: expected {sys.argv[0]} <source.json> <arrival_rate>")
    return
  lmba = float(sys.argv[2])
  probabilities, q = extractProbabilities(sys.argv[1])
  
  matrix = createMatrix(probabilities, q)
  b = -np.array(probabilities)*lmba

  x = np.linalg.solve(matrix, b)
  for i in x:
    print(i)

if __name__ == "__main__":
  main()