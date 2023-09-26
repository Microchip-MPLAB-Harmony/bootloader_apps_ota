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
def uint16(v):
    return [(v >> 0) & 0xff, (v >> 8) & 0xff]


#------------------------------------------------------------------------------
def calc_checksum(data):

    for d in data:
            # Calculate the sum of all bytes in the file
            total = sum(data)
    checksum = 0xFFFF - (total&0x0000FFFF) + 1
    return checksum

def main():
    header_version = 0x02           # 1 byte (must be 0x02)
    ota_image_dec = 0x00            # 1 byte (no use for HOST OTA)
    checksum = 0x0000               # 2 byte (MBD app req),Calculate dynamically
    flash_image_ID = 0x00000000     # 4 byte (Optional: Flash image ID ,For MBD app to display)
    flash_image_rev = 0x00000000    # 4 byte (optional: Flash image revision_number, For MBD app to display)
    ota_file_type = 0x02            # 1 byte (must be 0x02)
    reserved = 0x00                 # 1 byte
    crc16 = 0x0000                  # 2 byte, optional

    parser = optparse.OptionParser(usage = 'usage: %prog [options]')
    parser.add_option('-v', '--verbose', dest='verbose', help='enable verbose output', default=False, action='store_true')
    parser.add_option('-f', '--binary', dest='binary', help='binary input file', metavar='BINARY_FILE')
    parser.add_option('-o', '--outputBinary', dest='outputBinary', help='binary output file', default='RNBD_image.bin', metavar='OUTPUT_BINARY_FILE')
    parser.add_option('-r', '--flash_image_rev', dest='flash_image_rev', help='Flash Image revision', default='0', metavar='FLASH_IMAGE_REV')
    parser.add_option('-i', '--flash_image_ID', dest='flash_image_ID', help='Flash Image ID', default='0', metavar='FLASH_IMAGE_ID')
    
    (options, args) = parser.parse_args()


    if options.binary is None:
        error('Binary input file is required (use -f option)')

    flash_image_ID = int(options.flash_image_ID,0)
    flash_image_rev = int(options.flash_image_rev,0)
    output_binary = options.outputBinary
    input_binary = options.binary
    
    data = []
    data += [(x) for x in open(options.binary, 'rb').read()]

    checksum = calc_checksum(data)
    
    header_info = [header_version] + [ota_image_dec] + uint16(checksum) + uint32(flash_image_ID) + uint32(flash_image_rev) + [ota_file_type] + [reserved] + uint16(crc16)   
    header = bytes(header_info) + b"MCHP" + bytes(12)

    #print ("Header length : " + str(len(header)))
    #for byte in header:
    #    print(hex(byte),'',end='')


    #header - need to be appended to the start of binary file
    with open(output_binary,'wb') as f:
        f.write(header + bytes(data))

    print(output_binary + " is generated successfully")
#------------------------------------------------------------------------------

main()
