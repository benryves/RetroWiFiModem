/*
 * PPPoS serial output callback
 *
 * ppp_pcb, PPP control block
 * data, buffer to write to serial port
 * len, length of the data buffer
 * ctx, optional user-provided callback context pointer
 *
 * Return value: len if write succeed
 */
static u32_t ppp_output_cb(ppp_pcb *pcb, unsigned char *data, u32_t len, void *ctx) {
   if (state == ONLINE) {
      return Serial.write(data, len);
   } else {
      return 0;
   }
}

// fires when connection established or torn down
static void ppp_status_cb(ppp_pcb *pcb, int err_code, void *ctx) {
  struct netif *pppif = ppp_netif(pcb);
  LWIP_UNUSED_ARG(ctx);

  switch(err_code) {
    case PPPERR_NONE: // No error == connected successfully
#if DEBUG
      Serial.println(F("status_cb: Connected"));
#if PPP_IPV4_SUPPORT
      Serial.print(F("   our_ipaddr  = "));
      Serial.println(ipaddr_ntoa(&pppif->ip_addr));
      Serial.print(F("   his_ipaddr  = "));
      Serial.println(ipaddr_ntoa(&pppif->gw));
      Serial.print(F("   netmask     = "));
      Serial.println(ipaddr_ntoa(&pppif->netmask));
#if LWIP_DNS
      const ip_addr_t *ns;
      ns = dns_getserver(0);
      Serial.print(F("   dns1        = "));
      Serial.println(ipaddr_ntoa(ns));
      ns = dns_getserver(1);
      Serial.print(F("   dns2        = "));
      Serial.println(ipaddr_ntoa(ns));
#endif /* LWIP_DNS */
#endif /* PPP_IPV4_SUPPORT */
#if PPP_IPV6_SUPPORT
      //printf("   our6_ipaddr = %s\n", ip6addr_ntoa(netif_ip6_addr(pppif, 0)));
#endif /* PPP_IPV6_SUPPORT */
#endif /* DEBUG */
      // Enable NAT-ing this connection
      ip_napt_enable(pppif->ip_addr.addr, 1);
      break;
    case PPPERR_USER: // clean disconnect
      // Serial.println(F("PPP: shutdown"));
      if (ppp_free(ppp) == 0) {
        ppp = NULL;
        // Serial.println(F("PPP: freed"));
      } else {
        Serial.println(F("ppp_free() failed"));
      }
      break;
    // anything other than NONE or USER is an error, abort and go back to command mode
    case PPPERR_PARAM:
      Serial.println(F("PPP: Invalid parameter"));
      break;
    case PPPERR_OPEN:
      Serial.println(F("PPP: Unable to open PPP session"));
      break;
    case PPPERR_DEVICE:
      Serial.println(F("PPP: Invalid I/O device"));
      break;
    case PPPERR_ALLOC:
      Serial.println(F("PPP: Unable to allocate resources"));
      break;
    case PPPERR_CONNECT:
      Serial.println(F("PPP: Connection lost"));
      break;
    case PPPERR_AUTHFAIL:
      Serial.println(F("PPP: Failed authentication challenge"));
      break;
    case PPPERR_PROTOCOL:
      Serial.println(F("PPP: Failed to meet protocol"));
      break;
    case PPPERR_PEERDEAD:
      Serial.println(F("PPP: Connection timeout"));
      break;
    case PPPERR_IDLETIMEOUT:
      Serial.println(F("PPP: Idle Timeout"));
      break;
    case PPPERR_CONNECTTIME:
      Serial.println(F("PPP: Max connect time reached"));
      break;
    case PPPERR_LOOPBACK:
      Serial.println(F("PPP: Loopback detected"));
      break;
  }

  // for any error other than USER or NONE, hang up
  if (!((err_code == PPPERR_USER) || (err_code == PPPERR_NONE))) {
    endCall();
  }
}
