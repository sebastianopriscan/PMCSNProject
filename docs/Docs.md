# PMCSN

## 0. Descrizione del sistema

Parco divertimenti con due *tipi di biglietti/utenti*:
- VIP: priorità
- normali: senza priorità

Due *tipi di attrazioni*:
- Spettacoli
- Giostre

## 1. Goals and Objectives

Obiettivo preliminare: studiare l'attuale configurazione, 
decidendo il numero di attrazioni

Trovare la divisione percentuale dei *tipi di biglietti* 

Trovare la divisione percentuale dei *tipi di attrazioni* e la capienza
di ogni attrazione

In modo tale da massimizzare profitto e mantenere i tempi di attesa entro 
obiettivi di QoS (TODO: da definire)

Profitto dipende da:
- guardagno da biglietto normale e vip
- costi di manutenzione e aggiunta di attrazioni

Obiettivo aggiuntivo: minimizzare clienti Idle (non in coda/giostra)

## 2. Conceptual Model

Client:
- `state`: enum { BUSY, IDLE, IN_QUEUE }
- `type`: enum { NORMAL, VIP }
- `group`: Client [1..n]
- `max_queue_time`: `int`

Attraction:
- `type`: enum { SHOW, RIDE }
- `state`: enum { BUSY, IDLE } [1..capacity]
- `capacity`: `int`
- `popularity`: `float`
- `mean_length_time`: `int`

Objective Parameters (dynamic):
- price of VIP ticket: `int`
- price of normal ticket: `int`
- percentage of VIP ticket: `int`
- percentage of normal ticket: `int`
- percentage of rides: `int`
- percentage of shows: `int`

Constants:
- maintenance cost per rides: `int`
- maintenance cost per shows: `int`
- construction cost per seat (per rides): `int`

<!-- TODO: check with step 7 -->
<!-- Simulation Parameters: -->
<!-- - number of clients: int -->
<!-- - number of attractions: int -->

Constraints (interrelation):
- $\sum_{j \in \text{Attraction}} \text{j.popularity} = 1$
- percentage of VIP ticket + percentage of normal ticket = 100
- percentage of rides + percentage of shows = 100

[Figma](https://www.figma.com/file/zFG8SEBIXFHGgtrGlmpwuW/PMCSNAmusementParkProject?type=design&mode=design&t=TMgxlRdeCfl4DeZ4-1)

## 3. Specification Model

Rides:
- Arrival: exponential
- Service Time: normal (with mean _vedi tabella_ and variance $0.1*\text{mean}$, coupled between servers)
  - TODO: check variance

Shows:
- Arrival: exponential
- Service Time: exponential/pareto, uncoupled

Patience: normal (TODO: choose parameters)
- Determined by the lowest `max_queue_time` in the group

Decision (switch from IDLE to IN_QUEUE): exponential (with low lambda)

| Attraction Name    | Arrival    | Service Time Mean    | Service Time Variance | Type | No. of seats| Arrival Distribution | Service Distribution | 
|---------------- | --------------- | --------------- | ----- | -- | -- | -- | -- |
| Alice in Wonderland    | --    | <center>4 Min    | 0.1*mean | <center> Ride | 16 | <center> Exponential | <center> Normal |
| Casey Jr. Circus Train   | --   | <center> 4 Min   | 0.1*mean | <center> Ride | 20 | <center> Exponential | <center> Normal |
| Dumbo the Flying Elephant | --   | <center> 2 Min   | 0.1*mean | <center> Ride | 16 | <center> Exponential | <center> Normal |
| It's a small world | --   | <center> 14 Min   | 0.1*mean | <center> Ride | 16 | <center> Exponential | <center> Normal |
| King Arthur Carousel | --   | <center> 3 Min   | 0.1*mean | <center> Ride | 68 | <center> Exponential | <center> Normal |
| Mad Tea Party | --   | <center> 2 Min   | --- | <center> Ride | 18 | <center> Exponential | <center> Normal |
| Matterhorn Bobsleds | --   | <center> 4 Min   | 0.1*mean | <center> Ride | 120 | <center> Exponential | <center> Normal |
| Mr Toad's Wild Ride | --   | <center> 2 Min   | 0.1*mean | <center> Ride | 32 | <center> Exponential | <center> Normal |
| Pearly Band | --   | <center> 15 Min   | --- | <center> Show | -- | <center> Exponential | <center> Normal |
| Peter Pan's Flight | --   | <center> 3 Min   | 0.1*mean | <center> Ride | 18 | <center> Exponential | <center> Normal |
| Pinocchio's Daring Journey | --   | <center> 3 Min   | 0.1*mean | <center> Ride | 24 | <center> Exponential | <center> Normal |
| The Royal Swing Big Band Ball | --   | <center> 30 Min   | --- | <center> Show | -- | <center> Exponential | <center> Normal |
| Sleeping Beauty Castle Walkthrough | --   | <center> 15 Min   | --- | <center> Show | -- | <center> Exponential | <center> Normal |
| Snow White's Enchanted Wish | --   | <center> 2 Min   | 0.1*mean | <center> Ride | 16 | <center> Exponential | <center> Normal |
| Storybook Land Canal Boats | --   | <center> 10 Min   | 0.1*mean | <center> Ride | 14 | <center> Exponential | <center> Normal |
| Storytelling at Royal Theatre | --   | <center> 20 Min   | --- | <center> Show | -- | <center> Exponential | <center> Normal |
| Tale of the Lion King | --   | <center> 24 Min   | --- | <center> Show | -- | <center> Exponential | <center> Normal |
