#ifndef MODEL_H
#error "Cannot include this file directly"
#endif

#ifndef CLIENT_H
#define CLIENT_H

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
} ;

#endif