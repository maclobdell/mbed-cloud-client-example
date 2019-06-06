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

#ifndef SMDEVICECENTRAL_H
#define SMDEVICECENTRAL_H

#include "SMDevice.h"

#include <events/mbed_events.h>
#include "ble/BLE.h"
#include "SecurityManager.h"

/** A central device will scan, connect to a peer and request pairing. */
class SMDeviceCentral : public SMDevice {
public:
    SMDeviceCentral(BLE &ble, events::EventQueue &event_queue,
                    BLEProtocol::AddressBytes_t &peer_address)
        : SMDevice(ble, event_queue, peer_address)
    {
    }

    virtual void start()
    {
        ble::ScanParameters params;
        ble_error_t error = _ble.gap().setScanParameters(params);

        if (error) {
            print_error(error, "Error in Gap::startScan %d\r\n");
            return;
        }

        /* start scanning, results will be handled by onAdvertisingReport */
        error = _ble.gap().startScan();

        if (error) {
            print_error(error, "Error in Gap::startScan %d\r\n");
            return;
        }

        printf("Please advertise\r\n");

        printf("Scanning for: ");
        print_address(_peer_address);
    }

private:
    /* Gap::EventHandler */

    /** Look at scan payload to find a peer device and connect to it */
    virtual void onAdvertisingReport(const ble::AdvertisingReportEvent &event)
    {
        /* don't bother with analysing scan result if we're already connecting
         */
        if (_is_connecting) {
            return;
        }

        /* parse the advertising payload, looking for a discoverable device */
        if (memcmp(event.getPeerAddress().data(), _peer_address,
                   sizeof(_peer_address)) == 0) {
            ble_error_t error = _ble.gap().stopScan();

            if (error) {
                print_error(error, "Error caused by Gap::stopScan");
                return;
            }

            ble::ConnectionParameters connection_params(
                ble::phy_t::LE_1M, ble::scan_interval_t(50),
                ble::scan_window_t(50), ble::conn_interval_t(50),
                ble::conn_interval_t(100), ble::slave_latency_t(0),
                ble::supervision_timeout_t(100));
            connection_params.setOwnAddressType(
                ble::own_address_type_t::RANDOM);

            error =
                _ble.gap().connect(event.getPeerAddressType(),
                                   event.getPeerAddress(), connection_params);

            if (error) {
                print_error(error, "Error caused by Gap::connect");
                return;
            }

            /* we may have already scan events waiting
             * to be processed so we need to remember
             * that we are already connecting and ignore them */
            _is_connecting = true;

            return;
        }
    }

    /** This is called by Gap to notify the application we connected,
     *  in our case it immediately request pairing */
    virtual void onConnectionComplete(const ble::ConnectionCompleteEvent &event)
    {
        if (event.getStatus() == BLE_ERROR_NONE) {
            /* store the handle for future Security Manager requests */
            _handle = event.getConnectionHandle();

            printf("Connected\r\n");

            /* in this example the local device is the master so we request
             * pairing */
            ble_error_t error = _ble.securityManager().requestPairing(_handle);

            if (error) {
                printf("Error during SM::requestPairing %d\r\n", error);
                return;
            }

            /* upon pairing success the application will disconnect */
        }

        /* failed to connect - restart scan */
        ble_error_t error = _ble.gap().startScan();

        if (error) {
            print_error(error, "Error in Gap::startScan %d\r\n");
            return;
        }
    };
};

#endif