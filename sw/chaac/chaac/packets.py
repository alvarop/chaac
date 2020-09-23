from chaac.chaac_packet import ChaacPacket

# Note, if you update this, make sure to update the corresponding enum
# packet_type_t in fw/chaac/apps/controller/src/chaac_packet.h
PACKET_TYPE_DATA = 1
PACKET_TYPE_GPS = 2
PACKET_TYPE_BOOT = 3
PACKET_TYPE_CLEAR_RAIN = 4


PacketHeader = ChaacPacket("PacketHeader", [("uid", "I"), ("packet_type", "B")])

# LEGACY chaac v1.0 WeatherPacket
WeatherPacket = ChaacPacket(
    "WeatherPacket",
    [
        ("uid", "I"),
        ("packet_type", "B"),
        ("sample", "I"),
        ("wind_speed", "f"),
        ("wind_dir", "f"),
        ("rain", "f"),
        ("temperature", "f"),
        ("humidity", "f"),
        ("temperature_in", "f"),
        ("pressure", "f"),
        ("light", "f"),
        ("battery", "f"),
        ("solar_panel", "f"),
    ],
)

GPSPacket = ChaacPacket(
    "GPSPacket",
    [
        ("uid", "I"),
        ("packet_type", "B"),
        ("lat_degrees", "i"),
        ("lat_minutes", "d"),
        ("lat_cardinal", "c"),
        ("lon_degrees", "i"),
        ("lon_minutes", "d"),
        ("lon_cardinal", "c"),
    ],
)

BootPacket = ChaacPacket(
    "BootPacket", [("uid", "I"), ("packet_type", "B"), ("flags", "B")]
)

BLEWeatherPacket = ChaacPacket(
    "BLEWeatherPacket",
    [
        ("uid", "L"),
        ("addr_pad", "H"),
        ("sample", "B"),
        ("wind_dir", "B"),
        ("rain", "B"),
        ("rsvd", "B"),
        ("wind_speed", "H"),
        ("temperature", "h"),
        ("humidity", "H"),
        ("pressure", "H"),
        ("battery", "H"),
        ("solar_panel", "H"),
        ("rssi", "b"),
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
        ("pressure", "H"),
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

PacketTypes = {
    10: WeatherPacketV1P0,
    30: ButtonPacketV1P0,
    0xF0: RangeTestPacketV1P0,
}

