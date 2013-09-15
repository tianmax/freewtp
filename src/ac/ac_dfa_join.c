#include "ac.h"
#include "capwap_dfa.h"
#include "capwap_array.h"
#include "ac_session.h"
#include "ac_backend.h"
#include <json/json.h>
#include <arpa/inet.h>

/* */
static int ac_dfa_state_join_check_authorizejoin(struct ac_session_t* session, struct ac_soap_response* response) {
	xmlChar* xmlResult;

	if ((response->responsecode != HTTP_RESULT_OK) || !response->xmlResponseReturn) {
		/* TODO: check return failed code */
		return CAPWAP_RESULTCODE_JOIN_FAILURE_UNKNOWN_SOURCE;
	}

	// Check return value
	xmlResult = xmlNodeGetContent(response->xmlResponseReturn);
	if (xmlStrcmp(xmlResult, (const xmlChar *)"true")) {
		return CAPWAP_RESULTCODE_JOIN_FAILURE_UNKNOWN_SOURCE;
	}

	return CAPWAP_RESULTCODE_SUCCESS;
}

/* */
static struct ac_soap_response* ac_dfa_state_join_parsing_request(struct ac_session_t* session, struct capwap_parsed_packet* packet) {
	int i;
	const char* jsonmessage;
	char* base64confstatus;
	struct json_object* jsonarray;
	struct json_object* jsonparam;
	struct json_object* jsonhash;
	struct ac_soap_response* response;
	struct capwap_location_element* location;
	struct capwap_wtpboarddata_element* wtpboarddata;
	struct capwap_wtpdescriptor_element* wtpdescriptor;
	struct capwap_wtpname_element* wtpname;
	struct capwap_wtpframetunnelmode_element* wtpframetunnelmode;
	struct capwap_wtpmactype_element* wtpmactype;
	struct capwap_ecnsupport_element* ecnsupport;
	struct capwap_localipv4_element* localipv4;
	struct capwap_localipv6_element* localipv6;
	struct capwap_wtprebootstat_element* wtprebootstat;
	unsigned short binding = GET_WBID_HEADER(packet->rxmngpacket->header);

	/* Create SOAP request with JSON param
		{
			Binding: {
				Type: [int]
			}
			LocationData: {
				Location: [string]
			},
			WTPBoardData: {
				VendorIdentifier: [int],
				BoardDataSubElement: [
					{
						BoardDataType: [int],
						BoardDataValue: [base64]
					}
				]
			},
			WTPDescriptor: {
				MaxRadios: [int],
				RadiosInUse: [int],
				EncryptionSubElement: [
					{
						WBID: [int],
						EncryptionCapabilities: [int]
					}
				],
				DescriptorSubElement: [
					{
						DescriptorVendorIdentifier: [int],
						DescriptorType: [int],
						DescriptorData: [string]
					}
				]
			},
			WTPName: {
				Name: [string]
			},
			WTPFrameTunnelMode: {
				NativeFrameTunnel: [bool]
				FrameTunnelMode8023: [bool],
				LocalBridge: [bool]
			},
			WTPMACType: {
				Type: [int]
			},
			WTPRadioInformation: [
				<IEEE 802.11 BINDING>
				IEEE80211WTPRadioInformation: {
					RadioID: [int],
					IEEE80211nMode: [bool],
					IEEE80211gMode: [bool],
					IEEE80211bMode: [bool],
					IEEE80211aMode: [bool]
				}
			]
			ECNSupport: {
				Mode: [int]
			}
			CAPWAPLocalIPv4Address: {
				Address: [string]
			},
			CAPWAPLocalIPv6Address: {
				Address: [string]
			},
			WTPRebootStatistics: {
				RebootCount: [int],
				ACInitiatedCount: [int],
				LinkFailureCount: [int],
				SWFailureCount: [int],
				HWFailureCount: [int],
				OtherFailureCount: [int],
				UnknownFailureCount: [int],
				LastFailureType: [int]
			}
		}
	*/

	/* */
	jsonparam = json_object_new_object();

	/* Binding */
	jsonhash = json_object_new_object();
	json_object_object_add(jsonhash, "Type", json_object_new_int((int)binding));
	json_object_object_add(jsonparam, "Binding", jsonhash);

	/* LocationData */
	location = (struct capwap_location_element*)capwap_get_message_element_data(packet, CAPWAP_ELEMENT_LOCATION);

