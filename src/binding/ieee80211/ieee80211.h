#ifndef __CAPWAP_IEEE802_11_HEADER__
#define __CAPWAP_IEEE802_11_HEADER__

#include <linux/types.h>
#include <asm/byteorder.h>
#include <linux/if_ether.h>

#ifndef STRUCT_PACKED
#define STRUCT_PACKED					__attribute__((__packed__))
#endif

/* Global values */
#define IEEE80211_MTU										2304

#define IS_IEEE80211_FREQ_BG(x)			(((x >= 2412) && (x <= 2484)) ? 1 : 0)
#define IS_IEEE80211_FREQ_A(x)			(((x >= 5035) && (x <= 5825)) ? 1 : 0)

/* Rate into multiple of 500Kbps */
#define IEEE80211_RATE_1M				2
#define IEEE80211_RATE_2M				4
#define IEEE80211_RATE_5_5M				11
#define IEEE80211_RATE_11M				22
#define IEEE80211_RATE_6M				12
#define IEEE80211_RATE_9M				18
#define IEEE80211_RATE_12M				24
#define IEEE80211_RATE_18M				36
#define IEEE80211_RATE_24M				48
#define IEEE80211_RATE_36M				72
#define IEEE80211_RATE_48M				96
#define IEEE80211_RATE_54M				108
#define IEEE80211_RATE_80211N			127

#define IS_IEEE80211_RATE_B(x)			(((x == IEEE80211_RATE_1M) || (x == IEEE80211_RATE_2M) || (x == IEEE80211_RATE_5_5M) || (x == IEEE80211_RATE_11M)) ? 1 : 0)
#define IS_IEEE80211_RATE_G(x)			(((x == IEEE80211_RATE_6M) || (x == IEEE80211_RATE_9M) || (x == IEEE80211_RATE_12M) || (x == IEEE80211_RATE_18M) || (x == IEEE80211_RATE_24M) || (x == IEEE80211_RATE_36M) || (x == IEEE80211_RATE_48M) || (x == IEEE80211_RATE_54M)) ? 1 : 0)
#define IS_IEEE80211_RATE_A(x)			(((x == IEEE80211_RATE_6M) || (x == IEEE80211_RATE_9M) || (x == IEEE80211_RATE_12M) || (x == IEEE80211_RATE_18M) || (x == IEEE80211_RATE_24M) || (x == IEEE80211_RATE_36M) || (x == IEEE80211_RATE_48M) || (x == IEEE80211_RATE_54M)) ? 1 : 0)
#define IS_IEEE80211_RATE_N(x)			((x == IEEE80211_RATE_80211N) ? 1 : 0)

/* Frame control type */
#define IEEE80211_FRAMECONTROL_TYPE_MGMT					0
#define IEEE80211_FRAMECONTROL_TYPE_CTRL					1
#define IEEE80211_FRAMECONTROL_TYPE_DATA					2

/* Frame control Management subtype */
#define IEEE80211_FRAMECONTROL_MGMT_SUBTYPE_ASSOC_REQ				0
#define IEEE80211_FRAMECONTROL_MGMT_SUBTYPE_ASSOC_RESP				1
#define IEEE80211_FRAMECONTROL_MGMT_SUBTYPE_REASSOC_REQ				2
#define IEEE80211_FRAMECONTROL_MGMT_SUBTYPE_REASSOC_RESP			3
#define IEEE80211_FRAMECONTROL_MGMT_SUBTYPE_PROBE_REQ				4
#define IEEE80211_FRAMECONTROL_MGMT_SUBTYPE_PROBE_RESP				5
#define IEEE80211_FRAMECONTROL_MGMT_SUBTYPE_TIMING_ADV				6
#define IEEE80211_FRAMECONTROL_MGMT_SUBTYPE_BEACON					8
#define IEEE80211_FRAMECONTROL_MGMT_SUBTYPE_ATIM					9
#define IEEE80211_FRAMECONTROL_MGMT_SUBTYPE_DISASSOC				10
#define IEEE80211_FRAMECONTROL_MGMT_SUBTYPE_AUTH					11
#define IEEE80211_FRAMECONTROL_MGMT_SUBTYPE_DEAUTH					12
#define IEEE80211_FRAMECONTROL_MGMT_SUBTYPE_ACTION					13
#define IEEE80211_FRAMECONTROL_MGMT_SUBTYPE_ACTION_NOACK			14

/* Frame control Control subtype */
#define IEEE80211_FRAMECONTROL_CTRL_SUBTYPE_CTRLWRAPPER				7
#define IEEE80211_FRAMECONTROL_CTRL_SUBTYPE_BLOCKACK_REQ			8
#define IEEE80211_FRAMECONTROL_CTRL_SUBTYPE_BLOCKACK				9
#define IEEE80211_FRAMECONTROL_CTRL_SUBTYPE_PSPOLL					10
#define IEEE80211_FRAMECONTROL_CTRL_SUBTYPE_RTS						11
#define IEEE80211_FRAMECONTROL_CTRL_SUBTYPE_CTS						12
#define IEEE80211_FRAMECONTROL_CTRL_SUBTYPE_ACK						13
#define IEEE80211_FRAMECONTROL_CTRL_SUBTYPE_CFEND					14
#define IEEE80211_FRAMECONTROL_CTRL_SUBTYPE_CFEND_CFACK				15

