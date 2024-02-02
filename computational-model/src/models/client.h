#ifndef MODEL_H
#error "Cannot include this file directly"
#endif

#ifndef CLIENT_H
#define CLIENT_H

struct reservation {
    int ride_idx ;
    int clients_left ;
    char expired ;
} ;

enum client_privilege {
    VIP,
    NORMAL
} ;

struct client {
    enum client_privilege type ;
    double client_percentage ;
    double exit_time;
    
    // Statistics
    int lost_patience_times; // Track how many times the client has lost patience
    double arrival_time; // client has entered in the park (not in the entrance queue)

    int num_active_reservations;
    struct reservation active_reservations[5];
} ;

#endif