	jsonhash = json_object_new_object();
	json_object_object_add(jsonhash, "Location", json_object_new_string((char*)location->value));
	json_object_object_add(jsonparam, "LocationData", jsonhash);

	/* WTPBoardData */
	wtpboarddata = (struct capwap_wtpboarddata_element*)capwap_get_message_element_data(packet, CAPWAP_ELEMENT_WTPBOARDDATA);

	jsonhash = json_object_new_object();
	json_object_object_add(jsonhash, "VendorIdentifier", json_object_new_int((int)wtpboarddata->vendor));

	jsonarray = json_object_new_array();
	for (i = 0; i < wtpboarddata->boardsubelement->count; i++) {
		char* base64data;
		int base64length;
		struct json_object* jsonboard;
		struct capwap_wtpboarddata_board_subelement* wtpboarddata_board = (struct capwap_wtpboarddata_board_subelement*)capwap_array_get_item_pointer(wtpboarddata->boardsubelement, i);

		/* Encoded base64 board data */
		base64data = (char*)capwap_alloc(AC_BASE64_ENCODE_LENGTH((int)wtpboarddata_board->length));
		base64length = ac_base64_binary_encode((const char*)wtpboarddata_board->data, (int)wtpboarddata_board->length, base64data);
		base64data[base64length] = 0;

		jsonboard = json_object_new_object();
		json_object_object_add(jsonboard, "BoardDataType", json_object_new_int((int)wtpboarddata_board->type));
		json_object_object_add(jsonboard, "BoardDataValue", json_object_new_string(base64data));
		json_object_array_add(jsonarray, jsonboard);

		capwap_free(base64data);
	}

	json_object_object_add(jsonhash, "BoardDataSubElement", jsonarray);
	json_object_object_add(jsonparam, "WTPBoardData", jsonhash);

	/* WTPDescriptor */
	wtpdescriptor = (struct capwap_wtpdescriptor_element*)capwap_get_message_element_data(packet, CAPWAP_ELEMENT_WTPDESCRIPTOR);

	jsonhash = json_object_new_object();
	json_object_object_add(jsonhash, "MaxRadios", json_object_new_int((int)wtpdescriptor->maxradios));
	json_object_object_add(jsonhash, "RadiosInUse", json_object_new_int((int)wtpdescriptor->radiosinuse));

	jsonarray = json_object_new_array();
	for (i = 0; i < wtpdescriptor->encryptsubelement->count; i++) {
		struct json_object* jsonencrypt;
		struct capwap_wtpdescriptor_encrypt_subelement* wtpdescriptor_encrypt = (struct capwap_wtpdescriptor_encrypt_subelement*)capwap_array_get_item_pointer(wtpdescriptor->encryptsubelement, i);

		jsonencrypt = json_object_new_object();
		json_object_object_add(jsonencrypt, "WBID", json_object_new_int((int)wtpdescriptor_encrypt->wbid));
		json_object_object_add(jsonencrypt, "EncryptionCapabilities", json_object_new_int((int)wtpdescriptor_encrypt->capabilities));
		json_object_array_add(jsonarray, jsonencrypt);
	}

	json_object_object_add(jsonhash, "EncryptionSubElement", jsonarray);

	jsonarray = json_object_new_array();
	for (i = 0; i < wtpdescriptor->descsubelement->count; i++) {
		struct json_object* jsondesc;
		struct capwap_wtpdescriptor_desc_subelement* wtpdescriptor_desc = (struct capwap_wtpdescriptor_desc_subelement*)capwap_array_get_item_pointer(wtpdescriptor->descsubelement, i);

		jsondesc = json_object_new_object();
		json_object_object_add(jsondesc, "DescriptorVendorIdentifier", json_object_new_int((int)wtpdescriptor_desc->vendor));
		json_object_object_add(jsondesc, "DescriptorType", json_object_new_int((int)wtpdescriptor_desc->type));
		json_object_object_add(jsondesc, "DescriptorData", json_object_new_string((char*)wtpdescriptor_desc->data));
		json_object_array_add(jsonarray, jsondesc);
	}

	json_object_object_add(jsonhash, "DescriptorSubElement", jsonarray);
	json_object_object_add(jsonparam, "WTPDescriptor", jsonhash);

	/* WTPName */
	wtpname = (struct capwap_wtpname_element*)capwap_get_message_element_data(packet, CAPWAP_ELEMENT_WTPNAME);

	jsonhash = json_object_new_object();
	json_object_object_add(jsonhash, "Name", json_object_new_string((char*)wtpname->name));
	json_object_object_add(jsonparam, "WTPName", jsonhash);

