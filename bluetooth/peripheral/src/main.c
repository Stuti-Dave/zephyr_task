/*
* Copyright (c) 2024 Nordic Semiconductor ASA
* SPDX-License-Identifier: Apache-2.0
*
* Description: Custom GATT Service with 2 Characteristics
* 1. Characteristic 1: Read and Notify
* 2. Characteristic 2: Write Only
* Notifications are sent every 60 seconds with incrementing value
* Write Characteristic logs the written value and the data length, offset and data received
* The peripheral advertises and waits for a central to connect. Once connected, it starts sending notifications
* on Characteristic 1 every 60 seconds for a total of 10 notifications.
* If no central connects within 60 seconds, it logs that no device is connected and exits
*
* Author: Stuti Dave
* Date: 8th Sept 2025
* Last Modified: 11th Sept 2025
* */

 /** @file
 *  @brief Custom GATT Service sample
 */


//////////////////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////////////////
#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/sys/check.h>
#include <zephyr/sys/atomic.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/logging/log.h>

//////////////////////////////////////////////////////////////////////////////////
// Logging
//////////////////////////////////////////////////////////////////////////////////

#define LOG_LEVEL CONFIG_BT_HRS_LOG_LEVEL
LOG_MODULE_REGISTER(main);

//////////////////////////////////////////////////////////////////////////////////
// Private defines and macros
//////////////////////////////////////////////////////////////////////////////////

#define MAX_LENGTH 32
#define MAX_NOTIFY_COUNT 10

/* 128-bit UUID for custom service */
#define BT_UUID_CUSTOM_SERVICE_VAL BT_UUID_128_ENCODE(0x8a5c1d32, 0x4c7e, 0x4d8b, 0xb0c4, 0x3f9f79dbd6f1)
#define BT_UUID_CUSTOM_SERVICE BT_UUID_DECLARE_128(BT_UUID_CUSTOM_SERVICE_VAL)

/* Characteristic UUIDs */
#define BT_UUID_CUSTOM_CHAR1_VAL BT_UUID_128_ENCODE(0x3f8e27a1, 0xb5b2, 0x46ea, 0x8d8a, 0x7f3a41a6c9e3)
#define BT_UUID_CUSTOM_CHAR1 BT_UUID_DECLARE_128(BT_UUID_CUSTOM_CHAR1_VAL)

#define BT_UUID_CUSTOM_CHAR2_VAL BT_UUID_128_ENCODE(0xf24a6e4c, 0x92cb, 0x40d0, 0xb7f6, 0xcc5d48cb5b7a)
#define BT_UUID_CUSTOM_CHAR2 BT_UUID_DECLARE_128(BT_UUID_CUSTOM_CHAR2_VAL)

//////////////////////////////////////////////////////////////////////////////////
// Structures and Global Variables
/////////////////////////////////////////////////////////////////////////////////

/* Callback structure for the service */
struct bt_cgs_cb {
        void (*ntf_changed)(bool enabled);
        int (*ctrl_point_write)(uint8_t request);
        sys_snode_t _node;
};

/* Global variables for characteristic values and state */ 
static uint8_t cgs_blsc;
//static sys_slist_t cgs_cbs = SYS_SLIST_STATIC_INIT(&cgs_cbs);

/* Characteristic values */
static char char1_value[32] = "Hello from peripheral";
static char char2_value[32] = {0};

//////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
//////////////////////////////////////////////////////////////////////////////////

/* Load settings for notifications */
void settings_load(void);

//////////////////////////////////////////////////////////////////////////////////
// Helper Functions and GATT Characteristics & Service Declaration
//////////////////////////////////////////////////////////////////////////////////

/* GATT characteristics read/write functions */
static ssize_t read_char1(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
    const char *value = (const char *)attr->user_data;

    return bt_gatt_attr_read(conn, attr, buf, len, offset, value, strlen(value));
}

static ssize_t write_char2(struct bt_conn *conn, const struct bt_gatt_attr *attr, 
                            const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
    LOG_INF("Written to Char2: len=%d, offset=%d", len, offset);
    LOG_INF("Data: %.*s", len, (char *)buf);

    char *value = (char *)attr->user_data;
    if (offset + len > sizeof(char2_value)) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }
    memcpy(value + offset, buf, len);
    value[offset + len] = '\0'; // Null-terminate the string
    LOG_INF("Char2 value updated to: %s", value);

    return len;
}

