#!/usr/bin/env python3
"""
Simple SPARTN message parser
Parses SPARTN binary format and displays message statistics
"""

import sys
import struct

def parse_spartn_message(data, offset):
    """Parse a single SPARTN message starting at offset"""
    if offset + 4 > len(data):
        return None, 0
    
    # SPARTN frame starts with 0x73 (preamble)
    if data[offset] != 0x73:
        return None, 0
    
    # Message type: bits 1-7 of byte 1
    # Message subtype: bits 0 of byte 1 + bits 5-7 of byte 2 (4 bits total)
    # Payload length: bits 0-4 of byte 2 + bits 3-7 of byte 3 (10 bits total)
    
    byte1 = data[offset + 1]
    byte2 = data[offset + 2]
    byte3 = data[offset + 3]
    
    # Combine into 24-bit value for easier bit extraction
    bits = (byte1 << 16) | (byte2 << 8) | byte3
    
    # TF002: Message type (7 bits, starting at bit 0)
    message_type = (bits >> 17) & 0x7F
    
    # TF003: Payload length (10 bits, starting at bit 7)
    payload_length = (bits >> 7) & 0x3FF
    
    # TF007: Message subtype (4 bits, after TF006 frame CRC)
    # Need to read byte 4 for subtype
    if offset + 4 > len(data):
        return None, 0
    byte4 = data[offset + 4]
    message_subtype = byte4 >> 4
    
    # Total message length: 1 (preamble) + 3 (header) + payload + 2 (CRC)
    total_length = 1 + 3 + payload_length + 2
    
    if offset + total_length > len(data):
        return None, 0
    
    return {
        'type': message_type,
        'subtype': message_subtype,
        'payload_length': payload_length,
        'total_length': total_length,
        'offset': offset
    }, total_length

def get_message_name(msg_type, subtype):
    """Get human-readable message name"""
    type_names = {
        0: 'OCB',
        1: 'HPAC',
        2: 'GAD',
    }
    
    subtype_names = {
        0: 'GPS',
        1: 'GLONASS',
        2: 'Galileo',
        3: 'BeiDou',
        4: 'QZSS',
    }
    
    type_name = type_names.get(msg_type, f'Type{msg_type}')
    subtype_name = subtype_names.get(subtype, f'Sub{subtype}')
    
    return f"{type_name}-{subtype_name}"

def parse_spartn_file(filename):
    """Parse SPARTN file and return statistics"""
    with open(filename, 'rb') as f:
        data = f.read()
    
    messages = []
    offset = 0
    
    while offset < len(data):
        msg, length = parse_spartn_message(data, offset)
        
        if msg is None:
            # Try to find next preamble
            offset += 1
            continue
        
        messages.append(msg)
        offset += length
    
    return messages

def print_statistics(messages):
    """Print message statistics"""
    if not messages:
        print("No messages found")
        return
    
    # Count by type/subtype
    counts = {}
    for msg in messages:
        key = (msg['type'], msg['subtype'])
        counts[key] = counts.get(key, 0) + 1
    
    print(f"Total messages: {len(messages)}")
    print(f"\nMessage breakdown:")
    
    # Sort by type, then subtype
    for (msg_type, subtype), count in sorted(counts.items()):
        name = get_message_name(msg_type, subtype)
        print(f"  {name:20s} (type={msg_type} sub={subtype}): {count:6d}")
    
    # Show first few messages
    print(f"\nFirst 10 messages:")
    for i, msg in enumerate(messages[:10]):
        name = get_message_name(msg['type'], msg['subtype'])
        print(f"  {i+1:3d}. {name:20s} offset={msg['offset']:8d} length={msg['total_length']:4d}")

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <spartn_file>")
        sys.exit(1)
    
    filename = sys.argv[1]
    
    try:
        messages = parse_spartn_file(filename)
        print_statistics(messages)
    except FileNotFoundError:
        print(f"Error: File '{filename}' not found")
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)
