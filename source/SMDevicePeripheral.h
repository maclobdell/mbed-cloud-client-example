// ----------------------------------------------------------------------------
// Copyright 2016-2019 ARM Ltd.
//
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------

#ifndef SMDEVICEPERIPHERAL_H
#define SMDEVICEPERIPHERAL_H

#include "SMDevice.h"

#include <events/mbed_events.h>
#include "ble/BLE.h"
#include "SecurityManager.h"

/** A peripheral device will advertise, accept the connection and request
 * a change in link security. */
class SMDevicePeripheral : public SMDevice {
public:
    SMDevicePeripheral(BLE &ble, events::EventQueue &event_queue, BLEProtocol::AddressBytes_t &peer_address)
        : SMDevice(ble, event_queue, peer_address) { }

    virtual void start()
    {
        /* Set up and start advertising */
        uint8_t adv_buffer[ble::LEGACY_ADVERTISING_MAX_SIZE];
        /* use the helper to build the payload */
        ble::AdvertisingDataBuilder adv_data_builder(
            adv_buffer
        );

        adv_data_builder.setFlags();
        adv_data_builder.setName(MBED_CONF_APP_BLUETOOTH_NAME);

        /* Set payload for the set */
        ble_error_t error = _ble.gap().setAdvertisingPayload(
            ble::LEGACY_ADVERTISING_HANDLE,
            adv_data_builder.getAdvertisingData()
        );

        if (error) {
            print_error(error, "Gap::setAdvertisingPayload() failed");
            _event_queue.break_dispatch();
            return;
        }

        ble::AdvertisingParameters adv_parameters(
            ble::advertising_type_t::CONNECTABLE_UNDIRECTED
        );

        error = _ble.gap().setAdvertisingParameters(
            ble::LEGACY_ADVERTISING_HANDLE,
            adv_parameters
        );

        if (error) {
            print_error(error, "Gap::setAdvertisingParameters() failed");
            return;
        }

        error = _ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);

        if (error) {
            print_error(error, "Gap::startAdvertising() failed");
            return;
        }

        printf("Please connect to device\r\n");

        /** This tells the stack to generate a pairingRequest event
         * which will require this application to respond before pairing
         * can proceed. Setting it to false will automatically accept
         * pairing. */
        _ble.securityManager().setPairingRequestAuthorisation(true);
    };

    /** This is called by Gap to notify the application we connected,
     *  in our case it immediately requests a change in link security */
    virtual void onConnectionComplete(const ble::ConnectionCompleteEvent &event)
    {
        ble_error_t error;

        /* remember the device that connects to us now so we can connect to it
         * during the next demonstration */
        memcpy(_peer_address, event.getPeerAddress().data(), sizeof(_peer_address));

        printf("Connected to peer: ");
        print_address(event.getPeerAddress().data());

        _handle = event.getConnectionHandle();

        /* Request a change in link security. This will be done
         * indirectly by asking the master of the connection to
         * change it. Depending on circumstances different actions
         * may be taken by the master which will trigger events
         * which the applications should deal with. */
        error = _ble.securityManager().setLinkSecurity(
            _handle,
            SecurityManager::SECURITY_MODE_ENCRYPTION_NO_MITM
        );

        if (error) {
            printf("Error during SM::setLinkSecurity %d\r\n", error);
            return;
        }
    };
};

#endif