/* Custom Service Declaration */
BT_GATT_SERVICE_DEFINE(custom_svc,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_CUSTOM_SERVICE),
    BT_GATT_CHARACTERISTIC(BT_UUID_CUSTOM_CHAR1,
                           BT_GATT_CHRC_READ  | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ , 
                           read_char1, NULL, char1_value),
    BT_GATT_CCC(NULL, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
    BT_GATT_CHARACTERISTIC(BT_UUID_CUSTOM_CHAR2,
                           BT_GATT_CHRC_WRITE,
                           BT_GATT_PERM_WRITE,
                           NULL, write_char2, NULL),
);

///////////////////////////////////////////////////////////////////////////////
// Bluetooth Initialization
///////////////////////////////////////////////////////////////////////////////

/* GATT Service Initialization */
static int cgs_init(void)
{
	cgs_blsc = 0x01;

	return 0;
}

SYS_INIT(cgs_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

///////////////////////////////////////////////////////////////////////////////
// Advertisement
///////////////////////////////////////////////////////////////////////////////

/* Start advertising */
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_LIMITED | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME,
            sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};
static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

/* Bluetooth enable and initiate advertising */
static void bt_ready(void)
{
	int err;

	LOG_ERR("Bluetooth initialized\n");

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)\n", err);
		return;
	}

	LOG_INF("Advertising successfully started\n");
}

//////////////////////////////////////////////////////////////////////////////////
// Connection Management and Notification
//////////////////////////////////////////////////////////////////////////////////

/* Connection state management */
enum {
        STATE_CONNECTED,
        STATE_DISCONNECTED,
        STATE_BITS,
};

static ATOMIC_DEFINE(state, STATE_BITS);

/* Connection callbacks */
static void connected(struct bt_conn *conn, uint8_t err)
{
        if (err) {
                LOG_ERR("Connection failed, err 0x%02x %s\n", err, bt_hci_err_to_str(err));
        } else {
                LOG_INF("Connected\n");
                atomic_clear_bit(state, STATE_DISCONNECTED);
                atomic_set_bit(state, STATE_CONNECTED);
        }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
        LOG_ERR("Disconnected, reason 0x%02x %s\n", reason, bt_hci_err_to_str(reason));

        atomic_clear_bit(state, STATE_CONNECTED);
        atomic_set_bit(state, STATE_DISCONNECTED);
}

/* Connection callback structure definition */
BT_CONN_CB_DEFINE(conn_callbacks) = {
        .connected = connected,
        .disconnected = disconnected,
};

/* Notification function */
int bt_cgs_notify(uint8_t value)
{
    if (!atomic_test_bit(state, STATE_CONNECTED)) {
        return -ENOTCONN;
    }

    uint8_t cgm[2];
    cgm[0] = 0x06; // uint8, sensor contact
    cgm[1] = value;
    
    int ret = bt_gatt_notify(NULL, &custom_svc.attrs[1], cgm, sizeof(cgm));
    
    if (ret == 0) {
        LOG_INF("Sending notification: %02x %02x", cgm[0], cgm[1]);
    } else {
        LOG_ERR("Notification failed (err %d)", ret);
        return ret;
    }
    
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////
// Main Application
//////////////////////////////////////////////////////////////////////////////////

int main(void)
{
    int err = bt_enable(NULL);
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)", err);
        return -1;
    }

    bt_ready();
    LOG_INF("Bluetooth initialized");

    k_sleep(K_SECONDS(60)); // Wait for connections
    if (atomic_test_bit(state, STATE_CONNECTED)) {

        LOG_INF("Device already connected starting notifications");
        for(uint8_t notify_count = 0; notify_count < MAX_NOTIFY_COUNT ; notify_count++) {
            err = bt_cgs_notify(notify_count);
            k_sleep(K_SECONDS(60));
            if (err) {
                LOG_ERR("Notification %d failed (err %d)", notify_count, err);
            }
        }
        LOG_INF("Completed %d notifications", MAX_NOTIFY_COUNT);
    } else {
            LOG_INF("Device not connected, cannot start notifications");
    }
    
    return 0;
}