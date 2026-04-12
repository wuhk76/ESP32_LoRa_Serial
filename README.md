# ESP32_LoRa_Serial
Simple Python-based API for Heltec WiFi LoRa 32 (V3)

## Overview

This repository provides a lightweight `Esplora` class that wraps serial communication and implements a basic framed data protocol using:

- **STX (Start of Text)**: `0x02`
- **ETX (End of Transmission request)**: `0x03`
- **4-byte big-endian length field**
- **Raw payload data**

It is designed to work with embedded firmware (such as ESP32/LoRa devices) that follows the same protocol.

---

## Features

- Simple API for sending and receiving binary data
- Automatic packet framing (STX + length + payload)
- Request-based receiving (ETX trigger)
- Minimal dependencies

---

## Installation

Install the required dependency:

```bash
pip install pyserial
```

---

Open Arduino IDE 2.3x or later, and upload LoRa_PTP_Firmware.ino to your ESP32 device.

---

## Usage

```python
from esplora import Esplora

# Initialize connection
esp = Esplora('/dev/ttyUSB0', 115200)

# Send data
esp.send('Hello, world!')

# Receive response from second ESP32 serial link;
response = esp.receive()
print(response)
```

---

## Protocol Specification

### Sending Data

When calling:

```python
send(data, encoding = '')
```

The following packet is transmitted:

```
[0x02][size (4 bytes, big endian)][data]
```

- `0x02`: Start of Text (STX)
- `size`: Length of data payload
- `data`: Raw bytes

---

### Receiving Data

When calling:

```python
receive(decoding = '')
```

The following occurs:

1. Sends:
   ```
   [0x03]
   ```
   (ETX — request for data)

2. Reads:
   ```
   [size (4 bytes)][data]
   ```

- `size`: 4-byte big-endian integer
- `data`: Payload of that size

---

## Class Reference

### `Esplora(port, baud, timeout = 1)`

Initializes a serial connection.

- `port`: Serial port (e.g., `/dev/ttyUSB0`, `COM3`)
- `baud`: Baud rate (e.g. `115200`)
- `timeout`: Read timeout in seconds

---

### `send(data: bytes)`

Sends a packet to the device.

- `data`: Byte string to send

---

### `receive() -> bytes`

Requests and reads a packet from the device.

- Returns: Received byte payload

---

## Example Packet Flow

```
PC → ESP:
  0x02 00 00 00 05 48 65 6C 6C 6F   ("Hello")

PC ← ESP (after ETX request):
  00 00 00 05 57 6F 72 6C 64        ("World")
```

---

## Notes

- This library assumes the device firmware implements the same protocol.
- No checksum or error correction is included — consider adding this for reliability.
- Blocking reads depend on the configured timeout.

---

## License

MIT License (or specify your preferred license)

---

## Future Improvements

- Add checksum / CRC support
- Mutithreading in the receive function
- Packet validation and error handling
- Higher-level message abstractions
