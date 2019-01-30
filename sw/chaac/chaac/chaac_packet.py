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

    def decode(self, packet):
        return self.named_tuple._make(struct.unpack_from(self.struct_str, packet, 0))

    def encode(self, args):
        return struct.pack(self.struct_str, *args)

    def round(self, namedtuple):        
        packet_dict = packet._asdict()
        for key,val in packet_dict.items():
            if isinstance(val, float):
                packet_dict[key] = round(val, 3)
        return self.named_tuple._make(**packet_dict)
