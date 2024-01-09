#ifndef MODEL_H
#error "Cannot include this file directly"
#endif

#ifndef CLIENT_H
#define CLINET_H

enum client_privilege {
    VIP,
    NORMAL
} ;

struct client {
    enum client_privilege type ;
    double patience_mu ;
    int should_exit;
} ;

#endif