/* Frame control Data subtype */
#define IEEE80211_FRAMECONTROL_DATA_SUBTYPE_DATA					0
#define IEEE80211_FRAMECONTROL_DATA_SUBTYPE_DATA_CFACK				1
#define IEEE80211_FRAMECONTROL_DATA_SUBTYPE_DATA_CFPOLL				2
#define IEEE80211_FRAMECONTROL_DATA_SUBTYPE_DATA_CFACK_CFPOLL		3
#define IEEE80211_FRAMECONTROL_DATA_SUBTYPE_NULL					4
#define IEEE80211_FRAMECONTROL_DATA_SUBTYPE_CFACK					5
#define IEEE80211_FRAMECONTROL_DATA_SUBTYPE_CFPOLL					6
#define IEEE80211_FRAMECONTROL_DATA_SUBTYPE_CFACK_CFPOLL			7
#define IEEE80211_FRAMECONTROL_DATA_SUBTYPE_QOSDATA					8
#define IEEE80211_FRAMECONTROL_DATA_SUBTYPE_QOSDATA_CFACK			9
#define IEEE80211_FRAMECONTROL_DATA_SUBTYPE_QOSDATA_CFPOLL			10
#define IEEE80211_FRAMECONTROL_DATA_SUBTYPE_QOSDATA_CFACK_CFPOLL	11
#define IEEE80211_FRAMECONTROL_DATA_SUBTYPE_QOSNULL					12
#define IEEE80211_FRAMECONTROL_DATA_SUBTYPE_QOSCFPOLL				14
#define IEEE80211_FRAMECONTROL_DATA_SUBTYPE_QOSCFACK_CFPOLL			15

/* */
#define IEEE80211_FRAME_CONTROL(type, stype) 				__cpu_to_le16((type << 2) | (stype << 4))

/* 802.11 Packet - IEEE802.11 is a little-endian protocol */
struct ieee80211_header {
	__le16 framecontrol;
	__le16 durationid;
	uint8_t address1[ETH_ALEN];
	uint8_t address2[ETH_ALEN];
	uint8_t address3[ETH_ALEN];
	__le16 sequencecontrol;
} STRUCT_PACKED;

/* */
struct ieee80211_header_mgmt {
	__le16 framecontrol;
	__le16 durationid;
	uint8_t da[ETH_ALEN];
	uint8_t sa[ETH_ALEN];
	uint8_t bssid[ETH_ALEN];
	__le16 sequencecontrol;

	uint8_t timestamp[8];
	__le16 beaconinterval;
	__le16 capability;
} STRUCT_PACKED;

/* 802.11 Generic information element */
struct ieee80211_ie {
	uint8_t id;
	uint8_t len;
} STRUCT_PACKED;

/* 802.11 SSID information element */
#define IEEE80211_IE_SSID							0
#define IEEE80211_IE_SSID_MAX_LENGTH				32

struct ieee80211_ie_ssid {
	uint8_t id;
	uint8_t len;
	uint8_t ssid[0];
} STRUCT_PACKED;

/* 802.11 Supported Rates information element */
#define IEEE80211_IE_SUPPORTED_RATES				1
#define IEEE80211_IE_SUPPORTED_RATES_MAX_LENGTH		8

struct ieee80211_ie_supported_rates {
	uint8_t id;
	uint8_t len;
	uint8_t rates[0];
} STRUCT_PACKED;

/* 802.11 DSSS information element */
#define IEEE80211_IE_DSSS							3

struct ieee80211_ie_dsss {
	uint8_t id;
	uint8_t len;
	uint8_t channel;
} STRUCT_PACKED;

/* 802.11 Country information element */
#define IEEE80211_IE_COUNTRY						7

struct ieee80211_ie_country_channelgroup {
	uint8_t firstchannel;
	uint8_t numberchannels;
	uint8_t maxtxpower;
} STRUCT_PACKED;

struct ieee80211_ie_country {
	uint8_t id;
	uint8_t len;
	uint8_t country[3];
	uint8_t channelgroup[0];
} STRUCT_PACKED;

/* 802.11 ERP information element */
#define IEEE80211_IE_ERP							42

struct ieee80211_ie_erp {
	uint8_t id;
	uint8_t len;
	uint8_t params;
} STRUCT_PACKED;

/* 802.11 Extended Supported Rates information element */
#define IEEE80211_IE_EXTENDED_SUPPORTED_RATES		50

struct ieee80211_ie_extended_supported_rates {
	uint8_t id;
	uint8_t len;
	uint8_t rates[0];
} STRUCT_PACKED;

/* 802.11 EDCA Parameter Set information element */
#define IEEE80211_IE_EDCA_PARAMETER_SET				12
#define IEEE80211_IE_EDCA_PARAMETER_SET_LENGTH		18

#define EDCA_PARAMETER_RECORD_AC_BE_FIELD			0
#define EDCA_PARAMETER_RECORD_AC_BK_FIELD			1
#define EDCA_PARAMETER_RECORD_AC_VI_FIELD			2
#define EDCA_PARAMETER_RECORD_AC_VO_FIELD			3

struct ieee80211_ie_edca_parameter_set {
	/* TODO */
} STRUCT_PACKED;

/* 802.11 QoS Capability information element */
#define IEEE80211_IE_QOS_CAPABILITY					46
#define IEEE80211_IE_QOS_CAPABILITY_LENGTH			1

struct ieee80211_ie_qos_capability {
	/* TODO */
};

/* 802.11 Power Constraint information element */
#define IEEE80211_IE_POWER_CONSTRAINT				52
#define IEEE80211_IE_POWER_CONSTRAINT_LENGTH		1

struct ieee80211_ie_power_constraint {
	/* TODO */
} STRUCT_PACKED;

#endif /* __CAPWAP_IEEE802_11_HEADER__ */
