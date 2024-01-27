package main

import (
	"flag"
	"fmt"
	"log"
	"math"
	"math/big"
	"os"
	"strconv"
	"strings"
)

var source string
var tolerance float64

func factorial(n *big.Int) *big.Int {
	if n.Cmp(big.NewInt(1)) == 1 {
		return n.Mul(n, factorial(n.Sub(n, big.NewInt(1))))
	}
	return big.NewInt(1)
}

func formula(lambda float64, node_mean_time float64, server_num int) float64 {
	rho := lambda * node_mean_time / float64(server_num)
	e_s := node_mean_time / float64(server_num)

	p_0 := big.NewFloat(
		math.Pow(float64(server_num)*rho, float64(server_num)) / (1 - rho))
	fact := big.NewFloat(0).SetInt(factorial(big.NewInt(int64(server_num))))
	p_0 = p_0.Quo(p_0, fact)

	for i := 0; i < server_num; i++ {
		pow := big.NewFloat(math.Pow(float64(server_num)*rho, float64(i)))
		fact := big.NewFloat(0).SetInt(factorial(big.NewInt(int64(i))))
		p_0 = p_0.Add(pow.Quo(pow, fact), p_0)
	}

	p_0.Quo(big.NewFloat(1.0), p_0)

	// p_q := big.NewFloat(p_0)
	first_part := big.NewFloat(math.Pow(float64(server_num)*rho, float64(server_num)) / (1 - rho))
	fact = big.NewFloat(0).SetInt(factorial(big.NewInt(int64(server_num))))
	first_part = first_part.Quo(first_part, fact)
	p_q := p_0.Mul(p_0, first_part)
	// p_q := big.NewFloat()
	// p_q = p_0 * math.Pow(float64(server_num)*rho), float64(server_num) / (float64(factorial(server_num)) * (1 - rho))
	e_tq := p_q.Mul(big.NewFloat(e_s/(1-rho)), p_q)
	ret, _ := e_tq.Float64()
	return ret
}

func init() {
	flag.StringVar(&source, "source", "", "csv file")
	flag.Float64Var(&tolerance, "tolerance", 0.5, "tolerance")
}

func parse_csv(source string) (
	names []string, queue_times []float64,
	lambdas []float64, node_mean_times []float64, server_nums []int,
) {
	bytes, err := os.ReadFile(source)
	if err != nil {
		log.Fatalf("Could not read source file: %+v", err)
	}
	contents := string(bytes)
	split := strings.Split(contents, "\n")[1:]
	for _, v := range split {
		if v == "" {
			continue
		}
		data := strings.Split(v, ",")
		names = append(names, data[0])
		queue_time, err := strconv.ParseFloat(strings.TrimSpace(data[1]), 64)
		if err != nil {
			log.Fatalf("Could not parse queue_time: %+v", err)
		}
		lambda, err := strconv.ParseFloat(strings.TrimSpace(data[3]), 64)
		if err != nil {
			log.Fatalf("Could not parse lambda: %+v", err)
		}
		node_mean_time, err := strconv.ParseFloat(strings.TrimSpace(data[6]), 64)
		if err != nil {
			log.Fatalf("Could not parse node_mean_time: %+v", err)
		}
		queue_times = append(queue_times, queue_time)
		lambdas = append(lambdas, lambda)
		node_mean_times = append(node_mean_times, node_mean_time)
		server_nums = append(server_nums, len(data)-7)
	}
	return
}

func main() {
	flag.Parse()
	names, queue_times, lambdas, node_mean_times, server_nums := parse_csv(source)
	fmt.Println("Name, Lambda, Service Time, Server Num, Empirical Queue Time, Theoretical Queue Time, , Check Result")
	for i, v := range names {
		theoretical_queue_time := formula(lambdas[i], node_mean_times[i], server_nums[i])
		check := math.Abs(theoretical_queue_time-queue_times[i]) < tolerance
		rho := lambdas[i] * node_mean_times[i] / float64(server_nums[i])
		fmt.Printf("%s, %f, %f, %d, %f, %f, %f, , %t\n", v, lambdas[i], node_mean_times[i], server_nums[i], queue_times[i], theoretical_queue_time, rho, check)
	}
}
