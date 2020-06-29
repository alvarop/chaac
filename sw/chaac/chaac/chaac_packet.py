import struct
import collections


class ChaacPacket:
    def __init__(self, name, packet_struct):
        self.packet_struct = packet_struct
        self.name = name
        self.named_tuple = collections.namedtuple(
            name, list(list(zip(*packet_struct))[0])
        )
        self.struct_str = "<" + "".join(list(list(zip(*packet_struct))[1]))

    def from_dict(self, packet_dict):
        items = []
        for item in self.packet_struct:
            items.append(packet_dict[item[0]])

        return self.named_tuple._make(items)

    def decode(self, packet, offset=0):
        return self.named_tuple._make(struct.unpack_from(self.struct_str, packet, offset))

    def encode(self, args):
        return struct.pack(self.struct_str, *args)

    def round(self, namedtuple):
        # Round all floating point items to 3 decimal places
        # TODO - have precision and other options set in packet_struct
        data_dict = namedtuple._asdict()
        for key, val in data_dict.items():
            if isinstance(val, float):
                data_dict[key] = round(val, 3)
        return self.named_tuple(**data_dict)

    def size(self):
        return struct.calcsize(self.struct_str)