	/* WTPFrameTunnelMode */
	wtpframetunnelmode = (struct capwap_wtpframetunnelmode_element*)capwap_get_message_element_data(packet, CAPWAP_ELEMENT_WTPFRAMETUNNELMODE);

	jsonhash = json_object_new_object();
	json_object_object_add(jsonhash, "NativeFrameTunnel", json_object_new_boolean(((wtpframetunnelmode->mode & CAPWAP_WTP_NATIVE_FRAME_TUNNEL) ? 1 : 0)));
	json_object_object_add(jsonhash, "FrameTunnelMode8023", json_object_new_boolean(((wtpframetunnelmode->mode & CAPWAP_WTP_8023_FRAME_TUNNEL) ? 1 : 0)));
	json_object_object_add(jsonhash, "LocalBridge", json_object_new_boolean(((wtpframetunnelmode->mode & CAPWAP_WTP_LOCAL_BRIDGING) ? 1 : 0)));
	json_object_object_add(jsonparam, "WTPFrameTunnelMode", jsonhash);

	/* WTPMACType */
	wtpmactype = (struct capwap_wtpmactype_element*)capwap_get_message_element_data(packet, CAPWAP_ELEMENT_WTPMACTYPE);

	jsonhash = json_object_new_object();
	json_object_object_add(jsonhash, "Type", json_object_new_int((int)wtpmactype->type));
	json_object_object_add(jsonparam, "WTPMACType", jsonhash);

	/* WTPRadioInformation */
	if (binding == CAPWAP_WIRELESS_BINDING_IEEE80211) {
		struct capwap_array* wtpradioinformation = (struct capwap_array*)capwap_get_message_element_data(packet, CAPWAP_ELEMENT_80211_WTPRADIOINFORMATION);

		jsonarray = json_object_new_array();
		for (i = 0; i < wtpradioinformation->count; i++) {
			struct json_object* jsonradio;
			struct capwap_80211_wtpradioinformation_element* radio = *(struct capwap_80211_wtpradioinformation_element**)capwap_array_get_item_pointer(wtpradioinformation, i);

			jsonradio = json_object_new_object();
			json_object_object_add(jsonradio, "RadioID", json_object_new_int((int)radio->radioid));
			json_object_object_add(jsonradio, "IEEE80211nMode", json_object_new_boolean(((radio->radiotype & CAPWAP_RADIO_TYPE_80211N) ? 1: 0)));
			json_object_object_add(jsonradio, "IEEE80211gMode", json_object_new_boolean(((radio->radiotype & CAPWAP_RADIO_TYPE_80211G) ? 1: 0)));
			json_object_object_add(jsonradio, "IEEE80211bMode", json_object_new_boolean(((radio->radiotype & CAPWAP_RADIO_TYPE_80211B) ? 1: 0)));
			json_object_object_add(jsonradio, "IEEE80211aMode", json_object_new_boolean(((radio->radiotype & CAPWAP_RADIO_TYPE_80211A) ? 1: 0)));
			json_object_array_add(jsonarray, jsonradio);
		}

		jsonhash = json_object_new_object();
		json_object_object_add(jsonhash, "IEEE80211WTPRadioInformation", jsonarray);
		json_object_object_add(jsonparam, "WTPRadioInformation", jsonhash);
	}

	/* ECNSupport */
	ecnsupport = (struct capwap_ecnsupport_element*)capwap_get_message_element_data(packet, CAPWAP_ELEMENT_ECNSUPPORT);

	jsonhash = json_object_new_object();
	json_object_object_add(jsonhash, "Mode", json_object_new_int((int)ecnsupport->flag));
	json_object_object_add(jsonparam, "ECNSupport", jsonhash);

	/* CAPWAPLocalIPv4Address */
	localipv4 = (struct capwap_localipv4_element*)capwap_get_message_element_data(packet, CAPWAP_ELEMENT_LOCALIPV4);
	if (localipv4) {
		char ipbuffer[INET_ADDRSTRLEN];

		jsonhash = json_object_new_object();
		json_object_object_add(jsonhash, "Address", json_object_new_string(inet_ntop(AF_INET, (void*)&localipv4->address, ipbuffer, INET_ADDRSTRLEN)));
		json_object_object_add(jsonparam, "CAPWAPLocalIPv4Address", jsonhash);
	}

