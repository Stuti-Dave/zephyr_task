

#include <zephyr/device.h>
#include <zephyr/devicetree.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#define MAX_LENGTH 32

// Custom Service UUID (128-bit)
#define BT_UUID_CUSTOM_SERVICE_VAL 0xa84f1cba
static struct bt_uuid_32 custom_service_uuid = BT_UUID_INIT_32(BT_UUID_CUSTOM_SERVICE_VAL);

// Custom Characteristic UUIDs (16-bit)
#define BT_UUID_CUSTOM_CHAR1_VAL 0x7B3C
#define BT_UUID_CUSTOM_CHAR2_VAL 0xE2A1
static struct bt_uuid_16 custom_char1_uuid = BT_UUID_INIT_16(BT_UUID_CUSTOM_CHAR1_VAL);
static struct bt_uuid_16 custom_char2_uuid = BT_UUID_INIT_16(BT_UUID_CUSTOM_CHAR2_VAL);

// Characteristic values
static char char1_value[32] = "Hello from peripheral";
static char char2_value[32] = {0};

// Advertising data
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_CUSTOM_SERVICE_VAL)
};

enum {
        STATE_CONNECTED,
        STATE_DISCONNECTED,
        STATE_BITS,
};

static ATOMIC_DEFINE(state, STATE_BITS);

static void connected(struct bt_conn *conn, uint8_t err)
{
        if (err) {
                printk("Connection failed, err 0x%02x %s\n", err, bt_hci_err_to_str(err));
        } else {
                printk("Connected\n");
                atomic_clear_bit(state, STATE_DISCONNECTED);
                atomic_set_bit(state, STATE_CONNECTED);
        }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
        printk("Disconnected, reason 0x%02x %s\n", reason, bt_hci_err_to_str(reason));

        atomic_clear_bit(state, STATE_CONNECTED);
        atomic_set_bit(state, STATE_DISCONNECTED);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
        .connected = connected,
        .disconnected = disconnected,
};

// GATT Characteristic Read/Write 
static ssize_t read_char1(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
    const char *value = (const char *)attr->user_data;
    return bt_gatt_attr_read(conn, attr, buf, len, offset, value, strlen(value));
}

static ssize_t write_char2(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
    char *value = (char *)attr->user_data;

    if (offset + len > MAX_LENGTH - 1) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    memcpy(value + offset, buf, len);
    value[offset + len] = '\0';

    printk("Characteristic 2 written: %s\n", value);

    return len;
}

BT_GATT_SERVICE_DEFINE(custom_svc,
    BT_GATT_PRIMARY_SERVICE(&custom_service_uuid),
    BT_GATT_CHARACTERISTIC(&custom_char1_uuid.uuid,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ,
                           read_char1, NULL, char1_value),
    BT_GATT_CCC(NULL, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
    BT_GATT_CHARACTERISTIC(&custom_char2_uuid.uuid,
                           BT_GATT_CHRC_WRITE,
                           BT_GATT_PERM_WRITE,
                           NULL, write_char2, char2_value),
);

static void bt_ready(int err)
{
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }

    printk("Bluetooth initialized\n");

    err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
        printk("Advertising failed to start (err %d)\n", err);
        return;
    }

    printk("Advertising successfully started\n");
}

void main(void)
{
    int err;

    printk("Starting Bluetooth Peripheral GATT example\n");

    err = bt_enable(bt_ready);
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }

    while (1) {
        if (atomic_test_and_clear_bit(state, STATE_DISCONNECTED)) {
            printk("Disconnected event\n");
        }

        if (atomic_test_and_clear_bit(state, STATE_CONNECTED)) {
            printk("Connected event\n");
        }

        // Simulate sending notifications every 10 seconds
        k_sleep(K_SECONDS(10));
    }
}