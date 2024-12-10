#include "lib_tar.h"

/**
 * Checks whether the archive is valid.
 *
 * Each non-null header of a valid archive has:
 *  - a magic value of "ustar" and a null,
 *  - a version value of "00" and no null,
 *  - a correct checksum
 *
 * @param tar_fd A file descriptor pointing to the start of a file supposed to contain a tar archive.
 *
 * @return a zero or positive value if the archive is valid, representing the number of non-null headers in the archive,
 *         -1 if the archive contains a header with an invalid magic value,
 *         -2 if the archive contains a header with an invalid version value,
 *         -3 if the archive contains a header with an invalid checksum value
 */
int check_archive(int tar_fd){

    // valid magic 
    tar_header_t header;
    if (strncmp(header.magic, TMAGIC, TMAGLEN) != 0) {
        return -1;
    }

    // valid version
    if (strncmp(header.version, TVERSION, TVERSLEN) != 0){
        return -2;
    }

    // valid checksum value
    char tmp_chksum[sizeof(header.chksum)];
    memcpy(tmp_chksum, header.chksum, sizeof(header.chksum));
    memset(header.chksum, ' ', sizeof(header.chksum));
    int res_sum = 0;
    uint8_t *header_bytes = (uint8_t *)&header;
    for (size_t i = 0; i < sizeof(tar_header_t); i++) {
        res_sum += header_bytes[i];
    }
    int stored_checksum = TAR_INT(tmp_chksum);
    if (res_sum != stored_checksum) {
        return -3;
    }

    return 0;
}

/**
 * Checks whether an entry exists in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive,
 *         any other value otherwise.
 */