	/* CAPWAPLocalIPv6Address */
	localipv6 = (struct capwap_localipv6_element*)capwap_get_message_element_data(packet, CAPWAP_ELEMENT_LOCALIPV6);
	if (localipv6) {
		char ipbuffer[INET6_ADDRSTRLEN];

		jsonhash = json_object_new_object();
		json_object_object_add(jsonhash, "Address", json_object_new_string(inet_ntop(AF_INET, (void*)&localipv6->address, ipbuffer, INET6_ADDRSTRLEN)));
		json_object_object_add(jsonparam, "CAPWAPLocalIPv6Address", jsonhash);
	}

	/* WTPRebootStatistics */
	wtprebootstat = (struct capwap_wtprebootstat_element*)capwap_get_message_element_data(packet, CAPWAP_ELEMENT_WTPREBOOTSTAT);
	if (wtprebootstat) {
		jsonhash = json_object_new_object();
		json_object_object_add(jsonhash, "RebootCount", json_object_new_int((int)wtprebootstat->rebootcount));
		json_object_object_add(jsonhash, "ACInitiatedCount", json_object_new_int((int)wtprebootstat->acinitiatedcount));
		json_object_object_add(jsonhash, "LinkFailureCount", json_object_new_int((int)wtprebootstat->linkfailurecount));
		json_object_object_add(jsonhash, "SWFailureCount", json_object_new_int((int)wtprebootstat->swfailurecount));
		json_object_object_add(jsonhash, "HWFailureCount", json_object_new_int((int)wtprebootstat->hwfailurecount));
		json_object_object_add(jsonhash, "OtherFailureCount", json_object_new_int((int)wtprebootstat->otherfailurecount));
		json_object_object_add(jsonhash, "UnknownFailureCount", json_object_new_int((int)wtprebootstat->unknownfailurecount));
		json_object_object_add(jsonhash, "LastFailureType", json_object_new_int((int)wtprebootstat->lastfailuretype));
		json_object_object_add(jsonparam, "WTPRebootStatistics", jsonhash);
	}

	/* Get JSON param and convert base64 */
	jsonmessage = json_object_to_json_string(jsonparam);
	base64confstatus = capwap_alloc(AC_BASE64_ENCODE_LENGTH(strlen(jsonmessage)));
	ac_base64_string_encode(jsonmessage, base64confstatus);

	/* Send message */
	response = ac_soap_joinevent(session, session->wtpid, base64confstatus);

	/* Free JSON */
	json_object_put(jsonparam);
	capwap_free(base64confstatus);

	return response;
}

