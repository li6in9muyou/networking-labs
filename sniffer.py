import binascii
import socket
import traceback


def port_number(bytes):
    """
    >>> port_number(b'\\x56\\xce')
    '22222'
    """
    return str(int.from_bytes(bytes, 'big'))


def dotted_decimal(four_bytes):
    """
    >>> dotted_decimal(b'\\xac\\x12\\x70\\x01')
    '172.18.112.1'
    """
    return '.'.join((map(str, four_bytes)))


def ascii_decode(bytes):
    """
    >>> ascii_decode(b"\\x32\\x30\\x32\\x33\\x2d\\x30\\x33\\x2d\\x30\\x33\\x20\\x32\\x33\\x3a\\x30\\x34\\x3a\\x35\\x34\\x0a")
    '2023-03-03 23:04:54\\n'
    >>> ascii_decode(b"\\x9f")
    b'9f'
    """
    try:
        return bytes.decode('ascii')
    except UnicodeDecodeError:
        return binascii.hexlify(bytes, ' ')


def header(title):
    """
    >>> header("Application")
    'Application++++++++++++++++++++++++++++++++++++++++'
    """
    return f'{title}{"+" * 40}'


def parse_packet_then_print(bb):
    data = bb[38:]
    print(header("Applications"))
    print('Data:', ascii_decode(data))

    print(header("Transportation"))
    src_port = port_number(bb[20:22])
    dst_port = port_number(bb[22:24])
    print(f'Src Port: {src_port}\nDest Port: {dst_port}')

    print(header("Network"))
    src_ip = dotted_decimal(bb[12:16])
    dst_ip = dotted_decimal(bb[16:20])
    print(f'Src IP: {src_ip}\nDest IP: {dst_ip}')

    print('\n\n\n')


if __name__ == '__main__':
    import sys
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.IPPROTO_IP)
        s.bind((sys.argv[1], 0))
        s.setsockopt(socket.IPPROTO_IP, socket.IP_HDRINCL, 1)
        s.ioctl(socket.SIO_RCVALL, socket.RCVALL_ON)

        for _ in range(100):
            raw_bytes, from_ip = s.recvfrom(2048)
            parse_packet_then_print(raw_bytes)

        # disabled promiscuous mode
        s.ioctl(socket.SIO_RCVALL, socket.RCVALL_OFF)
    except Exception as e:
        traceback.print_exc()
    except IndexError as e:
        print(f"Usage: {sys.argv[0]} HOST\nBind raw socket to HOST")
    finally:
        input("press any key to continue...")
