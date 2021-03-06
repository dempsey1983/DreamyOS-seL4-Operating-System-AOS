/*
 * Copyright 2014, NICTA
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(NICTA_BSD)
 */

struct serial {
    void (*fHandler) (struct serial *serial, char c);
    struct udp_pcb *fUpcb;
    void * routine;
};

extern struct serial *serial_init(void);
extern int serial_send(struct serial *serial, char *data, int len);
extern int serial_register_handler(struct serial *serial, 
			void (*handler) (struct serial *serial, char c));
extern int serial_deregister_handler(struct serial *serial);