int exists(int tar_fd, char *path) {
    tar_header_t header;
    ssize_t num_bytes;

    while ((num_bytes = read(tar_fd, &header, sizeof(tar_header_t))) == sizeof(tar_header_t)) {
        if (strncmp(header.name, path, sizeof(header.name)) == 0) {
            return 3;
        }
        // padding
        lseek(tar_fd, 512 - num_bytes, SEEK_CUR);
    }

    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a directory.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a directory,
 *         any other value otherwise.
 */
int is_dir(int tar_fd, char *path){
    tar_header_t header;
    ssize_t num_bytes;

    while((num_bytes = read(tar_fd, &header, sizeof(tar_header_t))) == sizeof(tar_header_t)){
        // path entry (compare)
        if (strcnmp(header.name, path, sizeof(header.name)) == 0 ){
            // directory ? if yes -> not 0
            if (header.typeflag == '5'){
                return 3;
            }
            return 0;
        }
        //padding
        lseek(tar_fd, 512 - num_bytes, SEEK_CUR);
    }
    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a file.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a file,
 *         any other value otherwise.
 */
int is_file(int tar_fd, char *path){
    tar_header_t header;
    ssize_t num_bytes;

    while((num_bytes = read(tar_fd, &header, sizeof(tar_header_t))) == sizeof(tar_header_t)){
        // path entry (compare)
        if (strcnmp(header.name, path, sizeof(header.name)) == 0 ){
            // same but for files
            if (header.typeflag == '0'){
                return 3;
            }
            return 0;
        }
        //padding
        lseek(tar_fd, 512 - num_bytes, SEEK_CUR);
    }
    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a symlink.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 * @return zero if no entry at the given path exists in the archive or the entry is not symlink,
 *         any other value otherwise.
 */
int is_symlink(int tar_fd, char *path){
    tar_header_t header;
    ssize_t num_bytes;

    while((num_bytes = read(tar_fd, &header, sizeof(tar_header_t))) == sizeof(tar_header_t)){
        // path entry (compare)
        if (strcnmp(header.name, path, sizeof(header.name)) == 0 ){
            // same but for syst link
            if (header.typeflag == '2'){
                return 3;
            }
            return 0;
        }
        //padding
        lseek(tar_fd, 512 - num_bytes, SEEK_CUR);
    }
    return 0;
}


/**
 * Lists the entries at a given path in the archive.
 * list() does not recurse into the directories listed at the given path.
 *
 * Example:
 *  dir/          list(..., "dir/", ...) lists "dir/a", "dir/b", "dir/c/" and "dir/e/"
 *   ├── a
 *   ├── b
 *   ├── c/
 *   │   └── d
 *   └── e/
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive. If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param entries An array of char arrays, each one is long enough to contain a tar entry path.
 * @param no_entries An in-out argument.
 *                   The caller set it to the number of entries in `entries`.
 *                   The callee set it to the number of entries listed.
 *
 * @return zero if no directory at the given path exists in the archive,
 *         any other value otherwise.
 */
int list(int tar_fd, char *path, char **entries, size_t *no_entries) {
    tar_header_t header;
    ssize_t num_bytes;
    size_t lenght_of_path = strlen(path);
    size_t entries_compt = 0;

    // while on header
    while ((num_bytes = read(tar_fd, &header, sizeof(tar_header_t))) == sizeof(tar_header_t)) {
        // entry begin with path ?
        if (strncmp(header.name, path, lenght_of_path) == 0) {
            char *subpath = header.name + lenght_of_path;
            if (strchr(subpath, '/') == NULL || strchr(subpath, '/') == subpath + strlen(subpath) - 1) {
                // add to table if ok
                if (entries_compt < *no_entries) {
                    strncpy(entries[entries_compt], header.name, sizeof(header.name));
                }
                entries_compt++;
            }
        }
        // padding
        lseek(tar_fd, 512 - num_bytes, SEEK_CUR);
    }

    *no_entries = entries_compt;
    // return 0 if not found
    if (entries_compt > 0) {
        return 3;
    } else {
        return 0;
    }
}

/**
 * Reads a file at a given path in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive to read from.  If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param offset An offset in the file from which to start reading from, zero indicates the start of the file.
 * @param dest A destination buffer to read the given file into.
 * @param len An in-out argument.
 *            The caller set it to the size of dest.
 *            The callee set it to the number of bytes written to dest.
 *
 * @return -1 if no entry at the given path exists in the archive or the entry is not a file,
 *         -2 if the offset is outside the file total length,
 *         zero if the file was read in its entirety into the destination buffer,
 *         a positive value if the file was partially read, representing the remaining bytes left to be read to reach
 *         the end of the file.
 *
 */
ssize_t read_file(int tar_fd, char *path, size_t offset, uint8_t *dest, size_t *len) {
    tar_header_t header;
    ssize_t num_bytes;

    // while on header tar
    while ((num_bytes = read(tar_fd, &header, sizeof(tar_header_t))) == sizeof(tar_header_t)) {
        // size of file
        size_t file_size = TAR_INT(header.size);
        
        if (strncmp(header.name, path, sizeof(header.name)) != 0) {
            size_t file_size = TAR_INT(header.size);
            lseek(tar_fd, ((file_size + 511) / 512) * 512, SEEK_CUR);
            continue;
        }

        // regaular file ?
        if (header.typeflag != REGTYPE && header.typeflag != AREGTYPE) {
            return -1;
        }

        // offset ok ?
        if (offset >= file_size) {
            return -2;
        }

        size_t bytes_lenght;
        if (file_size - offset < *len) {
            bytes_lenght = file_size - offset;
        } else {
            bytes_lenght = *len;
        }

        lseek(tar_fd, offset, SEEK_CUR);

        // r data
        ssize_t bytes_r = read(tar_fd, dest, bytes_lenght);
        if (bytes_r < 0) {
            return -1;
        }

        *len = bytes_r;

        // data remaining
        size_t bytes_left = file_size - offset - bytes_r;
        if (bytes_left > 0) {
            return bytes_left;
        } else {
            return 0;
        }
}