/* */
static uint32_t ac_dfa_state_join_create_response(struct ac_session_t* session, struct capwap_parsed_packet* packet, struct ac_soap_response* response, struct capwap_packet_txmng* txmngpacket) {
	int i;
	int j;
	int length;
	char* json;
	xmlChar* xmlResult;
	struct json_object* jsonroot;
	struct json_object* jsonelement;
	struct capwap_list* controllist;
	struct capwap_list_item* item;
	unsigned short binding = GET_WBID_HEADER(packet->rxmngpacket->header);

	if ((response->responsecode != HTTP_RESULT_OK) || !response->xmlResponseReturn) {
		return CAPWAP_RESULTCODE_FAILURE;
	}

	/* Receive SOAP response with JSON result
		{
			WTPRadioInformation: [
				<IEEE 802.11 BINDING>
				IEEE80211WTPRadioInformation: {
					RadioID: [int],
					IEEE80211nMode: [bool],
					IEEE80211gMode: [bool],
					IEEE80211bMode: [bool],
					IEEE80211aMode: [bool]
				}
			]
			ACIPv4List: [
				{
					ACIPAddress: [string]
				}
			],
			ACIPv6List: [
				{
					ACIPAddress: [string]
				}
			]
		}
	*/

	/* Decode base64 result */
	xmlResult = xmlNodeGetContent(response->xmlResponseReturn);
	if (!xmlResult) {
		return CAPWAP_RESULTCODE_FAILURE;
	}

	length = xmlStrlen(xmlResult);
	if (!length) {
		return CAPWAP_RESULTCODE_FAILURE;
	}

	json = (char*)capwap_alloc(AC_BASE64_DECODE_LENGTH(length));
	ac_base64_string_decode((const char*)xmlResult, json);

	xmlFree(xmlResult);

	/* Parsing JSON result */
	jsonroot = json_tokener_parse(json);
	capwap_free(json);

	/* Add message elements response, every local value can be overwrite from backend server */

	/* AC Descriptor */
	ac_update_statistics();
	capwap_packet_txmng_add_message_element(txmngpacket, CAPWAP_ELEMENT_ACDESCRIPTION, &g_ac.descriptor);

	/* AC Name */
	capwap_packet_txmng_add_message_element(txmngpacket, CAPWAP_ELEMENT_ACNAME, &g_ac.acname);

	/* WTP Radio Information */
	if (binding == CAPWAP_WIRELESS_BINDING_IEEE80211) {
		struct capwap_array* wtpradioinformation = (struct capwap_array*)capwap_get_message_element_data(packet, CAPWAP_ELEMENT_80211_WTPRADIOINFORMATION);

		/* */
		jsonelement = NULL;
		if (jsonroot) {
			jsonelement = json_object_object_get(jsonroot, "WTPRadioInformation");
			if (jsonelement && (json_object_get_type(jsonelement) == json_type_array)) {
				length = json_object_array_length(jsonelement);
			} else {
				jsonelement = NULL;
			}
		}

		/* */
		for (i = 0; i < wtpradioinformation->count; i++) {
			struct capwap_80211_wtpradioinformation_element radio;

			/* Override WTP Radio Information value with SOAP response */
			memcpy(&radio, *(struct capwap_80211_wtpradioinformation_element**)capwap_array_get_item_pointer(wtpradioinformation, i), sizeof(struct capwap_80211_wtpradioinformation_element));
			if (jsonelement && length) {
				for (j = 0; j < length; j++) {
					struct json_object* jsonvalue = json_object_array_get_idx(jsonelement, i);
					if (jsonvalue && (json_object_get_type(jsonvalue) == json_type_object)) {
						struct json_object* jsonitem;

						/* RadioID */
						jsonitem = json_object_object_get(jsonvalue, "RadioID");
						if (jsonitem && (json_object_get_type(jsonitem) == json_type_int)) {
							if ((int)radio.radioid == json_object_get_int(jsonitem)) {
								radio.radiotype = 0;

								/* IEEE80211nMode */
								jsonitem = json_object_object_get(jsonvalue, "IEEE80211nMode");
								if (jsonitem && (json_object_get_type(jsonitem) == json_type_boolean)) {
									radio.radiotype |= (json_object_get_boolean(jsonitem) ? CAPWAP_RADIO_TYPE_80211N : 0);
								}

								/* IEEE80211gMode */
								jsonitem = json_object_object_get(jsonvalue, "IEEE80211gMode");
								if (jsonitem && (json_object_get_type(jsonitem) == json_type_boolean)) {
									radio.radiotype |= (json_object_get_boolean(jsonitem) ? CAPWAP_RADIO_TYPE_80211G : 0);
								}

								/* IEEE80211bMode */
								jsonitem = json_object_object_get(jsonvalue, "IEEE80211bMode");
								if (jsonitem && (json_object_get_type(jsonitem) == json_type_boolean)) {
									radio.radiotype |= (json_object_get_boolean(jsonitem) ? CAPWAP_RADIO_TYPE_80211B : 0);
								}

								/* IEEE80211aMode */
								jsonitem = json_object_object_get(jsonvalue, "IEEE80211aMode");
								if (jsonitem && (json_object_get_type(jsonitem) == json_type_boolean)) {
									radio.radiotype |= (json_object_get_boolean(jsonitem) ? CAPWAP_RADIO_TYPE_80211A : 0);
								}

								break;
							}
						}
					}
				}
			}

			capwap_packet_txmng_add_message_element(txmngpacket, CAPWAP_ELEMENT_80211_WTPRADIOINFORMATION, &radio);
		}
	}

	/* ECN Support */
	capwap_packet_txmng_add_message_element(txmngpacket, CAPWAP_ELEMENT_ECNSUPPORT, &session->dfa.ecn);

	/* Get information from any local address */
	controllist = capwap_list_create();
	ac_get_control_information(controllist);

	/* CAPWAP Control IP Address */
	for (item = controllist->first; item != NULL; item = item->next) {
		struct ac_session_control* sessioncontrol = (struct ac_session_control*)item->item;

		if (sessioncontrol->localaddress.ss_family == AF_INET) {
			struct capwap_controlipv4_element element;

			memcpy(&element.address, &((struct sockaddr_in*)&sessioncontrol->localaddress)->sin_addr, sizeof(struct in_addr));
			element.wtpcount = sessioncontrol->count;
			capwap_packet_txmng_add_message_element(txmngpacket, CAPWAP_ELEMENT_CONTROLIPV4, &element);
		} else if (sessioncontrol->localaddress.ss_family == AF_INET6) {
			struct capwap_controlipv6_element element;

			memcpy(&element.address, &((struct sockaddr_in6*)&sessioncontrol->localaddress)->sin6_addr, sizeof(struct in6_addr));
			element.wtpcount = sessioncontrol->count;
			capwap_packet_txmng_add_message_element(txmngpacket, CAPWAP_ELEMENT_CONTROLIPV6, &element);
		}
	}

	capwap_list_free(controllist);

	/* CAPWAP Local IP Address */
	if (session->acctrladdress.ss_family == AF_INET) {
		struct capwap_localipv4_element addr;

		memcpy(&addr.address, &((struct sockaddr_in*)&session->acctrladdress)->sin_addr, sizeof(struct in_addr));
		capwap_packet_txmng_add_message_element(txmngpacket, CAPWAP_ELEMENT_LOCALIPV4, &addr);
	} else if (session->acctrladdress.ss_family == AF_INET6) {
		struct capwap_localipv6_element addr;

		memcpy(&addr.address, &((struct sockaddr_in6*)&session->acctrladdress)->sin6_addr, sizeof(struct in6_addr));
		capwap_packet_txmng_add_message_element(txmngpacket, CAPWAP_ELEMENT_LOCALIPV6, &addr);
	}

	/* ACIPv4List */
	jsonelement = NULL;
	if (jsonroot) {
		jsonelement = json_object_object_get(jsonroot, "ACIPv4List");
		if (jsonelement && (json_object_get_type(jsonelement) == json_type_array)) {
			length = json_object_array_length(jsonelement);
		} else {
			jsonelement = NULL;
		}
	}

	if (jsonelement) {
		struct capwap_acipv4list_element* responseacipv4list;

		responseacipv4list = (struct capwap_acipv4list_element*)capwap_alloc(sizeof(struct capwap_acipv4list_element));
		responseacipv4list->addresses = capwap_array_create(sizeof(struct in_addr), 0, 0);

		for (j = 0; j < length; j++) {
			struct json_object* jsonvalue = json_object_array_get_idx(jsonelement, i);
			if (jsonvalue && (json_object_get_type(jsonvalue) == json_type_object)) {
				struct json_object* jsonitem;

				/* ACIPAddress */
				jsonitem = json_object_object_get(jsonvalue, "ACIPAddress");
				if (jsonitem && (json_object_get_type(jsonitem) == json_type_string)) {
					const char* value = json_object_get_string(jsonitem);
					if (value) {
						struct sockaddr_storage address;
						if (capwap_address_from_string(value, &address)) {
							/* Accept only IPv4 address */
							if (address.ss_family == AF_INET) {
								struct sockaddr_in* address_in = (struct sockaddr_in*)&address;
								struct in_addr* responseaddress_in = (struct in_addr*)capwap_array_get_item_pointer(responseacipv4list->addresses, responseacipv4list->addresses->count);
								memcpy(responseaddress_in, &address_in->sin_addr, sizeof(struct in_addr));
							}
						}
					}
				}
			}
		}

		if (responseacipv4list->addresses->count > 0) {
			capwap_packet_txmng_add_message_element(txmngpacket, CAPWAP_ELEMENT_ACIPV4LIST, responseacipv4list);
		}

		capwap_array_free(responseacipv4list->addresses);
		capwap_free(responseacipv4list);
	} else if (session->dfa.acipv4list.addresses->count > 0) {
		capwap_packet_txmng_add_message_element(txmngpacket, CAPWAP_ELEMENT_ACIPV4LIST, &session->dfa.acipv4list);
	}

	/* ACIPv6List */
	jsonelement = NULL;
	if (jsonroot) {
		jsonelement = json_object_object_get(jsonroot, "ACIPv6List");
		if (jsonelement && (json_object_get_type(jsonelement) == json_type_array)) {
			length = json_object_array_length(jsonelement);
		} else {
			jsonelement = NULL;
		}
	}

	if (jsonelement) {
		int j;
		struct capwap_acipv6list_element* responseacipv6list;

		responseacipv6list = (struct capwap_acipv6list_element*)capwap_alloc(sizeof(struct capwap_acipv6list_element));
		responseacipv6list->addresses = capwap_array_create(sizeof(struct in6_addr), 0, 0);

		for (j = 0; j < length; j++) {
			struct json_object* jsonvalue = json_object_array_get_idx(jsonelement, i);
			if (jsonvalue && (json_object_get_type(jsonvalue) == json_type_object)) {
				struct json_object* jsonitem;

				/* ACIPAddress */
				jsonitem = json_object_object_get(jsonvalue, "ACIPAddress");
				if (jsonitem && (json_object_get_type(jsonitem) == json_type_string)) {
					const char* value = json_object_get_string(jsonitem);
					if (value) {
						struct sockaddr_storage address;
						if (capwap_address_from_string(value, &address)) {
							/* Accept only IPv6 address */
							if (address.ss_family == AF_INET6) {
								struct sockaddr_in6* address_in6 = (struct sockaddr_in6*)&address;
								struct in6_addr* responseaddress_in6 = (struct in6_addr*)capwap_array_get_item_pointer(responseacipv6list->addresses, responseacipv6list->addresses->count);
								memcpy(responseaddress_in6, &address_in6->sin6_addr, sizeof(struct in6_addr));
							}
						}
					}
				}
			}
		}

		if (responseacipv6list->addresses->count > 0) {
			capwap_packet_txmng_add_message_element(txmngpacket, CAPWAP_ELEMENT_ACIPV6LIST, responseacipv6list);
		}

		capwap_array_free(responseacipv6list->addresses);
		capwap_free(responseacipv6list);
	} else if (session->dfa.acipv6list.addresses->count > 0) {
		capwap_packet_txmng_add_message_element(txmngpacket, CAPWAP_ELEMENT_ACIPV6LIST, &session->dfa.acipv6list);
	}

	/* CAPWAP Transport Protocol */
	capwap_packet_txmng_add_message_element(txmngpacket, CAPWAP_ELEMENT_TRANSPORT, &session->dfa.transport);

	/* CAPWAP_ELEMENT_IMAGEIDENTIFIER */				/* TODO */
	/* CAPWAP_ELEMENT_MAXIMUMLENGTH */					/* TODO */
	/* CAPWAP_ELEMENT_VENDORPAYLOAD */					/* TODO */

	if (jsonroot) {
		json_object_put(jsonroot);
	}

	return CAPWAP_RESULTCODE_SUCCESS;
}

