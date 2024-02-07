import sys
import csv
import math

def parse_csv(csv_source) :
  name = []
  queue_time = []
  lmba = []
  node_mean_time = []
  server_num = []

  with open(csv_source) as csvfile:
    reader = csv.reader(csvfile, delimiter=',')
    next(reader, None)  # skip the headers
    for row in reader:
      name.append(row[0])
      queue_time.append(float(row[1]))
      lmba.append(float(row[3]))
      node_mean_time.append(float(row[6]))
      server_num.append(len(row)-7)
  return name, queue_time, lmba, node_mean_time, server_num

def main() :
  if len(sys.argv) != 3:
    print(f"Incorrect usage; expected {sys.argv[0]} <csv_source> <tolerance>")
    exit(1)
  csv = sys.argv[1]
  tolerance = float(sys.argv[2])
  name, queue_time, lmba, node_mean_time, server_num = parse_csv(csv)

  print('Name, Lambda, Service Time, Server Num, Empirical Queue Time, Theoretical Queue Time, Rho, , Check Result')
  for i in range(len(name)) :
    s_num = server_num[i]
    if i == 3:
      s_num = 100
    theoretical_queue_time = formula(lmba=lmba[i], node_mean_time=node_mean_time[i], server_num=s_num)
    check = abs(theoretical_queue_time - queue_time[i]) < tolerance
    print(f'{name[i]}, {lmba[i]}, {node_mean_time[i]}, {s_num}, {queue_time[i]}, {theoretical_queue_time}, {lmba[i] * node_mean_time[i] / s_num} , {check}')
 
# E[T_q] = (P_q E[S]) / (1-rho)
# E[S] = E[S_i] / m
# P_q = (((m rho)^m) / (m! (1-rho)))* p(0)
# p(0) = [sum_i=0^{m-1} ((m rho)^i / i!) + ((m rho)^m) / (m! (1-rho))]^(-1)
def formula(lmba, node_mean_time, server_num) :
  rho = lmba * node_mean_time / server_num
  if rho > 1.0:
    return 0
  e_s = node_mean_time / server_num

  tmp_log_2 = server_num * math.log(server_num * rho) - math.log(1 - rho)
  for i in range(1, server_num + 1):
    tmp_log_2 = tmp_log_2 - math.log(i)
  
  sum = 0.0
  for i in range(0, server_num):
    tmp_log = i * math.log(server_num * rho)
    for j in range(1, i + 1):
      tmp_log = tmp_log - math.log(j)
    sum = sum + math.exp(tmp_log)
  p_0 = 0
  try:
    p_0 = 1 / (math.exp(sum) + math.exp(tmp_log_2))
  except:
    p_0 = 0.001

  # p_0 = pow(server_num * rho, server_num) / (math.factorial(server_num)*(1 - rho))
  # for i in range(0, server_num) :
  #   p_0 += pow(server_num * rho, i) / math.factorial(i)
  # p_0 = 1 / p_0

  p_q = p_0 * math.exp(tmp_log_2)
  # p_q = p_0 * pow(server_num * rho, server_num) / (math.factorial(server_num)*(1 - rho))

  e_tq = p_q * e_s / (1 - rho)
  return e_tq

if __name__ == "__main__":
  main()
