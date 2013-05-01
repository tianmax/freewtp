#include "wtp.h"
#include "capwap_dfa.h"
#include "wtp_dfa.h"

/* */
int wtp_dfa_state_reset(struct capwap_packet* packet, struct timeout_control* timeout) {
	ASSERT(packet == NULL);
	ASSERT(timeout != NULL);

	return wtp_teardown_connection(timeout);
}
