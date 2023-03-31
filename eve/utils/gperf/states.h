#define CALIX_NETCONF_NEW_DOCUMENT   1
#define CALIX_NETCONF_NOTIFICATION   2
#define CALIX_NETCONF_RPC_REPLY      3

struct calix_netconf_document {
    int state;
    void *data;
};

static void XMLCALL
calix_doc_tag_start(void *user_data, const XML_Char *name, const XML_Char **atts);

static void XMLCALL
calix_doc_tag_end(void *user_data, const XML_Char *name);

static void
calix_doc_char_data(void* user_data, const char* val, int len);

struct calix_netconf_notification {
    int event_time;
};

/*
Notification types:
  ont-missing
  ont-ds-sdber
  ont-us-sdber
  ont-arrival
  ont-departure
  eth-eth-down
  ont-dying-gasp
  low-rx-opt-pwr-ne
  loss-of-pon
  omci-comm-fail
  ont-sw-mismatch
  ont-download-start
  ont-download-fail
*/

/*
new calix netconf xml document
  state: notification
  state: rpc_reply


  notification
    eventTime
    notication type
      description
      ...


  rpc-reply
    data
      status
        [tree, eg.]
        system
          ont
            ont-id
            status
            linkage
            detail
            counters
            pon-us-cos-status
            qos

          ont
            ..



<?xml version="1.0" encoding="UTF-8"?>
<notification xmlns="urn:ietf:params:xml:ns:netconf:notification:1.0"><eventTime>2022-12-26T11:23:38-06:00</eventTime>
<ont-missing xmlns='http://www.calix.com/ns/exa/gpon-interface-base'>
  <description>Provisioned ONT is missing</description>
  <probable-cause>Provisioned ONT is not accessible on the PON.</probable-cause>
  <details>SerialNo=A7162A</details>
  <device-sequence-number>141015</device-sequence-number>
  <instance-id>10.141014</instance-id>
  <id>5020</id>
  <name>ont-missing</name>
  <perceived-severity>CLEAR</perceived-severity>
  <prev-severity>MINOR</prev-severity>
  <alarm>true</alarm>
  <service-impacting>true</service-impacting>
  <service-affecting>true</service-affecting>
  <alarm-type>EQUIPMENT</alarm-type>
  <category>PON</category>
  <hw-failure-event>false</hw-failure-event>
  <repair-action>No action.</repair-action>
  <address>/config/system/ont[ont-id='13387']</address>
  <port>gp3</port>
  <ont-id>13387</ont-id>
  <phys-pon-shelf>1</phys-pon-shelf>
  <phys-pon-slot>2</phys-pon-slot>
  <phys-pon-port>gp3</phys-pon-port>
  <serial-number>A7162A</serial-number>
  <equipment-type>ONT</equipment-type>
  <vendor-id>CXNK</vendor-id>
</ont-missing>
</notification>

<?xml version="1.0" encoding="UTF-8"?>
<rpc-reply xmlns="urn:ietf:params:xml:ns:netconf:base:1.0" message-id="487">
  <data>
    <status xmlns="http://www.calix.com/ns/exa/base">
      <system>
        <ont xmlns="http://www.calix.com/ns/exa/gpon-interface-base">
          <ont-id>511</ont-id>
          <status>
            <vendor>CXNK</vendor>
            <serial-number>BC4023</serial-number>
            <upgrade-class>none</upgrade-class>
            <oper-state>present</oper-state>
            <linked-pon>1/2/gp6</linked-pon>
            <model>803G</model>
            <clei>BVMCN10ARA</clei>
            <curr-version>11.2.6.10.2</curr-version>
            <alt-version>MFG_1.0.24_4.1</alt-version>
            <voip-config-version>0</voip-config-version>
            <rg-config-version>0</rg-config-version>
            <curr-committed>true</curr-committed>
            <onu-mac-addr>84:d3:43:28:dc:3a</onu-mac-addr>
            <mta-mac-addr>84:d3:43:28:dc:3b</mta-mac-addr>
            <mfg-serial-number>422109026888</mfg-serial-number>
            <product-code>P1</product-code>
          </status>
          <linkage>
            <status>Confirmed</status>
            <linked-by>Serial-Number</linked-by>
          </linkage>
          <detail>
            <range-length>11536</range-length>
            <up-time>162238</up-time>
            <opt-signal-level>-20.000</opt-signal-level>
            <tx-opt-level>2.000</tx-opt-level>
            <ne-opt-signal-level>-22.400</ne-opt-signal-level>
            <us-sdber-rate>1.00E-14</us-sdber-rate>
            <ds-sdber-rate>1.00E-14</ds-sdber-rate>
          </detail>
          <counters>
            <ont-counters>
              <upstream-bip-errors>1</upstream-bip-errors>
              <upstream-missed-bursts>0</upstream-missed-bursts>
              <upstream-gem-hec-errors>0</upstream-gem-hec-errors>
              <upstream-bip8-err-sec>1</upstream-bip8-err-sec>
              <upstream-bip8-severely-err-sec>0</upstream-bip8-severely-err-sec>
              <upstream-bip8-unavail-sec>0</upstream-bip8-unavail-sec>
              <upstream-missed-bursts-err-sec>0</upstream-missed-bursts-err-sec>
              <upstream-fec-corrected-bytes>0</upstream-fec-corrected-bytes>
              <upstream-fec-corrected-code-words>0</upstream-fec-corrected-code-words>
              <upstream-fec-uncorrected-code-words>0</upstream-fec-uncorrected-code-words>
              <upstream-fec-total-code-words>0</upstream-fec-total-code-words>
              <upstream-fec-sec>0</upstream-fec-sec>
              <downstream-bip-errors>1</downstream-bip-errors>
              <downstream-bip8-err-sec>1</downstream-bip8-err-sec>
              <downstream-bip8-severely-err-sec>0</downstream-bip8-severely-err-sec>
              <downstream-bip8-unavail-sec>0</downstream-bip8-unavail-sec>
              <downstream-fec-corrected-bytes>0</downstream-fec-corrected-bytes>
              <downstream-fec-corrected-code-words>0</downstream-fec-corrected-code-words>
              <downstream-fec-uncorrected-code-words>0</downstream-fec-uncorrected-code-words>
              <downstream-fec-total-code-words>0</downstream-fec-total-code-words>
              <downstream-fec-sec>0</downstream-fec-sec>
              <doze-time>0</doze-time>
              <cyclic-sleep-time>0</cyclic-sleep-time>
              <watchful-sleep-time>0</watchful-sleep-time>
              <energy-consumed>0</energy-consumed>
              <cpu-percent-utilization>0</cpu-percent-utilization>
              <ram-available-amount>0</ram-available-amount>
              <ram-utilization>0</ram-utilization>
              <flash-available-amount>0</flash-available-amount>
              <flash-utilization>0</flash-utilization>
              <software-errors>0</software-errors>
              <errors-in-operations>0</errors-in-operations>
            </ont-counters>
          </counters>
          <pon-us-cos-status>
            <pon-cos>
              <name>cos-1</name>
              <policy>sp</policy>
              <queue>
                <id>1</id>
                <reference-count>1</reference-count>
                <max-size>256</max-size>
                <queue-depth>256</queue-depth>
                <weight>1</weight>
                <green-minimum-drop-threshold>512</green-minimum-drop-threshold>
                <green-maximum-drop-threshold>512</green-maximum-drop-threshold>
                <green-packet-drop-probability>256 (100.00%)</green-packet-drop-probability>
                <yellow-minimum-drop-threshold>512</yellow-minimum-drop-threshold>
                <yellow-maximum-drop-threshold>512</yellow-maximum-drop-threshold>
                <yellow-packet-drop-probability>256 (100.00%)</yellow-packet-drop-probability>
                <queue-drop-averaging-coefficient>2^(-9)</queue-drop-averaging-coefficient>
              </queue>
            </pon-cos>
          </pon-us-cos-status>
          <qos>
            <pon-qos>
              <bandwidth>
                <index>0</index>
                <vlan>2062</vlan>
                <c-vlan>127</c-vlan>
                <port>511/g1</port>
                <direction>ingress</direction>
                <type>meter</type>
                <pon-cos>cos-1</pon-cos>
                <dba-priority>BE-1</dba-priority>
                <cir>0</cir>
                <eir>100000</eir>
                <pon-cos-air>0</pon-cos-air>
                <pon-cos-eir>100000</pon-cos-eir>
              </bandwidth>
            </pon-qos>
          </qos>
        </ont>

*/
