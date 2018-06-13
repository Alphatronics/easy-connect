/*
 * FILE: easy-connect.cpp
 *
 * Copyright (c) 2015 - 2017 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed.h"
#include "easy-connect.h"

/*
 * Instantiate the configured network interface
 */
#if MBED_CONF_APP_NETWORK_INTERFACE == ETHERNET
#include "EthernetInterface.h"
EthernetInterface eth;


#elif MBED_CONF_APP_NETWORK_INTERFACE == CELLULAR_ONBOARD
#include "OnboardCellularInterface.h"
OnboardCellularInterface cellular;

#elif MBED_CONF_APP_NETWORK_INTERFACE == CELLULAR
#include "EasyCellularConnection.h"
EasyCellularConnection cellular;


#else
#error "No connectivity method chosen. Please add 'config.network-interfaces.value' to your mbed_app.json (see README.md for more information)."
#endif // MBED_CONF_APP_NETWORK_INTERFACE





/* \brief print_MAC - print_MAC  - helper function to print out MAC address
 * in: network_interface - pointer to network i/f
 *     bool log-messages   print out logs or not
 * MAC address is printed, if it can be acquired & log_messages is true.
 *
 */
void print_MAC(NetworkInterface* network_interface, bool log_messages) {
#if MBED_CONF_APP_NETWORK_INTERFACE != CELLULAR_ONBOARD && MBED_CONF_APP_NETWORK_INTERFACE != CELLULAR
    const char *mac_addr = network_interface->get_mac_address();
    if (mac_addr == NULL) {
        if (log_messages) {
            printf("[EasyConnect] ERROR - No MAC address\n");
        }
        return;
    }
    if (log_messages) {
        printf("[EasyConnect] MAC address %s\n", mac_addr);
    }
#endif
}



/* \brief easy_connect     easy_connect() function to connect the pre-defined network bearer,
 *                         config done via mbed_app.json (see README.md for details).
 *
 * IN: bool  log_messages  print out diagnostics or not.
 */
NetworkInterface* easy_connect(const char * apn, bool log_messages) {
    NetworkInterface* network_interface = NULL;
    int connect_success = -1;

    /// This should be removed once mbedOS supports proper dual-stack
    if (log_messages) {
#if defined (EASY_CONNECT_MESH) || (MBED_CONF_LWIP_IPV6_ENABLED==true)
        printf("[EasyConnect] IPv6 mode\n");
#else
        printf("[EasyConnect] IPv4 mode\n");
#endif
    }


#if MBED_CONF_APP_NETWORK_INTERFACE == CELLULAR_ONBOARD || MBED_CONF_APP_NETWORK_INTERFACE == CELLULAR
#  ifdef MBED_CONF_APP_CELLULAR_SIM_PIN
    cellular.set_sim_pin(MBED_CONF_APP_CELLULAR_SIM_PIN);
#  endif
#    ifndef MBED_CONF_APP_CELLULAR_USERNAME
#      define MBED_CONF_APP_CELLULAR_USERNAME 0
#    endif
#    ifndef MBED_CONF_APP_CELLULAR_PASSWORD
#      define MBED_CONF_APP_CELLULAR_PASSWORD 0
#    endif
    cellular.modem_debug_on(log_messages);
    cellular.set_credentials(apn, MBED_CONF_APP_CELLULAR_USERNAME, MBED_CONF_APP_CELLULAR_PASSWORD);
    if (log_messages) {
        printf("[EasyConnect] Connecting using Cellular interface and APN %s\n", apn);
    }

    connect_success = cellular.connect();
    network_interface = &cellular;

#elif MBED_CONF_APP_NETWORK_INTERFACE == ETHERNET
    if (log_messages) {
        printf("[EasyConnect] Using Ethernet\n");
    }
    network_interface = &eth;
    connect_success = eth.connect();
#endif


    if(connect_success == 0) {
        if (log_messages) {
            printf("[EasyConnect] Connected to Network successfully\n");
            print_MAC(network_interface, log_messages);
        }
    } else {
        if (log_messages) {
            print_MAC(network_interface, log_messages);
            printf("[EasyConnect] Connection to Network Failed %d!\n", connect_success);
        }
        return NULL;
    }
    const char *ip_addr  = network_interface->get_ip_address();
    if (ip_addr == NULL) {
        if (log_messages) {
            printf("[EasyConnect] ERROR - No IP address\n");
        }
        return NULL;
    }

    if (log_messages) {
        printf("[EasyConnect] IP address %s\n", ip_addr);
    }
    return network_interface;
}


/* \brief easy_get_netif - easy_connect function to get pointer to network interface
 *                        without connecting to it.
 *
 * IN: bool  log_messages  print out diagnostics or not.
 */
NetworkInterface* easy_get_netif(bool log_messages) {

#if MBED_CONF_APP_NETWORK_INTERFACE == ETHERNET
    if (log_messages) {
        printf("[EasyConnect] Ethernet\n");
    }
    return &eth;


#elif MBED_CONF_APP_NETWORK_INTERFACE == CELLULAR_ONBOARD || MBED_CONF_APP_NETWORK_INTERFACE == CELLULAR
    if (log_messages) {
        printf("[EasyConnect] Cellular\n");
    }
    return  &cellular;
#endif
}

