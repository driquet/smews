/*
* Copyright or © or Copr. 2008, Simon Duquennoy
* 
* Author e-mail: simon.duquennoy@lifl.fr
* 
* This software is a computer program whose purpose is to design an
* efficient Web server for very-constrained embedded system.
* 
* This software is governed by the CeCILL license under French law and
* abiding by the rules of distribution of free software.  You can  use, 
* modify and/ or redistribute the software under the terms of the CeCILL
* license as circulated by CEA, CNRS and INRIA at the following URL
* "http://www.cecill.info".
* 
* As a counterpart to the access to the source code and  rights to copy,
* modify and redistribute granted by the license, users are provided only
* with a limited warranty  and the software's author,  the holder of the
* economic rights,  and the successive licensors  have only  limited
* liability. 
* 
* In this respect, the user's attention is drawn to the risks associated
* with loading,  using,  modifying and/or developing or reproducing the
* software by the user in light of its specific status of free software,
* that may mean  that it is complicated to manipulate,  and  that  also
* therefore means  that it is reserved for developers  and  experienced
* professionals having in-depth computer knowledge. Users are therefore
* encouraged to load and test the software's suitability as regards their
* requirements in conditions enabling the security of their systems and/or 
* data to be ensured and,  more generally, to use and operate it in the 
* same conditions as regards security. 
* 
* The fact that you are presently reading this means that you have had
* knowledge of the CeCILL license and that you accept its terms.
*/

#ifndef __CONNECTIONS_H__
#define __CONNECTIONS_H__

#include "handlers.h"
#include "coroutines.h"

/** TCP **/

/* TCP flags */
#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PSH 0x08
#define TCP_ACK 0x10
#define TCP_URG 0x20

/** Connections **/

#ifndef DISABLE_POST
/* Boundary structure (for multipart) */
struct boundary_t {
	char *boundary_ref; /* boundary which separates parts in multipart post data */
	char *boundary_buffer; /* buffer used to detect boundary */
	uint8_t boundary_size; /* size of boundary */
	uint8_t index;
	uint8_t ready_to_count; /* true when post header is parsed */
	uint8_t multi_part_counter; /* to know index of part parsed */
};

/* Post data structure */
struct post_data_t {
	uint8_t content_type;
	uint16_t content_length;
	struct boundary_t *boundary;
	struct coroutine_t coroutine;
	char *filename; /* filename of current part */
	void *post_data; /* data flowing between dopostin and dopostout functions */
};
#endif

/* Connection structure */
struct http_connection {
	unsigned char next_outseqno[4];
	unsigned char final_outseqno[4];
	unsigned char current_inseqno[4];
#ifndef IPV6
	unsigned char ip_addr[4];
#endif
	const struct output_handler_t * /*CONST_VAR*/ output_handler;

	unsigned char port[2];
	unsigned char cwnd[2];
	unsigned char inflight[2];

	unsigned const char * /*CONST_VAR*/ blob;
#ifndef DISABLE_ARGS
	struct args_t *args;
	unsigned char *curr_arg;
	unsigned char arg_ref_index;
#endif
#ifndef DISABLE_TIMERS
	unsigned char transmission_time;
#endif
	uint16_t tcp_mss: 12;
	uint8_t ready_to_send:1;
	enum tcp_state_e {tcp_listen, tcp_syn_rcvd, tcp_established, tcp_closing, tcp_last_ack} tcp_state: 3;
	enum parsing_state_e {parsing_out, parsing_cmd, parsing_url, parsing_end
#ifndef DISABLE_POST
	, parsing_post_attributes, parsing_post_end, parsing_post_content_type, parsing_post_args, parsing_post_data, parsing_boundary, parsing_init_buffer	} parsing_state: 4;
	struct post_data_t *post_data;
	uint8_t post_url_detected:1; /* bit used to know if url is post or get request */
#else
	} parsing_state: 2;
#endif
#ifndef DISABLE_COMET
	unsigned char comet_passive: 1;
	unsigned char comet_send_ack: 1;
	unsigned char comet_streaming: 1;
#endif
	struct generator_service_t *generator_service;
	
	struct http_connection *next;
	struct http_connection *prev;
#ifdef IPV6
	unsigned char ip_addr[0];
#endif
};

/* Loop on each connection */
#define FOR_EACH_CONN(item, code) \
	if(all_connections) { \
		struct http_connection *(item) = all_connections; \
		do { \
			{code} \
			(item) = (item)->next; \
		} while((item) != all_connections); \
	} \

/* Pseudo connection for RST */
struct http_rst_connection {
#ifdef IPV6
	unsigned char ip_addr[16];
#else
	unsigned char ip_addr[4];
#endif
	unsigned char current_inseqno[4];
	unsigned char next_outseqno[4];
	unsigned char port[2];
};

/* Shared global connections structures */
extern struct http_connection *all_connections;
extern struct http_rst_connection rst_connection;

/* Local IP address */
#ifdef IPV6
extern unsigned char local_ip_addr[16];
extern char ipcmp(unsigned char source_addr[],  unsigned char check_addr[]);
extern unsigned char * decompress_ip(unsigned char comp_ip_addr[], unsigned char full_ip_addr[], unsigned char indexes);
extern unsigned char * compress_ip(unsigned char full_ip_addr[], unsigned char comp_ip_addr[], unsigned char * indexes);
#else
extern unsigned char local_ip_addr[4];
#endif

/* Shared funuctions */
extern char something_to_send(const struct http_connection *connection);
extern void free_connection(const struct http_connection *connection);

#ifndef DISABLE_POST
/* Shared coroutine state (in = 0 / out = 1)*/
struct coroutine_state_t { 
	enum coroutine_state_e {cor_in, cor_out} state:1;
};
extern struct coroutine_state_t coroutine_state;
#endif

#endif /* __CONNECTIONS_H__ */
