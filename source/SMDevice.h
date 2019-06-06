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

#ifndef SMDEVICE_H
#define SMDEVICE_H

#include <events/mbed_events.h>
#include "ble/BLE.h"
#include "SecurityManager.h"
#include "pretty_printer.h"

/* we have to specify the disconnect call because of ambiguous overloads */
typedef ble_error_t (Gap::*disconnect_call_t)(
    ble::connection_handle_t, ble::local_disconnection_reason_t);
const static disconnect_call_t disconnect_call = &Gap::disconnect;

/** Base class for both peripheral and central. The same class that provides
 *  the logic for the application also implements the
 * SecurityManagerEventHandler which is the interface used by the Security
 * Manager to communicate events back to the applications. You can provide
 * overrides for a selection of events your application is interested in.
 */
class SMDevice : private mbed::NonCopyable<SMDevice>,
                 public SecurityManager::EventHandler,
                 public ble::Gap::EventHandler {
public:
    SMDevice(BLE &ble, events::EventQueue &event_queue,
             BLEProtocol::AddressBytes_t &peer_address)
        : _led1(LED2, 0), _ble(ble), _event_queue(event_queue),
          _peer_address(peer_address), _handle(0), _is_connecting(false){};

    virtual ~SMDevice()
    {
        if (_ble.hasInitialized()) {
            _ble.shutdown();
        }
    };

    /** Start BLE interface initialisation */
    void run()
    {
        ble_error_t error;

        /* to show we're running we'll blink every 500ms */
        _event_queue.call_every(500, this, &SMDevice::blink);

        if (_ble.hasInitialized()) {
            printf("Ble instance already initialised.\r\n");
            return;
        }

        /* this will inform us off all events so we can schedule their handling
         * using our event queue */
        _ble.onEventsToProcess(
            makeFunctionPointer(this, &SMDevice::schedule_ble_events));

        /* handle gap events */
        _ble.gap().setEventHandler(this);

        error = _ble.init(this, &SMDevice::on_init_complete);

        if (error) {
            printf("Error returned by BLE::init.\r\n");
            return;
        }

        /* this will not return until shutdown */
        _event_queue.dispatch_forever();
    };

private:
    /** Override to start chosen activity when initialisation completes */
    virtual void start() = 0;

    /** This is called when BLE interface is initialised and starts the
     * demonstration */
    void on_init_complete(BLE::InitializationCompleteCallbackContext *event)
    {
        ble_error_t error;

        if (event->error) {
            printf("Error during the initialisation\r\n");
            return;
        }

        /* This path will be used to store bonding information but will fallback
         * to storing in memory if file access fails (for example due to lack of
         * a filesystem) */
        const char *db_path = "/fs/bt_sec_db";
        /* If the security manager is required this needs to be called before
         * any calls to the Security manager happen. */
        error = _ble.securityManager().init(
            true, false, SecurityManager::IO_CAPS_NONE, NULL, false, db_path);

        if (error) {
            printf("Error during init %d\r\n", error);
            return;
        }

        error = _ble.securityManager().preserveBondingStateOnReset(true);

        if (error) {
            printf("Error during preserveBondingStateOnReset %d\r\n", error);
        }

#if MBED_CONF_APP_FILESYSTEM_SUPPORT
        /* Enable privacy so we can find the keys */
        error = _ble.gap().enablePrivacy(true);

        if (error) {
            printf("Error enabling privacy\r\n");
        }

        Gap::PeripheralPrivacyConfiguration_t configuration_p = {
            /* use_non_resolvable_random_address */ false,
            Gap::PeripheralPrivacyConfiguration_t::REJECT_NON_RESOLVED_ADDRESS};
        _ble.gap().setPeripheralPrivacyConfiguration(&configuration_p);

        Gap::CentralPrivacyConfiguration_t configuration_c = {
            /* use_non_resolvable_random_address */ false,
            Gap::CentralPrivacyConfiguration_t::RESOLVE_AND_FORWARD};
        _ble.gap().setCentralPrivacyConfiguration(&configuration_c);

        /* this demo switches between being master and slave */
        _ble.securityManager().setHintFutureRoleReversal(true);
#endif

        /* Tell the security manager to use methods in this class to inform us
         * of any events. Class needs to implement SecurityManagerEventHandler.
         */
        _ble.securityManager().setSecurityManagerEventHandler(this);

        /* gap events also handled by this class */
        _ble.gap().setEventHandler(this);

        /* print device address */
        Gap::AddressType_t addr_type;
        Gap::Address_t addr;
        _ble.gap().getAddress(&addr_type, addr);
        print_address(addr);

        /* start test in 500 ms */
        _event_queue.call_in(500, this, &SMDevice::start);
    };

    /** Schedule processing of events from the BLE in the event queue. */
    void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context)
    {
        _event_queue.call(mbed::callback(&context->ble, &BLE::processEvents));
    };

    /** Blink LED to show we're running */
    void blink(void)
    {
        _led1 = !_led1;
    };

private:
    /* Event handler */

    /** Respond to a pairing request. This will be called by the stack
     * when a pairing request arrives and expects the application to
     * call acceptPairingRequest or cancelPairingRequest */
    virtual void pairingRequest(ble::connection_handle_t connectionHandle)
    {
        printf("Pairing requested - authorising\r\n");
        _ble.securityManager().acceptPairingRequest(connectionHandle);
    }

    /** Inform the application of a successful pairing. Terminate the
     * demonstration. */
    virtual void
    pairingResult(ble::connection_handle_t connectionHandle,
                  SecurityManager::SecurityCompletionStatus_t result)
    {
        if (result == SecurityManager::SEC_STATUS_SUCCESS) {
            printf("Pairing successful\r\n");
        } else {
            printf("Pairing failed\r\n");
        }
    }

    /** Inform the application of change in encryption status. This will be
     * communicated through the serial port */
    virtual void linkEncryptionResult(ble::connection_handle_t connectionHandle,
                                      ble::link_encryption_t result)
    {
        if (result == ble::link_encryption_t::ENCRYPTED) {
            printf("Link ENCRYPTED\r\n");
        } else if (result == ble::link_encryption_t::ENCRYPTED_WITH_MITM) {
            printf("Link ENCRYPTED_WITH_MITM\r\n");
        } else if (result == ble::link_encryption_t::NOT_ENCRYPTED) {
            printf("Link NOT_ENCRYPTED\r\n");
        }

        /* disconnect in 2 s */
        _event_queue.call_in(
            2000, &_ble.gap(), disconnect_call, _handle,
            ble::local_disconnection_reason_t(
                ble::local_disconnection_reason_t::USER_TERMINATION));
    }

    /** This is called by Gap to notify the application we disconnected,
     *  in our case it ends the demonstration. */
    virtual void
    onDisconnectionComplete(const ble::DisconnectionCompleteEvent &)
    {
        printf("Diconnected\r\n");
        _event_queue.break_dispatch();
    };

    virtual void onAdvertisingEnd(const ble::AdvertisingEndEvent &)
    {
        printf("Advertising timed out - aborting\r\n");
        _event_queue.break_dispatch();
    }

    virtual void onScanTimeout(const ble::ScanTimeoutEvent &)
    {
        printf("Scan timed out - aborting\r\n");
        _event_queue.break_dispatch();
    }

private:
    DigitalOut _led1;

protected:
    BLE &_ble;
    events::EventQueue &_event_queue;
    BLEProtocol::AddressBytes_t &_peer_address;
    ble::connection_handle_t _handle;
    bool _is_connecting;
};

#endif