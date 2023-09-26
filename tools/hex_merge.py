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
import os
import optparse

def main():

    parser = optparse.OptionParser(usage = 'usage: %prog [options]')
    parser.add_option('-v',  '--verbose', dest='verbose', help='enable verbose output', default=False, action='store_true')
    parser.add_option('-o', '--hexfile1', dest='hexfile1', help='HEX file 1', metavar='HEX_FILE_1')
    parser.add_option('-a', '--hexfile2', dest='hexfile2', help='HEX file 2', metavar='HEX_FILE_2')

    (options, args) = parser.parse_args()

    if options.hexfile1 is None:
        error('HEX file 1 is required (use -f1 option)')

    if options.hexfile2 is None:
        error('HEX file 2 is required (use -f2 option)')

    # Open the hex file 1
    with open(options.hexfile1, 'r') as file:
        lines = file.readlines()

    # Remove the last line if it is :00000001FF
    if lines[-1].strip() == ':00000001FF':
        lines = lines[:-1]

    # Open the hex file 2 to append
    with open(options.hexfile2, 'r') as file:
        to_append = file.readlines()

    # Append the hex file 2 to the hex file 1
    lines += to_append

    # Write the updated hex file 1
    with open(options.hexfile1, 'w') as file:
        file.writelines(lines)

#------------------------------------------------------------------------------

main()
