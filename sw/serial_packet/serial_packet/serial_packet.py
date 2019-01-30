import struct
from serial_packet.crc import crc16

HEADER_LEN = 4
CRC_LEN = 2
START_BYTES = 0xaa55

# Encode serial packet with CRC
def encode_packet(data):
    buff = bytearray(len(data) + HEADER_LEN + CRC_LEN)
    struct.pack_into("HH", buff, 0, START_BYTES, len(data))
    struct.pack_into("{}s".format(len(data)), buff, HEADER_LEN, data)
    crc = crc16(buff[0:-CRC_LEN])
    struct.pack_into("H", buff, HEADER_LEN + len(data), crc)

    return buff


def check_crc(packet):
    packet_crc = struct.unpack("H", packet[-CRC_LEN:])[0]
    computed_crc = crc16(packet[0:-CRC_LEN])
    return packet_crc == computed_crc


def decode_packet(buff, callback):

    # Look for start bytes in packet
    for offset in range(len(buff)):

        # Need at least 4 bytes for the header and 2 for crc
        if (len(buff) - offset) < (HEADER_LEN + CRC_LEN):
            return False

        start, dlen = struct.unpack_from("HH", buff, offset)
        if start == START_BYTES:

            # Make sure we have enough bytes for the packet
            if (len(buff) - offset) < (HEADER_LEN + dlen + CRC_LEN):
                return False
            else:
                if (
                    check_crc(buff[offset : (offset + dlen + HEADER_LEN + CRC_LEN)])
                    is True
                ):
                    packet = buff[offset : (offset + dlen + HEADER_LEN + CRC_LEN)]
                    callback(packet[HEADER_LEN:-CRC_LEN])

                    # Remove all data before the packet
                    del buff[: (offset + dlen + HEADER_LEN + CRC_LEN)]
                    return True
                else:
                    # CRC Error, remove the header and keep processing
                    del buff[: (offset + 2)]
                    return True
