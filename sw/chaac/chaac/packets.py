from chaac.chaac_packet import ChaacPacket

# Note, if you update this, make sure to update the corresponding enum
# packet_type_t in fw/chaac/apps/controller/src/chaac_packet.h
PACKET_TYPE_DATA = 1
PACKET_TYPE_GPS = 2
PACKET_TYPE_BOOT = 3
PACKET_TYPE_CLEAR_RAIN = 4


PacketHeader = ChaacPacket("PacketHeader", [("uid", "I"), ("packet_type", "B")])

WeatherPacket = ChaacPacket(
    "WeatherPacket",
    [
        ("uid", "I"),
        ("packet_type", "B"),
        ("wind_speed", "f"),
        ("wind_dir", "f"),
        ("rain", "f"),
        ("temperature", "f"),
        ("humidity", "f"),
        ("temperature_in", "f"),
        ("pressure", "f"),
        ("light", "f"),
        ("battery", "f"),
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
