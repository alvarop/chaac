from chaac.chaac_packet import ChaacPacket

# Note, if you update this, make sure to update the corresponding enum
# packet_type_t in fw/chaac/apps/controller/src/chaac_packet.h
PACKET_TYPE_DATA = 1
PACKET_TYPE_GPS = 2
PACKET_TYPE_BOOT = 3
PACKET_TYPE_CLEAR_RAIN = 4


PacketHeader = ChaacPacket("PacketHeader", [("uid", "I"), ("packet_type", "B")])

ResetPacket = ChaacPacket(
    "ResetPacket",
    [
        ("uid", "I"),
        ("packet_type", "B"),
    ],
)

MemfaultPacket = ChaacPacket(
    "MemfaultPacket",
    [
        ("uid", "I"),
        ("packet_type", "B"),
        ("len", "H"),
    ],
)

WeatherPacketV1P0 = ChaacPacket(
    "WeatherPacketV1P0",
    [
        ("uid", "I"),
        ("packet_type", "B"),
        ("sample", "L"),
        ("wind_dir", "B"),
        ("rain", "B"),
        ("rsvd", "H"),
        ("wind_speed", "H"),
        ("temperature", "h"),
        ("humidity", "H"),
        ("pressure", "h"),
        ("battery", "H"),
        ("solar_panel", "H"),
    ],
)

WeatherPacketV1P1 = ChaacPacket(
    "WeatherPacketV1P1",
    [
        ("uid", "I"),
        ("packet_type", "B"),
        ("sample", "L"),
        ("wind_dir_deg", "H"),
        ("rain", "B"),
        ("wind_speed", "H"),
        ("gust_speed", "H"),
        ("temperature", "h"),
        ("alt_temperature", "h"),
        ("humidity", "H"),
        ("pressure", "h"),
        ("battery", "H"),
        ("solar_panel", "H"),
    ],
)

ButtonPacketV1P0 = ChaacPacket(
    "ButtonPacketV1P0",
    [
        ("uid", "I"),
        ("packet_type", "B"),
        ("ch1", "B"),
        ("ch2", "B"),
        ("battery", "H"),
    ],
)

LoraRxInfo = ChaacPacket(
    "LoraRxInfo",
    [
        ("rssi", "h"),
        ("snr", "b"),
        ("rsvd", "B")
    ],
)

RangeTestPacketV1P0 = ChaacPacket(
    "RangeTestPacketV1P0",
    [
        ("uid", "I"),
        ("packet_type", "B"),
        ("sample", "L"),
        ("tx_pwr", "b"),
    ],
)

# All packet types go here
PacketTypes = {
    1: ResetPacket,
    2: MemfaultPacket,
    10: WeatherPacketV1P0,
    11: WeatherPacketV1P1,
    30: ButtonPacketV1P0,
    0xF0: RangeTestPacketV1P0,
}

# Valid weather packets go here
WeatherPacketTypes = {
    10: WeatherPacketV1P0,
    11: WeatherPacketV1P1,
}

