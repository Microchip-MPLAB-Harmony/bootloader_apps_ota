#!/usr/bin/env python

"""*****************************************************************************
* Copyright (C) 2023 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*****************************************************************************"""

import struct
import os
import optparse

def uint32(v):
    return [(v >> 0) & 0xff, (v >> 8) & 0xff, (v >> 16) & 0xff, (v >> 24) & 0xff]

def crc32_tab_gen():
    res = []

    for i in range(256):
        value = i

        for j in range(8):
            if value & 1:
                value = (value >> 1) ^ 0xedb88320
            else:
                value = value >> 1

        res += [value]

    return res

#------------------------------------------------------------------------------
def crc32_calculate(tab, data):
    crc = 0xffffffff

    for d in data:
        crc = tab[(crc ^ d) & 0xff] ^ (crc >> 8)

    return crc

def main():
    header_version = 0              # 1 byte (0 - 255)
    signature_present = 1           # 1 byte (0 - 1)
    status = 4                      # 1 byte (0 - 4), 0 - Invalid, 1 - Downloaded, 2 - Unbooted, 3 - Disable, 4 - Valid
    image_storage = 1               # 1 byte (0 - 2), 1 - External Memory, 2 - Dual Bank
    image_type = 0                  # 1 byte (0 - 2), 0 - Active, 1 - Rollback, 2 - Factory
    program_address = 0x0           # 4 byte
    jump_address = 0x0              # 4 byte  # Active Bank address for application execution from bootloader
    imagesize = 0                   # 4 byte, Calculate dynamically
    load_address = 0x0              # 4 byte, 0 for Dual Bank Devices
    image_version = 0               # 1 byte
                                    # 8 byte (4 byte - CRC32, 4 byte- unused), Calculate dynamically
    signature_type = 1              # 1 byte, 1 for CRC32

    parser = optparse.OptionParser(usage = 'usage: %prog [options]')
    parser.add_option('-v', '--verbose', dest='verbose', help='enable verbose output', default=False, action='store_true')
    parser.add_option('-p', '--programAddr', dest='programAddr', help='Flash Application Program Address. Note - Inactive Bank address in case of Dual Bank Flash', metavar='PROGRAM_ADDR')
    parser.add_option('-j', '--jumpAddr', dest='jumpAddr', help='Flash Application Execution Address. Note - Active Bank address in case of Dual Bank Flash', metavar='JUMP_ADDR')
    parser.add_option('-l', '--loadAddr', dest='loadAddr', help='External Memory Program Address', default='0', metavar='LOAD_ADDR')
    parser.add_option('-f', '--binary', dest='binary', help='binary input file', metavar='BINARY_FILE')
    parser.add_option('-o', '--outputBinary', dest='outputBinary', help='binary output file', default='image.bin', metavar='OUTPUT_BINARY_FILE')
    parser.add_option('-r', '--headerVer', dest='headerVer', help='Header Version', default='0', metavar='HEADER_VERSION')
    parser.add_option('-i', '--imageVer', dest='imageVer', help='Image Version', default='0', metavar='IMAGE_VERSION')
    parser.add_option('-t', '--imageType', dest='imageType', help='Image Type', default='0', metavar='IMAGE_TYPE')
    parser.add_option('-s', '--imageStorage', dest='imageStorage', help='Image Storage', default='1', metavar='IMAGE_STORAGE')

    (options, args) = parser.parse_args()

    if options.programAddr is None:
        error('Flash Application Program Address is required (try -p option)')

    if options.jumpAddr is None:
        error('Flash Application Execution Address is required (use -j option)')

    if options.binary is None:
        error('Binary input file is required (use -f option)')

    program_address = int(options.programAddr, 0)
    jump_address = int(options.jumpAddr, 0)
    load_address = int(options.loadAddr, 0)
    header_version = int(options.headerVer, 0)
    image_version = int(options.imageVer, 0)
    image_type = int(options.imageType, 0)
    image_storage = int(options.imageStorage, 0)
    output_binary = options.outputBinary

    data = []
    data += [(x) for x in open(options.binary, 'rb').read()]

    while len(data) % 4 > 0:  # Image size must be aligned with 4-bytes for Hardware (e.g. DSU) based CRC32 calculation on the target
        data += [0xff]

    crc32_tab = crc32_tab_gen()
    crc32 = crc32_calculate(crc32_tab, data)
    crc_val = uint32(crc32) + uint32(0) # 8 byte (4 byte - CRC32, 4 byte- unused)

    imagesize = len(data)

    header_info = [header_version] + [signature_present] + [status] + [image_storage] + [image_type] + uint32(program_address) + uint32(jump_address) + uint32(imagesize) + uint32(load_address) + [image_version] + crc_val + [signature_type]
    header = "IMAGESTART0".encode() + bytes(header_info)

    #print ("Header length : " + str(len(header)))
    #for byte in header:
    #    print(hex(byte),'',end='')

    headerend = struct.pack('8s',str.encode("IMAGEEND"))

    #header - need to be appended to the start of binary file
    #headerend - need to be appended to the end of binary file
    with open(output_binary,'wb') as f:
        f.write(header + bytes(data)+ headerend)

    print(output_binary + " is generated successfully")
#------------------------------------------------------------------------------

main()
