
from chaac.chaac_packet import ChaacPacket

PACKET_TYPE_DATA = 1
PACKET_TYPE_GPS = 2
PACKET_TYPE_CMD = 3


PacketHeader = ChaacPacket(
    "PacketHeader",
    [
        ("uid", "I"),
        ("packet_type", "B")
    ]
)

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

CMDPacket = ChaacPacket(
    "CMDPacket",
    [
        ("uid", "I"),
        ("packet_type", "B"),
        ("cmd", "B"),
    ],
)