/* */
int ac_dfa_state_join(struct ac_session_t* session, struct capwap_parsed_packet* packet) {
	struct ac_soap_response* response;
	struct capwap_header_data capwapheader;
	struct capwap_packet_txmng* txmngpacket;
	struct capwap_sessionid_element* sessionid;
	struct capwap_wtpboarddata_element* wtpboarddata;
	int status = AC_DFA_ACCEPT_PACKET;
	struct capwap_resultcode_element resultcode = { .code = CAPWAP_RESULTCODE_FAILURE };

	ASSERT(session != NULL);
	
	if (packet) {
		unsigned short binding;

		/* Check binding */
		binding = GET_WBID_HEADER(packet->rxmngpacket->header);
		if (ac_valid_binding(binding)) {
			if (packet->rxmngpacket->ctrlmsg.type == CAPWAP_JOIN_REQUEST) {
				/* Get sessionid and verify unique id */
				sessionid = (struct capwap_sessionid_element*)capwap_get_message_element_data(packet, CAPWAP_ELEMENT_SESSIONID);
				if (!ac_has_sessionid(sessionid)) {
					char* wtpid;

					/* Checking macaddress for detect if WTP already connected */
					wtpboarddata = (struct capwap_wtpboarddata_element*)capwap_get_message_element_data(packet, CAPWAP_ELEMENT_WTPBOARDDATA);

					/* Get printable WTPID */
					wtpid = ac_get_printable_wtpid(wtpboarddata);
					if (wtpid && !ac_has_wtpid(wtpid)) {
						/* Request authorization of Backend for complete join */
						response = ac_soap_authorizejoin(session, wtpid);
						if (response) {
							resultcode.code = ac_dfa_state_join_check_authorizejoin(session, response);
							ac_soapclient_free_response(response);
						} else {
							resultcode.code = CAPWAP_RESULTCODE_JOIN_FAILURE_UNKNOWN_SOURCE;
						}
					} else {
						resultcode.code = CAPWAP_RESULTCODE_JOIN_FAILURE_UNKNOWN_SOURCE;
					}

					/* */
					if (CAPWAP_RESULTCODE_OK(resultcode.code)) {
						session->wtpid = wtpid;
						memcpy(&session->sessionid, sessionid, sizeof(struct capwap_sessionid_element));
						session->binding = binding;
					} else {
						capwap_free(wtpid);
					}
				} else {
					resultcode.code = CAPWAP_RESULTCODE_JOIN_FAILURE_ID_ALREADY_IN_USE;
				}
			} else {
				resultcode.code = CAPWAP_RESULTCODE_MSG_UNEXPECTED_INVALID_CURRENT_STATE;
			}
		} else {
			resultcode.code = CAPWAP_RESULTCODE_JOIN_FAILURE_BINDING_NOT_SUPPORTED;
		}

		/* Create response */
		capwap_header_init(&capwapheader, CAPWAP_RADIOID_NONE, binding);
		txmngpacket = capwap_packet_txmng_create_ctrl_message(&capwapheader, CAPWAP_JOIN_RESPONSE, packet->rxmngpacket->ctrlmsg.seq, session->mtu);

		/* */
		if (CAPWAP_RESULTCODE_OK(resultcode.code)) {
			response = ac_dfa_state_join_parsing_request(session, packet);
			if (response) {
				resultcode.code = ac_dfa_state_join_create_response(session, packet, response, txmngpacket);
				ac_soapclient_free_response(response);
			}
		}

		/* Add always result code message element */
		capwap_packet_txmng_add_message_element(txmngpacket, CAPWAP_ELEMENT_RESULTCODE, &resultcode);

		/* Join response complete, get fragment packets */
		ac_free_reference_last_response(session);
		capwap_packet_txmng_get_fragment_packets(txmngpacket, session->responsefragmentpacket, session->fragmentid);
		if (session->responsefragmentpacket->count > 1) {
			session->fragmentid++;
		}

		/* Free packets manager */
		capwap_packet_txmng_free(txmngpacket);

		/* Save remote sequence number */
		session->remoteseqnumber = packet->rxmngpacket->ctrlmsg.seq;
		capwap_get_packet_digest(packet->rxmngpacket, packet->connection, session->lastrecvpackethash);

		/* Send Join response to WTP */
		if (capwap_crypt_sendto_fragmentpacket(&session->ctrldtls, session->ctrlsocket.socket[session->ctrlsocket.type], session->responsefragmentpacket, &session->acctrladdress, &session->wtpctrladdress)) {
			if (CAPWAP_RESULTCODE_OK(resultcode.code)) {
				ac_dfa_change_state(session, CAPWAP_POSTJOIN_STATE);
			} else {
				ac_dfa_change_state(session, CAPWAP_JOIN_TO_DTLS_TEARDOWN_STATE);
				status = AC_DFA_NO_PACKET;
			}
		} else {
			/* Error to send packets */
			capwap_logging_debug("Warning: error to send join response packet");
			ac_dfa_change_state(session, CAPWAP_JOIN_TO_DTLS_TEARDOWN_STATE);
			status = AC_DFA_NO_PACKET;
		}
	} else {
		/* Join timeout */
		ac_dfa_change_state(session, CAPWAP_JOIN_TO_DTLS_TEARDOWN_STATE);
		status = AC_DFA_NO_PACKET;
	}

	return status;
}

/* */
int ac_dfa_state_postjoin(struct ac_session_t* session, struct capwap_parsed_packet* packet) {
	int status = AC_DFA_ACCEPT_PACKET;

	ASSERT(session != NULL);

	if (packet) {
		if (packet->rxmngpacket->ctrlmsg.type == CAPWAP_CONFIGURATION_STATUS_REQUEST) {
			ac_dfa_change_state(session, CAPWAP_CONFIGURE_STATE);
			status = ac_dfa_state_configure(session, packet);
		} else if (packet->rxmngpacket->ctrlmsg.type == CAPWAP_IMAGE_DATA_REQUEST) {
			ac_dfa_change_state(session, CAPWAP_IMAGE_DATA_STATE);
			status = ac_dfa_state_imagedata(session, packet);
		} else {
			ac_dfa_change_state(session, CAPWAP_JOIN_TO_DTLS_TEARDOWN_STATE);
			status = AC_DFA_NO_PACKET;
		}
	} else {
		/* Join timeout */
		ac_dfa_change_state(session, CAPWAP_JOIN_TO_DTLS_TEARDOWN_STATE);
		status = AC_DFA_NO_PACKET;
	}

	return status;
}

/* */
int ac_dfa_state_join_to_dtlsteardown(struct ac_session_t* session, struct capwap_parsed_packet* packet) {
	return ac_session_teardown_connection(session);
}
